#include "everygraph/data/commands/worksheet_edit.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <initializer_list>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "everygraph/application/document.hpp"
#include "everygraph/application/value.hpp"
#include "everygraph/data/chunk.hpp"
#include "everygraph/data/datetime.hpp"
#include "everygraph/data/document.hpp"
#include "everygraph/data/ids.hpp"
#include "everygraph/data/logical_type.hpp"
#include "everygraph/data/provenance.hpp"
#include "everygraph/data/row_domain.hpp"
#include "everygraph/data/units.hpp"
#include "everygraph/foundation/error.hpp"
#include "everygraph/foundation/ids.hpp"

namespace everygraph::data {
namespace {

constexpr std::uint32_t kSchemaVersion = 1;

// One block states no more cells than the chunk row bound the data model declares, because a
// successor column is published as one chunk. A caller with more rows uses the import or derive
// path, which splits them, rather than this editing command growing an unbounded buffer. Tied to
// `kDefaultChunkRowCount` rather than a number of its own so the two move together when the
// measured chunk policy replaces the declared starting value.
constexpr std::uint64_t kMaxGridCells = kDefaultChunkRowCount;

using Error = foundation::Error;
using ObjectId = foundation::ObjectId;
using Revision = foundation::Revision;
using ParameterMap = application::ParameterMap;
using CommandTransaction = application::CommandTransaction;
using CommandRequest = application::CommandRequest;
using ExecutionContext = application::ExecutionContext;
using ObjectState = application::ObjectState;
using Value = application::Value;

Diagnostic reject(DetailCode code, std::string message, std::string context = {}) {
    Location location;
    location.description = std::move(context);
    return Diagnostic::reject(code, std::move(message), std::move(location));
}

std::optional<Error> fail(DetailCode code, std::string message, std::string context = {}) {
    return reject(code, std::move(message), std::move(context)).to_error();
}

std::optional<Error> from_kernel(const std::optional<Error>& error, DetailCode code,
                                std::string context) {
    if (!error.has_value()) {
        return std::nullopt;
    }
    return fail(code, error->message, std::move(context));
}

std::optional<Error> annotate(const Diagnostic& diagnostic, std::string context) {
    Diagnostic annotated = diagnostic;
    annotated.location.description = std::move(context);
    return annotated.to_error();
}

// The argument reads of this family. An identity or a position travels as its decimal text, so
// no argument is ever a display label or an array position a re-sort could invalidate.
class Args {
public:
    explicit Args(const ParameterMap& values) : values_(values) {}

    std::optional<Error> text(std::string_view key, std::string& out, bool required = true) const {
        const auto found = values_.find(key);
        if (found == values_.end()) {
            if (required) {
                return fail(DetailCode::type_mismatch, "required argument is missing",
                            std::string{key});
            }
            return std::nullopt;
        }
        const auto* spelling = std::get_if<std::string>(&found->second);
        if (spelling == nullptr) {
            return fail(DetailCode::type_mismatch, "argument is not text", std::string{key});
        }
        out = *spelling;
        return std::nullopt;
    }

    std::optional<Error> optional_text(std::string_view key, std::optional<std::string>& out) const {
        const auto found = values_.find(key);
        if (found == values_.end()) {
            return std::nullopt;
        }
        const auto* spelling = std::get_if<std::string>(&found->second);
        if (spelling == nullptr) {
            return fail(DetailCode::type_mismatch, "argument is not text", std::string{key});
        }
        out = *spelling;
        return std::nullopt;
    }

    std::optional<Error> id(std::string_view key, std::uint64_t& out, bool required = true) const {
        std::string spelling;
        if (auto error = this->text(key, spelling, required)) {
            return error;
        }
        if (spelling.empty()) {
            if (required) {
                return fail(DetailCode::duplicate_id,
                            "an identity argument is the decimal text of a non-zero opaque id",
                            std::string{key});
            }
            return std::nullopt;
        }
        out = 0;
        return decimal(spelling, out, key,
                       "identity argument is not the decimal text of an opaque id");
    }

    std::optional<Error> position(std::string_view key, std::uint64_t& out,
                                  bool required = true) const {
        std::string spelling;
        if (auto error = this->text(key, spelling, required)) {
            return error;
        }
        if (spelling.empty()) {
            return std::nullopt;
        }
        out = 0;
        return decimal(spelling, out, key, "position argument is not a decimal count");
    }

    std::optional<Error> ids(std::string_view key, std::vector<std::uint64_t>& out) const {
        out.clear();
        std::string spelling;
        if (auto error = this->text(key, spelling)) {
            return error;
        }
        if (spelling.empty()) {
            return std::nullopt;
        }
        std::size_t start = 0;
        while (true) {
            const auto separator = spelling.find(';', start);
            const std::size_t stop = separator == std::string::npos ? spelling.size() : separator;
            std::uint64_t value = 0;
            if (start >= stop) {
                return fail(DetailCode::type_mismatch, "id list entry is empty",
                            std::string{spelling});
            }
            if (auto error = decimal(spelling.substr(start, stop - start), value, key,
                                     "id list entry is not the decimal text of an opaque id")) {
                return error;
            }
            out.push_back(value);
            if (separator == std::string_view::npos) {
                break;
            }
            start = separator + 1;
        }
        return std::nullopt;
    }

    std::optional<Error> optional_ids(std::string_view key,
                                      std::vector<std::uint64_t>& out) const {
        out.clear();
        return contains(key) ? this->ids(key, out) : std::nullopt;
    }

    std::optional<Error> all_ids(std::initializer_list<std::string_view> keys) const {
        for (std::string_view key : keys) {
            std::uint64_t value = 0;
            if (auto error = this->id(key, value)) {
                return error;
            }
        }
        return std::nullopt;
    }

    bool contains(std::string_view key) const { return values_.contains(key); }

private:
    static std::optional<Error> decimal(std::string_view spelling, std::uint64_t& out,
                                        std::string_view key, const char* message) {
        const auto parsed = std::from_chars(spelling.data(), spelling.data() + spelling.size(), out);
        if (parsed.ec != std::errc{} || parsed.ptr != spelling.data() + spelling.size()) {
            return fail(DetailCode::type_mismatch, message, std::string{key});
        }
        return std::nullopt;
    }

    const ParameterMap& values_;
};

void set_id(ParameterMap& properties, std::string_view key, std::uint64_t value) {
    properties[std::string{key}] = Value{std::to_string(value)};
}

void set_text(ParameterMap& properties, std::string_view key, std::string value) {
    properties[std::string{key}] = Value{std::move(value)};
}

std::optional<Error> check_unique(const CommandTransaction& transaction, std::uint64_t id,
                                 std::string context) {
    const auto* existing = transaction.find(ObjectId{id});
    if (existing == nullptr) {
        return std::nullopt;
    }
    return fail(DetailCode::duplicate_id,
                "this identity is already in use and identities are never reused",
                std::move(context) + "; existing object " + foundation::to_string(ObjectId{id}) +
                    " of type " + existing->type + " at revision " +
                    std::to_string(existing->revision));
}

std::optional<Error> create_object(CommandTransaction& transaction, std::uint64_t id,
                                  std::string_view type, ParameterMap properties,
                                  std::string context) {
    if (id == 0) {
        return fail(DetailCode::duplicate_id, "identity must be a non-zero opaque id",
                    std::move(context));
    }
    auto error = transaction.create_object(ObjectId{id}, std::string{type}, std::move(properties));
    if (error.has_value()) {
        return fail(DetailCode::duplicate_id, error->message, std::move(context));
    }
    return std::nullopt;
}

std::optional<Error> require_object(const CommandTransaction& transaction, std::uint64_t id,
                                   std::string_view expected_type, const ObjectState*& out,
                                   std::string context) {
    const auto* state = transaction.find(ObjectId{id});
    if (state == nullptr) {
        return fail(DetailCode::dangling_reference,
                    "the object this command refers to does not exist", std::move(context));
    }
    if (state->type != expected_type) {
        return fail(DetailCode::type_mismatch,
                    std::string{"the referenced object has type "} + state->type + " instead of " +
                        std::string{expected_type},
                    std::move(context));
    }
    out = state;
    return std::nullopt;
}

std::optional<Error> require_property(const ObjectState& state, std::string_view key,
                                    std::uint64_t& out, std::string context) {
    const auto value = read_id_property(state, key);
    if (!value.has_value()) {
        return fail(DetailCode::dangling_reference,
                    std::string{"the referenced object does not carry a readable "} +
                        std::string{key} + " property",
                    std::move(context));
    }
    out = *value;
    return std::nullopt;
}

std::optional<Error> write_property(CommandTransaction& transaction, std::uint64_t id,
                                  std::string_view key, std::string value, std::string context) {
    auto error = transaction.set_property(ObjectId{id}, std::string{key}, Value{std::move(value)});
    return from_kernel(error, DetailCode::dangling_reference, std::move(context));
}

std::optional<Error> check_choice(std::string_view spelling,
                                 const std::initializer_list<std::string_view> allowed,
                                 std::string context) {
    if (std::find(allowed.begin(), allowed.end(), spelling) != allowed.end()) {
        return std::nullopt;
    }
    return fail(DetailCode::type_mismatch, "argument is not one of the declared choices",
                std::move(context));
}

std::optional<Error> check_text(std::string_view spelling, std::string context) {
    if (spelling.empty()) {
        return std::nullopt;
    }
    auto validated = validate_text(spelling, TextNulPolicy::reject);
    if (!validated.ok()) {
        return annotate(validated.diagnostic(), std::move(context));
    }
    return std::nullopt;
}

// The worksheet's bindings in the order the document view reports them, because a block edit
// and a reorder both address columns by that order. The kernel hands out the object list by
// value, so this returns copies.
std::vector<ObjectState> bindings_in_display_order(const CommandTransaction& transaction,
                                                  std::uint64_t worksheet) {
    std::vector<ObjectState> found;
    for (const auto& state : transaction.objects()) {
        if (state.type != object_type::kColumnBinding) {
            continue;
        }
        const auto parent = read_id_property(state, property::kParent);
        if (parent.has_value() && *parent == worksheet) {
            found.push_back(state);
        }
    }
    const auto order_of = [](const ObjectState& state) {
        const auto index = read_id_property(state, property::kOrderIndex);
        return index.has_value() ? *index : 0ULL;
    };
    std::sort(found.begin(), found.end(),
              [&order_of](const ObjectState& left, const ObjectState& right) {
                  if (order_of(left) != order_of(right)) {
                      return order_of(left) < order_of(right);
                  }
                  return left.id < right.id;
              });
    return found;
}

// ---------------------------------------------------------------------------
// The declared text form of a typed value
// ---------------------------------------------------------------------------

ValueForm form(char kind, std::string payload) {
    ValueForm result;
    result.kind = kind;
    result.payload = std::move(payload);
    return result;
}

std::string hex_text(std::uint64_t bits) {
    static constexpr char kDigits[] = "0123456789abcdef";
    if (bits == 0) {
        return "0";
    }
    std::string text;
    std::uint64_t remainder = bits;
    while (remainder != 0) {
        text += kDigits[remainder & 0xFULL];
        remainder >>= 4;
    }
    std::reverse(text.begin(), text.end());
    return text;
}

// The shortest decimal that reads back as exactly the same number. `std::to_chars` and
// `std::from_chars` are locale-independent, which is why a stored number is written as a plain
// point decimal here whatever the host locale later becomes.
std::string float_text(double value) {
    // A non-finite state leaves here in the one spelling `read_decimal` accepts. `std::to_chars`
    // spells a NaN after its host library — this SDK writes `nan(snan)` and `-nan(ind)` for
    // payloads a decimal reader refuses — so a stored document would otherwise carry text its own
    // family cannot read back. §3.4 separates the state classes, not the payload bits, and the bit
    // form is how a payload travels.
    if (std::isnan(value)) {
        return "nan";
    }
    if (std::isinf(value)) {
        return value > 0.0 ? "inf" : "-inf";
    }
    std::string text;
    text.resize(64);
    const auto result = std::to_chars(text.data(), text.data() + text.size(), value,
                                      std::chars_format::general);
    if (result.ec != std::errc{}) {
        text.clear();
        return text;
    }
    text.resize(static_cast<std::size_t>(result.ptr - text.data()));
    return text;
}

std::optional<std::int64_t> parse_signed(std::string_view payload) {
    std::int64_t value = 0;
    const auto parsed = std::from_chars(payload.data(), payload.data() + payload.size(), value);
    if (parsed.ec != std::errc{} || parsed.ptr != payload.data() + payload.size()) {
        return std::nullopt;
    }
    return value;
}

std::optional<std::uint64_t> parse_unsigned(std::string_view payload) {
    std::uint64_t value = 0;
    const auto parsed = std::from_chars(payload.data(), payload.data() + payload.size(), value);
    if (parsed.ec != std::errc{} || parsed.ptr != payload.data() + payload.size()) {
        return std::nullopt;
    }
    return value;
}

// `std::from_chars` for a floating-point payload needs a newer deployment target than this module
// declares, so the decimal form is checked here and handed to the C parser, which is correctly
// rounded. Only `.` separates a decimal fraction, so a host locale that spells the separator
// another way stops short of the end and is a refusal rather than a value read under a different
// meaning. Infinity and NaN arrive in the spelling `float_text` writes.
enum class DecimalRead {
    ok,
    malformed,
    out_of_range,
};

DecimalRead read_decimal(std::string_view payload, double& out) {
    static constexpr double kInfinity = std::numeric_limits<double>::infinity();
    if (payload == "inf") {
        out = kInfinity;
        return DecimalRead::ok;
    }
    if (payload == "-inf") {
        out = -kInfinity;
        return DecimalRead::ok;
    }
    if (payload == "nan") {
        out = std::numeric_limits<double>::quiet_NaN();
        return DecimalRead::ok;
    }
    // `strtod` reads an empty string as a zero and leaves the end pointer at the start, which
    // would pass the full-consumption check below. An empty payload is no number at all, so it is
    // refused here rather than stored as a zero.
    if (payload.empty()) {
        return DecimalRead::malformed;
    }
    for (const char symbol : payload) {
        if (std::string_view("0123456789+-.eE").find(symbol) == std::string_view::npos) {
            return DecimalRead::malformed;
        }
    }
    const std::string text{payload};
    errno = 0;
    char* end = nullptr;
    const double value = std::strtod(text.c_str(), &end);
    if (end != text.c_str() + text.size()) {
        return DecimalRead::malformed;
    }
    // `strtod` sets `ERANGE` when the magnitude leaves the type: past the top it saturates to an
    // infinity and below the bottom it collapses to zero, and in neither case is the returned
    // number the one the caller wrote. A subnormal that survives as a non-zero value is a number
    // the type holds, whatever `errno` says, and is kept.
    if (errno == ERANGE && (value == 0.0 || std::fabs(value) >= 1.0)) {
        return DecimalRead::out_of_range;
    }
    out = value;
    return DecimalRead::ok;
}

std::optional<double> parse_decimal(std::string_view payload) {
    double value = 0.0;
    if (read_decimal(payload, value) != DecimalRead::ok) {
        return std::nullopt;
    }
    return value;
}

std::optional<std::uint64_t> parse_bits(std::string_view payload) {
    std::uint64_t value = 0;
    const auto parsed =
        std::from_chars(payload.data(), payload.data() + payload.size(), value, 16);
    if (parsed.ec != std::errc{} || parsed.ptr != payload.data() + payload.size()) {
        return std::nullopt;
    }
    return value;
}

bool is_value_kind(char kind) {
    return std::string_view("iufbntcq").find(kind) != std::string_view::npos;
}

// Reads `<kind><byte-length>:<payload>` from the front of `text`, consuming it.
std::optional<Diagnostic> read_form(std::string_view& text, ValueForm& out) {
    if (text.empty()) {
        return reject(DetailCode::type_mismatch, "value form is empty", std::string{text});
    }
    const char kind = text.front();
    if (!is_value_kind(kind)) {
        return reject(DetailCode::type_mismatch,
                      "value form starts with an undeclared kind character", std::string{text});
    }
    text.remove_prefix(1);
    std::size_t digits = 0;
    while (digits < text.size() && text[digits] >= '0' && text[digits] <= '9') {
        ++digits;
    }
    if (digits == 0 || digits >= text.size() || text[digits] != ':') {
        return reject(DetailCode::type_mismatch,
                      "value form is not <kind><byte-length>:<payload>", std::string{text});
    }
    std::size_t length = 0;
    const auto parsed = std::from_chars(text.data(), text.data() + digits, length);
    if (parsed.ec != std::errc{}) {
        return reject(DetailCode::type_mismatch, "value byte length is not a decimal count",
                      std::string{text});
    }
    text.remove_prefix(digits + 1);
    if (text.size() < length) {
        return reject(DetailCode::type_mismatch,
                      "value form promises " + std::to_string(length) + " bytes and has " +
                          std::to_string(text.size()),
                      std::string{text});
    }
    out.kind = kind;
    out.payload.assign(text.data(), length);
    text.remove_prefix(length);
    return std::nullopt;
}

// Reads the `<rows>x<cols>;` header of a block, consuming it. A block declares its shape before
// its cells, so a mismatched count is a rejected command rather than a silently truncated paste.
std::optional<Diagnostic> read_grid_header(std::string_view& text, std::uint64_t& rows,
                                           std::uint64_t& columns) {
    const auto separator = text.find(';');
    if (separator == std::string_view::npos) {
        return reject(DetailCode::type_mismatch, "a value grid is <rows>x<cols>;<cells>",
                      std::string{text});
    }
    const std::string_view header = text.substr(0, separator);
    const auto cross = header.find('x');
    if (cross == std::string_view::npos) {
        return reject(DetailCode::type_mismatch, "a value grid header is <rows>x<cols>",
                      std::string{header});
    }
    const auto read_count = [](std::string_view digits, std::uint64_t& out) {
        const auto parsed = std::from_chars(digits.data(), digits.data() + digits.size(), out);
        return parsed.ec != std::errc{} || parsed.ptr != digits.data() + digits.size();
    };
    if (read_count(header.substr(0, cross), rows) || read_count(header.substr(cross + 1), columns)) {
        return reject(DetailCode::type_mismatch,
                      "a value grid header count is not a decimal number", std::string{header});
    }
    if (rows == 0 || columns == 0) {
        return reject(DetailCode::type_mismatch,
                      "a value grid declares at least one row and column", std::string{header});
    }
    if (rows > kMaxGridCells / columns) {
        return reject(
            DetailCode::resource_limit,
            "a value grid of " + std::to_string(rows) + " by " + std::to_string(columns) +
                " cells is beyond the declared block bound of " + std::to_string(kMaxGridCells),
            std::string{header});
    }
    text.remove_prefix(separator + 1);
    return std::nullopt;
}

// ---------------------------------------------------------------------------
// One value form against one declared field
// ---------------------------------------------------------------------------

Diagnostic field_diagnostic(DetailCode code, std::string message, const FieldSchema& field,
                           std::uint64_t row_index, std::string detail = {}) {
    Location location;
    location.field = field.id;
    location.row_index = static_cast<std::int64_t>(row_index);
    location.description = std::string{to_string(field.type)};
    if (!detail.empty()) {
        location.description += " " + detail;
    }
    return Diagnostic::reject(code, std::move(message), std::move(location));
}

// Exactness, checked by round-tripping through the widths that are guaranteed in range. The
// range test comes *before* the cast back, because converting a double outside the target's
// range to an integer is undefined rather than a comparison: on this host it saturates to the
// boundary, so `INT64_MAX -> 2^63 -> INT64_MAX` reads as an exact round trip and would store a
// value the field never held. 2^63 is the first double above the largest int64 and 2^64 the
// first above the largest uint64, and both are exact.
constexpr double kPastInt64Max = 9223372036854775808.0;
constexpr double kPastUint64Max = 18446744073709551616.0;

[[nodiscard]] bool holds_exactly(std::int64_t value, LogicalType type) {
    const double widened = static_cast<double>(value);
    if (widened >= kPastInt64Max || static_cast<std::int64_t>(widened) != value) {
        return false;
    }
    if (type == LogicalType::float32) {
        const float narrowed = static_cast<float>(widened);
        return static_cast<double>(narrowed) == widened;
    }
    return type == LogicalType::float64;
}

[[nodiscard]] bool holds_unsigned_exactly(std::uint64_t value, LogicalType type) {
    const double widened = static_cast<double>(value);
    if (widened >= kPastUint64Max || static_cast<std::uint64_t>(widened) != value) {
        return false;
    }
    if (type == LogicalType::float32) {
        const float narrowed = static_cast<float>(widened);
        return static_cast<double>(narrowed) == widened;
    }
    return type == LogicalType::float64;
}

Result<Cell> check_category_code(const FieldSchema& field, std::int64_t code,
                                 std::uint64_t row_index) {
    if (field.dictionary == nullptr) {
        return Result<Cell>::failure(
            field_diagnostic(DetailCode::unknown_category, "a categorical field needs a dictionary",
                             field, row_index));
    }
    if (code < 0 || static_cast<std::size_t>(code) >= field.dictionary->labels.size()) {
        return Result<Cell>::failure(field_diagnostic(
            DetailCode::unknown_category,
            "category code " + std::to_string(code) + " is outside the declared dictionary", field,
            row_index));
    }
    return Cell{field.type, CategoryValue{code}};
}

Result<Cell> store_signed(const FieldSchema& field, std::int64_t value, std::uint64_t row_index) {
    if (is_signed_integer(field.type)) {
        if (!in_range(value, field.type)) {
            return Result<Cell>::failure(field_diagnostic(
                DetailCode::number_overflow,
                "value is outside the range of " + std::string{to_string(field.type)}, field,
                row_index, std::to_string(value)));
        }
        return Cell{field.type, IntValue{value}};
    }
    if (is_unsigned_integer(field.type)) {
        if (!in_range(value, field.type)) {
            return Result<Cell>::failure(field_diagnostic(
                DetailCode::number_overflow,
                "a negative value or one outside the range does not fit an unsigned field", field,
                row_index, std::to_string(value)));
        }
        return Cell{field.type, UIntValue{static_cast<std::uint64_t>(value)}};
    }
    if (is_temporal(field.type)) {
        switch (field.type) {
            case LogicalType::date:
                return Cell{field.type, DateValue{value}};
            case LogicalType::duration:
                return Cell{field.type, DurationValue{value}};
            default:
                return Cell{field.type, TimestampValue{value}};
        }
    }
    if (is_floating(field.type)) {
        if (!holds_exactly(value, field.type)) {
            return Result<Cell>::failure(field_diagnostic(
                DetailCode::precision_loss,
                "this integer is not exactly representable in the declared floating width", field,
                row_index, std::to_string(value)));
        }
        return field.type == LogicalType::float32
                   ? Cell{field.type, Float32Value{static_cast<float>(value)}}
                   : Cell{field.type, Float64Value{static_cast<double>(value)}};
    }
    return Result<Cell>::failure(
        field_diagnostic(DetailCode::type_mismatch,
                         "an integer form fills a signed, unsigned, temporal or exactly "
                         "representable floating field",
                         field, row_index));
}

Result<Cell> store_unsigned(const FieldSchema& field, std::uint64_t value, std::uint64_t row_index) {
    if (is_unsigned_integer(field.type)) {
        if (!in_range(value, field.type)) {
            return Result<Cell>::failure(field_diagnostic(
                DetailCode::number_overflow,
                "value is outside the range of " + std::string{to_string(field.type)}, field,
                row_index, std::to_string(value)));
        }
        return Cell{field.type, UIntValue{value}};
    }
    if (is_signed_integer(field.type)) {
        if (!in_range(value, field.type)) {
            return Result<Cell>::failure(field_diagnostic(
                DetailCode::number_overflow, "value is outside the range of the signed field", field,
                row_index, std::to_string(value)));
        }
        return Cell{field.type, IntValue{static_cast<std::int64_t>(value)}};
    }
    if (is_temporal(field.type)) {
        // A tick is a signed count, so the unsigned form reaches it only while it still fits the
        // signed range; in_range() reports no range for a temporal type and is not the check.
        if (value > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
            return Result<Cell>::failure(field_diagnostic(
                DetailCode::number_overflow, "tick count is outside the signed tick range", field,
                row_index, std::to_string(value)));
        }
        return store_signed(field, static_cast<std::int64_t>(value), row_index);
    }
    if (is_floating(field.type)) {
        if (!holds_unsigned_exactly(value, field.type)) {
            return Result<Cell>::failure(field_diagnostic(
                DetailCode::precision_loss,
                "this integer is not exactly representable in the declared floating width", field,
                row_index, std::to_string(value)));
        }
        return field.type == LogicalType::float32
                   ? Cell{field.type, Float32Value{static_cast<float>(value)}}
                   : Cell{field.type, Float64Value{static_cast<double>(value)}};
    }
    return Result<Cell>::failure(field_diagnostic(
        DetailCode::type_mismatch, "an unsigned form fills an integer, tick or floating field",
        field, row_index));
}

Result<Cell> store_double(const FieldSchema& field, double value, std::uint64_t row_index) {
    if (field.type == LogicalType::float64) {
        return Cell{field.type, Float64Value{value}};
    }
    if (field.type == LogicalType::float32) {
        const float narrowed = static_cast<float>(value);
        // A NaN stays a NaN: its payload is not preserved through a decimal spelling, which is
        // why the bit form exists, so only the state class is compared here.
        const bool same =
            std::isnan(value) ? std::isnan(narrowed) : static_cast<double>(narrowed) == value;
        if (!same) {
            return Result<Cell>::failure(field_diagnostic(
                DetailCode::precision_loss,
                "this decimal is not exactly representable in float32, so it is rejected rather "
                "than demoted",
                field, row_index, float_text(value)));
        }
        return Cell{field.type, Float32Value{narrowed}};
    }
    if (is_integer(field.type) || is_temporal(field.type)) {
        if (!std::isfinite(value) || std::trunc(value) != value || value >= kPastInt64Max ||
            value < -kPastInt64Max) {
            return Result<Cell>::failure(field_diagnostic(
                DetailCode::precision_loss,
                "a fractional, infinite or unrepresentable value does not fit an integer or tick "
                "field",
                field, row_index, float_text(value)));
        }
        return store_signed(field, static_cast<std::int64_t>(value), row_index);
    }
    return Result<Cell>::failure(field_diagnostic(
        DetailCode::type_mismatch, "a decimal form fills a floating, integer or tick field", field,
        row_index));
}

Result<Cell> store_bits(const FieldSchema& field, std::uint64_t bits, std::uint64_t row_index) {
    if (field.type == LogicalType::text) {
        return Result<Cell>::failure(field_diagnostic(
            DetailCode::type_mismatch, "a bit form exists only for fixed-width storage types", field,
            row_index));
    }
    const auto cell = Cell::from_bits(field.type, bits);
    if (cell.bit_pattern() != bits) {
        return Result<Cell>::failure(field_diagnostic(
            DetailCode::number_overflow,
            "bit form " + hex_text(bits) + " does not fit " + std::string{to_string(field.type)},
            field, row_index));
    }
    if (field.type == LogicalType::categorical) {
        return check_category_code(field, cell.category_code().value_or(-1), row_index);
    }
    return cell;
}

std::optional<double> numeric_of(const ValueForm& value) {
    if (value.kind == 'i') {
        const auto parsed = parse_signed(value.payload);
        if (!parsed.has_value()) {
            return std::nullopt;
        }
        return static_cast<double>(*parsed);
    }
    if (value.kind == 'u') {
        const auto parsed = parse_unsigned(value.payload);
        if (!parsed.has_value()) {
            return std::nullopt;
        }
        return static_cast<double>(*parsed);
    }
    if (value.kind == 'f') {
        return parse_decimal(value.payload);
    }
    return std::nullopt;
}

// The declared resolution of one value form for one field. The form names the storage class and
// the field decides whether it is accepted, so a value never arrives in a shape the column was
// not declared to hold. A named source unit is converted into the field's declared unit before
// the value is stored, so a reading taken in one unit never lands as if it were another.
Result<Cell> to_cell(const FieldSchema& field, const ValueForm& value, std::uint64_t row_index,
                     const Unit* source_unit) {
    if (value.kind == 'n') {
        if (!field.nullable) {
            return Result<Cell>::failure(field_diagnostic(
                DetailCode::type_mismatch,
                "a required field holds no missing value, so a blank cell is rejected rather than "
                "stored as a zero",
                field, row_index));
        }
        return Cell::missing(field.type);
    }
    if (source_unit != nullptr) {
        if (!is_numeric(field.type)) {
            return Result<Cell>::failure(field_diagnostic(
                DetailCode::type_mismatch, "a value in a named unit fills a numeric field", field,
                row_index));
        }
        if (!field.unit.has_value()) {
            return Result<Cell>::failure(field_diagnostic(
                DetailCode::unknown_unit,
                "the value arrived in a named unit but the field declares none, so it would be "
                "stored as a bare number",
                field, row_index));
        }
        const auto number = numeric_of(value);
        if (!number.has_value()) {
            return Result<Cell>::failure(field_diagnostic(
                DetailCode::number_ambiguous,
                "a unit conversion reads a signed, unsigned or decimal form", field, row_index,
                value.payload));
        }
        Quantity input;
        input.value = *number;
        input.unit = *source_unit;
        auto converted = convert(input, *field.unit);
        if (!converted.ok()) {
            Diagnostic diagnostic = converted.diagnostic_copy();
            diagnostic.location.field = field.id;
            diagnostic.location.row_index = static_cast<std::int64_t>(row_index);
            return Result<Cell>::failure(std::move(diagnostic));
        }
        // The accepted conversion scales in a wider float and narrows back, so a finite reading
        // can arrive here past the top of `double`. The caller named a finite number, so an
        // infinite result is refused rather than published as a state class it never stated.
        if (std::isfinite(input.value) && !std::isfinite(converted.value().value)) {
            return Result<Cell>::failure(field_diagnostic(
                DetailCode::number_overflow,
                "the value past the unit conversion leaves the range of the type, so it is refused "
                "rather than stored as an infinity",
                field, row_index, value.payload));
        }
        return store_double(field, converted.value().value, row_index);
    }
    switch (value.kind) {
        case 'i': {
            const auto parsed = parse_signed(value.payload);
            if (!parsed.has_value()) {
                return Result<Cell>::failure(field_diagnostic(
                    DetailCode::number_ambiguous, "signed value is not a plain decimal integer",
                    field, row_index, value.payload));
            }
            return store_signed(field, *parsed, row_index);
        }
        case 'u': {
            const auto parsed = parse_unsigned(value.payload);
            if (!parsed.has_value()) {
                return Result<Cell>::failure(field_diagnostic(
                    DetailCode::number_ambiguous, "unsigned value is not a plain decimal integer",
                    field, row_index, value.payload));
            }
            return store_unsigned(field, *parsed, row_index);
        }
        case 'f': {
            double parsed = 0.0;
            const auto read = read_decimal(value.payload, parsed);
            if (read == DecimalRead::out_of_range) {
                return Result<Cell>::failure(field_diagnostic(
                    DetailCode::number_overflow,
                    "a decimal outside the range of the target type is refused instead of stored "
                    "as an infinity or as a zero it did not declare",
                    field, row_index, value.payload));
            }
            if (read != DecimalRead::ok) {
                return Result<Cell>::failure(
                    field_diagnostic(DetailCode::number_ambiguous,
                                     "decimal value is not a plain locale-independent number; an "
                                     "empty payload, a decimal comma or a grouped spelling belongs "
                                     "to the import rules",
                                     field, row_index, value.payload));
            }
            return store_double(field, parsed, row_index);
        }
        case 'b': {
            const auto parsed = parse_bits(value.payload);
            if (!parsed.has_value()) {
                return Result<Cell>::failure(field_diagnostic(
                    DetailCode::type_mismatch, "bit value is not a hexadecimal pattern", field,
                    row_index, value.payload));
            }
            return store_bits(field, *parsed, row_index);
        }
        case 't': {
            if (field.type != LogicalType::text) {
                return Result<Cell>::failure(
                    field_diagnostic(DetailCode::type_mismatch, "a text form fills a text field",
                                     field, row_index));
            }
            auto validated = validate_text(value.payload, TextNulPolicy::reject);
            if (!validated.ok()) {
                Diagnostic diagnostic = validated.diagnostic_copy();
                diagnostic.location.field = field.id;
                diagnostic.location.row_index = static_cast<std::int64_t>(row_index);
                return Result<Cell>::failure(std::move(diagnostic));
            }
            return Cell{field.type, TextValue{validated.take()}};
        }
        case 'c': {
            if (field.type != LogicalType::categorical) {
                return Result<Cell>::failure(field_diagnostic(
                    DetailCode::type_mismatch, "a category label fills a categorical field", field,
                    row_index));
            }
            auto resolved = field.resolve_category(value.payload);
            if (!resolved.ok()) {
                Diagnostic diagnostic = resolved.diagnostic_copy();
                diagnostic.location.row_index = static_cast<std::int64_t>(row_index);
                return Result<Cell>::failure(std::move(diagnostic));
            }
            return resolved;
        }
        default: {
            const auto code = parse_signed(value.payload);
            if (!code.has_value()) {
                return Result<Cell>::failure(field_diagnostic(
                    DetailCode::type_mismatch, "category code is not a decimal integer", field,
                    row_index, value.payload));
            }
            return check_category_code(field, *code, row_index);
        }
    }
}

// ---------------------------------------------------------------------------
// Reading a column and rebuilding it
// ---------------------------------------------------------------------------

// A kept value is copied through the bits it is stored in, so an insert, delete, move, reorder
// or paste never re-keys a number through a decimal string and never loses a NaN payload or a
// signed zero.
std::optional<Diagnostic> copy_cell(ChunkBuilder& builder, const Cell& cell) {
    if (cell.is_missing()) {
        return builder.append_missing();
    }
    if (cell.type() == LogicalType::text) {
        return builder.append_text(cell.text_value().value_or(std::string_view{}));
    }
    const auto bits = cell.bit_pattern();
    if (!bits.has_value()) {
        return reject(DetailCode::type_mismatch, "the stored value reports no bit form to copy",
                      describe(cell));
    }
    return builder.append_bits(*bits);
}

Result<std::shared_ptr<const Chunk>> build_chunk(const FieldSchema& field, ChunkId id,
                                                 const std::vector<Cell>& cells) {
    auto builder = ChunkBuilder::create(field.id, field.type);
    if (!builder.ok()) {
        return Result<std::shared_ptr<const Chunk>>::failure(builder.diagnostic_copy());
    }
    for (std::size_t index = 0; index < cells.size(); ++index) {
        if (auto refusal = copy_cell(builder.value(), cells[index])) {
            Diagnostic diagnostic = *refusal;
            diagnostic.location.field = field.id;
            diagnostic.location.row_index = static_cast<std::int64_t>(index);
            return Result<std::shared_ptr<const Chunk>>::failure(std::move(diagnostic));
        }
    }
    auto finished = builder.value().finish(id, 0);
    if (!finished.ok()) {
        return Result<std::shared_ptr<const Chunk>>::failure(finished.diagnostic_copy());
    }
    return std::make_shared<const Chunk>(finished.take());
}

// One declared conversion route at a time. A value that cannot arrive in the target type without
// a loss the caller did not name is reported with its row and both types, which is what makes a
// preview and a rejected change state the same thing.
// The value of a floating source at float64 precision. A float32 cell answers only
// float32_value(), so reading one through float64_value() alone would report no route at all.
std::optional<double> source_float(const Cell& source, LogicalType from) {
    if (!is_floating(from)) {
        return std::nullopt;
    }
    if (const auto value = source.float64_value(); value.has_value()) {
        return *value;
    }
    if (const auto value = source.float32_value(); value.has_value()) {
        return static_cast<double>(*value);
    }
    return std::nullopt;
}

Result<Cell> convert_cell(const Cell& source, const FieldSchema& source_field,
                          const FieldSchema& target, ConversionPolicy policy,
                          std::uint64_t row_index) {
    const LogicalType from = source.is_missing() ? source_field.type : source.type();
    const LogicalType to = target.type;
    if (source.is_missing()) {
        if (!target.nullable) {
            return Result<Cell>::failure(field_diagnostic(
                DetailCode::type_mismatch,
                "a required field holds no missing value, so the conversion has nothing to write "
                "for this row",
                target, row_index));
        }
        return Cell::missing(to);
    }
    // A categorical code is a dictionary index, so only an unchanged dictionary makes an
    // equal-typed cell need no work; the shared_ptr of a restated dictionary is never the same
    // object even when its labels are, and the relabel below is what resolves it.
    if (from == to && from != LogicalType::categorical) {
        return source;
    }

    if (is_temporal(from) || is_temporal(to)) {
        if (!is_temporal(from) || !is_temporal(to)) {
            return Result<Cell>::failure(field_diagnostic(
                DetailCode::type_mismatch,
                "a temporal field stores ticks in its own tick unit, so it converts to no numeric "
                "field, no text and no other tick unit",
                target, row_index,
                std::string{to_string(from)} + " (" + to_string(tick_unit(from)) + ") to " +
                    to_string(to) + " (" + to_string(tick_unit(to)) + ")"));
        }
        if (tick_unit(from) != tick_unit(to)) {
            return Result<Cell>::failure(field_diagnostic(
                DetailCode::type_mismatch,
                "a day count is not a microsecond count, so this route would silently rescale the "
                "stored value",
                target, row_index));
        }
        const auto ticks = source.tick_value();
        if (!ticks.has_value()) {
            return Result<Cell>::failure(field_diagnostic(
                DetailCode::type_mismatch, "the source value holds no tick", target, row_index));
        }
        return store_signed(target, *ticks, row_index);
    }

    if (from == LogicalType::boolean || to == LogicalType::boolean) {
        return Result<Cell>::failure(field_diagnostic(
            DetailCode::type_mismatch,
            "a flag is not a number and a number is not a flag, so this route needs the declared "
            "threshold conversion of the reshape family",
            target, row_index));
    }

    if (from == LogicalType::text && to != LogicalType::text) {
        const auto spelling = source.text_value();
        ValueForm decoded;
        // A category is named by its label, so a text value reaches a categorical field through
        // the label form and resolves against the target dictionary; every other target reads
        // the spelling as a number.
        decoded.kind = to == LogicalType::categorical ? 'c' : 'f';
        decoded.payload = std::string{spelling.value_or(std::string_view{})};
        return to_cell(target, decoded, row_index, nullptr);
    }

    if (to == LogicalType::text) {
        if (from == LogicalType::categorical) {
            const auto code = source.category_code();
            if (source_field.dictionary == nullptr || !code.has_value()) {
                return Result<Cell>::failure(field_diagnostic(
                    DetailCode::unknown_category,
                    "a category converts to text through the label its dictionary declares", target,
                    row_index));
            }
            auto label = source_field.dictionary->label_for(*code);
            if (!label.ok()) {
                Diagnostic diagnostic = label.diagnostic_copy();
                diagnostic.location.field = target.id;
                diagnostic.location.row_index = static_cast<std::int64_t>(row_index);
                return Result<Cell>::failure(std::move(diagnostic));
            }
            return Cell{LogicalType::text, TextValue{label.take()}};
        }
        if (is_integer(from)) {
            if (const auto signed_value = source.int_value(); signed_value.has_value()) {
                return Cell{LogicalType::text, TextValue{std::to_string(*signed_value)}};
            }
            if (const auto unsigned_value = source.uint_value(); unsigned_value.has_value()) {
                return Cell{LogicalType::text, TextValue{std::to_string(*unsigned_value)}};
            }
        }
        if (const auto number = source_float(source, from); number.has_value()) {
            return Cell{LogicalType::text, TextValue{float_text(*number)}};
        }
        return Result<Cell>::failure(field_diagnostic(
            DetailCode::type_mismatch, "this stored form has no declared spelling as text", target,
            row_index));
    }

    if (from == LogicalType::categorical || to == LogicalType::categorical) {
        if (from != LogicalType::categorical || to != LogicalType::categorical) {
            return Result<Cell>::failure(field_diagnostic(
                DetailCode::type_mismatch,
                "a category code is a dictionary index, not a measurement, so it converts only "
                "between categorical fields or into text",
                target, row_index));
        }
        // The code is an index into the source dictionary, so the relabel goes through the label
        // and never carries an old index into a new dictionary.
        const auto code = source.category_code();
        if (source_field.dictionary == nullptr || !code.has_value()) {
            return Result<Cell>::failure(field_diagnostic(
                DetailCode::unknown_category,
                "the source category has no dictionary to read its label from", target, row_index));
        }
        auto label = source_field.dictionary->label_for(*code);
        if (!label.ok()) {
            Diagnostic diagnostic = label.diagnostic_copy();
            diagnostic.location.field = target.id;
            diagnostic.location.row_index = static_cast<std::int64_t>(row_index);
            return Result<Cell>::failure(std::move(diagnostic));
        }
        auto resolved = target.resolve_category(label.value());
        if (!resolved.ok()) {
            Diagnostic diagnostic = resolved.diagnostic_copy();
            diagnostic.location.row_index = static_cast<std::int64_t>(row_index);
            return Result<Cell>::failure(std::move(diagnostic));
        }
        return resolved;
    }

    // Numbers. A widening that keeps the value needs no policy; anything that could lose a
    // fraction, a digit of range or a bit of precision is named by `round` or rejected.
    if (const auto number = source_float(source, from); number.has_value()) {
        if (is_floating(to)) {
            // `round` is the caller's declared permission to narrow a float64 to float32, which
            // stores the nearest representable value. An out-of-range value has no policy that
            // accepts it, so a narrowing that would become an infinity is still refused.
            if (to == LogicalType::float32 && policy == ConversionPolicy::round) {
                const float narrowed = static_cast<float>(*number);
                if (std::isfinite(*number) && !std::isfinite(narrowed)) {
                    return Result<Cell>::failure(field_diagnostic(
                        DetailCode::number_overflow,
                        "this value is outside the range of float32, and no policy turns a width "
                        "change into a clamp",
                        target, row_index, float_text(*number)));
                }
                return Cell{to, Float32Value{narrowed}};
            }
            return store_double(target, *number, row_index);
        }
        if (!std::isfinite(*number)) {
            return Result<Cell>::failure(field_diagnostic(
                DetailCode::precision_loss,
                "an infinity or NaN has no integer representation, and no declared policy creates "
                "one",
                target, row_index, float_text(*number)));
        }
        if (std::trunc(*number) != *number) {
            if (policy != ConversionPolicy::round) {
                return Result<Cell>::failure(field_diagnostic(
                    DetailCode::precision_loss,
                    "a fractional value becomes an integer only under the declared `round` policy, "
                    "which the caller names and this command never applies by default",
                    target, row_index, float_text(*number)));
            }
            // No range test is needed on the way into `llround`: the non-finite route is refused
            // above, so this holds a finite value with a fraction, and every double with a
            // fraction sits below 2^52 and therefore inside the signed width. The width of a
            // narrower target is what `store_signed` tests.
            return store_signed(target, std::llround(*number), row_index);
        }
        return store_double(target, *number, row_index);
    }
    if (is_integer(from)) {
        if (const auto signed_value = source.int_value(); signed_value.has_value()) {
            return store_signed(target, *signed_value, row_index);
        }
        if (const auto unsigned_value = source.uint_value(); unsigned_value.has_value()) {
            return store_unsigned(target, *unsigned_value, row_index);
        }
    }
    return Result<Cell>::failure(
        field_diagnostic(DetailCode::type_mismatch,
                         std::string{"no declared conversion route from "} + to_string(from) +
                             " to " + to_string(to),
                         target, row_index));
}

bool same_cells(const std::vector<Cell>& left, const std::vector<Cell>& right) {
    if (left.size() != right.size()) {
        return false;
    }
    for (std::size_t index = 0; index < left.size(); ++index) {
        if (!structurally_equal(left[index], right[index])) {
            return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// The worksheet, the columns one edit touches and the successor they publish
// ---------------------------------------------------------------------------

struct Worksheet {
    std::uint64_t id{0};
    std::uint64_t domain{0};
    std::shared_ptr<const RowDomain> rows;
};

std::optional<Error> open_worksheet(const CommandTransaction& transaction, DataStore& store,
                                    std::uint64_t worksheet, Worksheet& out) {
    out.id = worksheet;
    const ObjectState* state = nullptr;
    if (auto error = require_object(transaction, worksheet, object_type::kWorksheet, state,
                                    "worksheet " + std::to_string(worksheet))) {
        return error;
    }
    if (auto error = require_property(*state, property::kRowDomain, out.domain,
                                      "worksheet " + std::to_string(worksheet))) {
        return error;
    }
    auto domain = store.row_domain(row_domain_id(out.domain));
    if (!domain.ok()) {
        return domain.diagnostic().to_error();
    }
    out.rows = domain.take();
    return std::nullopt;
}

// One same-domain column an edit touches. A mirror is read-only, and a correspondence-bound
// column keeps its own pairs and is re-paired by the refresh path, so neither is rewritten by an
// edit of the worksheet's rows. Both rules are the ones the accepted row-change command states.
struct ColumnEdit {
    std::uint64_t binding{0};
    std::uint64_t dataset{0};
    std::shared_ptr<const DatasetVersion> current;
    FieldSchema field;
    std::vector<Cell> from;
    std::vector<Cell> to;
    std::uint64_t successor_version{0};
    std::uint64_t successor_chunk{0};
};

std::optional<Error> read_column(const ObjectState& state, DataStore& store, ColumnEdit& out) {
    const std::string context = "column " + foundation::to_string(state.id);
    const auto policy = read_text_property(state, property::kWritePolicy);
    if (policy.has_value() && *policy == "mirror") {
        return fail(DetailCode::mirror_read_only,
                    "a mirror is a binding to another column's version identity, so editing it "
                    "needs the detach or shared-edit command that declares the write",
                    context);
    }
    const auto link = read_text_property(state, property::kLink);
    if (link.has_value() && *link != "same_domain") {
        return fail(DetailCode::positional_mapping_required,
                    "this column's rows come from another row-identity domain, so a write through "
                    "it needs the correspondence-resolved path",
                    context);
    }
    std::uint64_t version_id = 0;
    if (auto error = require_property(state, property::kVersion, version_id, context)) {
        return error;
    }
    if (auto error = require_property(state, property::kDataset, out.dataset, context)) {
        return error;
    }
    auto found = store.version(dataset_version_id(version_id));
    if (!found.ok()) {
        return found.diagnostic().to_error();
    }
    out.current = found.take();
    out.binding = state.id.value;
    if (out.current->schema()->fields.size() != 1) {
        return fail(DetailCode::type_mismatch,
                    "a worksheet column is stored as a single-field dataset version", context);
    }
    out.field = out.current->schema()->fields.front();
    out.from.reserve(out.current->row_count());
    for (std::uint64_t index = 0; index < out.current->row_count(); ++index) {
        auto cell = out.current->cell(out.field.id, index);
        if (!cell.ok()) {
            return cell.diagnostic().to_error();
        }
        out.from.push_back(cell.take());
    }
    out.to = out.from;
    return std::nullopt;
}

std::optional<Error> load_columns(const CommandTransaction& transaction, DataStore& store,
                                  const Worksheet& sheet, std::vector<ColumnEdit>& out) {
    out.clear();
    for (const auto& state : bindings_in_display_order(transaction, sheet.id)) {
        const auto link = read_text_property(state, property::kLink);
        if (link.has_value() && *link != "same_domain") {
            continue;
        }
        ColumnEdit column;
        if (auto error = read_column(state, store, column)) {
            return error;
        }
        if (column.current->row_domain() != sheet.rows->id()) {
            return fail(DetailCode::unmapped_row,
                        "a same-domain column is ordered by the worksheet's own row domain",
                        "column " + foundation::to_string(state.id));
        }
        out.push_back(std::move(column));
    }
    if (out.empty()) {
        return fail(DetailCode::unknown_field,
                    "this worksheet has no same-domain column, so there is nothing to edit",
                    "worksheet " + std::to_string(sheet.id));
    }
    return std::nullopt;
}

std::optional<Error> load_one_column(const CommandTransaction& transaction, DataStore& store,
                                     std::uint64_t column, ColumnEdit& out) {
    const ObjectState* state = nullptr;
    if (auto error = require_object(transaction, column, object_type::kColumnBinding, state,
                                    "column " + std::to_string(column))) {
        return error;
    }
    return read_column(*state, store, out);
}

// The identities a successor is published under. One entry per column the command can rewrite,
// in the worksheet's display order, so no command invents an identity and none reuses one.
std::optional<Error> read_successor_ids(const Args& args, std::size_t offset, std::size_t expected,
                                        std::string context, std::vector<ColumnEdit>& columns) {
    std::vector<std::uint64_t> versions;
    std::vector<std::uint64_t> chunks;
    if (auto error = args.ids(argument::kSuccessorVersions, versions)) {
        return error;
    }
    if (auto error = args.ids(argument::kSuccessorChunks, chunks)) {
        return error;
    }
    if (versions.size() != expected || chunks.size() != expected) {
        return fail(DetailCode::unmapped_row,
                    "the successor identity lists must name one version and one chunk per column "
                    "this command rewrites, in display order, and this one names " +
                        std::to_string(versions.size()) + " versions and " +
                        std::to_string(chunks.size()) + " chunks for " +
                        std::to_string(expected) + " columns",
                    std::move(context));
    }
    if (offset + expected > columns.size()) {
        return fail(DetailCode::index_out_of_range,
                    "the successor identity lists cover more columns than this worksheet holds",
                    std::move(context));
    }
    for (std::size_t index = 0; index < expected; ++index) {
        columns[offset + index].successor_version = versions[index];
        columns[offset + index].successor_chunk = chunks[index];
    }
    return std::nullopt;
}

// Writes one declared value block into the columns it covers. Every cell is resolved against
// its own field before anything is published, so a rejected block leaves no partial output.
struct BlockTarget {
    std::uint64_t first_row{0};
    std::uint64_t first_column{0};
    std::uint64_t rows{0};
    std::uint64_t columns{0};
    std::uint64_t row_count{0};
};

// The shape a block declares, read before the worksheet's columns are loaded, because the block
// width is what says how many successor identities the command must name.
std::optional<Error> grid_shape(std::string_view grid, std::uint64_t& rows,
                                std::uint64_t& columns) {
    std::string_view text = grid;
    if (auto refusal = read_grid_header(text, rows, columns)) {
        return annotate(*refusal, "value block");
    }
    std::uint64_t count = 0;
    while (!text.empty()) {
        ValueForm value;
        if (auto refusal = read_form(text, value)) {
            return annotate(*refusal, "value block");
        }
        ++count;
    }
    if (count != rows * columns) {
        return fail(DetailCode::unmapped_row,
                    "a value grid of " + std::to_string(rows * columns) + " cells holds " +
                        std::to_string(count) + " complete forms",
                    "value block");
    }
    return std::nullopt;
}

std::optional<Error> write_block(std::string_view grid, const BlockTarget& target,
                                  std::vector<ColumnEdit>& columns, std::string context) {
    auto parsed = parse_grid(grid);
    if (!parsed.ok()) {
        return annotate(parsed.diagnostic(), std::move(context));
    }
    if (target.rows != 0 && parsed.value().size() != static_cast<std::size_t>(target.rows) *
                                                             static_cast<std::size_t>(target.columns)) {
        return fail(DetailCode::unmapped_row, "a value block declares " +
                                                  std::to_string(target.rows) + " by " +
                                                  std::to_string(target.columns) + " cells",
                    std::move(context));
    }
    if (target.first_column + target.columns > columns.size()) {
        return fail(DetailCode::index_out_of_range,
                    "the value block reaches past the last editable column of the worksheet, so a "
                    "paste is rejected instead of dropping cells",
                    std::move(context));
    }
    if (target.first_row + target.rows > target.row_count) {
        return fail(DetailCode::index_out_of_range,
                    "the value block reaches past the last row of the worksheet, so a paste is "
                    "rejected instead of growing the sheet or dropping cells",
                    std::move(context));
    }
    // The row bound above is the one the buffer index rests on: `load_columns` refuses a column
    // whose version belongs to another domain, and a version's cell count is its domain's row
    // count, so every column this block covers holds exactly `target.row_count` values.
    const auto& values = parsed.value();
    for (std::uint64_t row = 0; row < target.rows; ++row) {
        for (std::uint64_t column = 0; column < target.columns; ++column) {
            const auto& entry = values[static_cast<std::size_t>(row) *
                                           static_cast<std::size_t>(target.columns) +
                                       static_cast<std::size_t>(column)];
            const ColumnEdit& target_column = columns[target.first_column + column];
            auto cell = to_cell(target_column.field, entry, target.first_row + row, nullptr);
            if (!cell.ok()) {
                return annotate(cell.diagnostic(),
                                "row " + std::to_string(target.first_row + row) + " of column " +
                                    foundation::to_string(
                                        ObjectId{columns[target.first_column + column].binding}));
            }
            columns[target.first_column + column].to[target.first_row + row] = cell.take();
        }
    }
    return std::nullopt;
}

// How the resulting rows come from the current ones: each result row names the index it copies
// from, or nothing when it is a brand new row that holds no value yet.
struct RowEdit {
    std::vector<RowId> rows;
    std::vector<std::optional<std::size_t>> origins;
};

std::optional<Error> remap_rows(const RowEdit& edit, std::vector<ColumnEdit>& columns,
                                std::string context) {
    if (edit.rows.size() != edit.origins.size()) {
        return fail(DetailCode::unmapped_row,
                    "the revised row list and its copy plan must have equal length",
                    std::move(context));
    }
    for (auto& column : columns) {
        column.to.clear();
        column.to.reserve(edit.rows.size());
        for (std::size_t index = 0; index < edit.origins.size(); ++index) {
            const auto origin = edit.origins[index];
            if (!origin.has_value()) {
                if (!column.field.nullable) {
                    return fail(
                        DetailCode::type_mismatch,
                        "a new row gives a required field no value, so declare the values for it "
                        "instead of storing a blank",
                        std::move(context) + ": column " +
                            foundation::to_string(ObjectId{column.binding}) + ", row " +
                            to_string(edit.rows[index]));
                }
                column.to.push_back(Cell::missing(column.field.type));
                continue;
            }
            if (*origin >= column.from.size()) {
                return fail(DetailCode::index_out_of_range,
                            "the copy plan names a row this column does not hold",
                            std::move(context));
            }
            column.to.push_back(column.from[*origin]);
        }
    }
    return std::nullopt;
}

// Records one immutable version as a document object and stages its payload for the
// transaction that publishes it. This mirrors the accepted publish path so an edit gains a
// successor without a second mutation route.
std::optional<Error> publish_version(CommandTransaction& transaction, DataStore& store,
                                     std::uint64_t transaction_id,
                                     const std::shared_ptr<const DatasetVersion>& version,
                                     std::optional<std::uint64_t> previous, std::string context) {
    if (version == nullptr) {
        return fail(DetailCode::unknown_draft, "the successor version is not available",
                    std::move(context));
    }
    if (auto error = check_unique(transaction, version->id().raw(),
                                  context + " version " + to_string(version->id()))) {
        return error;
    }
    ParameterMap properties;
    set_id(properties, property::kVersion, version->id().raw());
    set_id(properties, property::kDataset, version->dataset().raw());
    set_id(properties, property::kRowDomain, version->row_domain().raw());
    set_id(properties, property::kRowCount, version->row_count());
    set_text(properties, property::kContentHash, version->content_hash().hex());
    set_text(properties, property::kLabel, version->provenance().operation_id);
    if (previous.has_value()) {
        set_id(properties, property::kPreviousVersion, *previous);
    }
    if (auto error = create_object(transaction, version->id().raw(),
                                   object_type::kDatasetVersion, std::move(properties),
                                   "version " + to_string(version->id()))) {
        return error;
    }
    store.stage(transaction_id, version);
    return std::nullopt;
}

struct EditTarget {
    std::uint64_t worksheet{0};
    std::uint64_t revised_domain{0};
    bool rows_named{false};
    const RowEdit* rows{nullptr};
    std::string operation;
    std::string parameters;
};

// Publishes what the edit computed, or nothing at all. The row domain identity only changes when
// the row identities or their order change, and a column only gains a successor when its field,
// its cells or its domain changed, so an edit that states no difference writes no object and the
// kernel reports a command that changed nothing.
std::optional<Error> publish_edit(CommandTransaction& transaction, DataStore& store,
                                  const ExecutionContext& context, const Worksheet& sheet,
                                  const EditTarget& target, std::vector<ColumnEdit>& columns,
                                  std::string label) {
    std::vector<ColumnEdit*> rewritten;
    bool rows_change = false;
    if (target.rows_named && target.rows != nullptr) {
        rows_change = target.rows->rows != sheet.rows->rows();
    }
    for (auto& column : columns) {
        const bool schema_change = column.field.canonical_text() !=
                                   column.current->schema()->fields.front().canonical_text();
        const bool cells_change = !same_cells(column.from, column.to);
        if (!rows_change && !schema_change && !cells_change) {
            continue;
        }
        rewritten.push_back(&column);
    }
    if (!rows_change && rewritten.empty()) {
        return std::nullopt;
    }

    const std::uint64_t domain_id = rows_change ? target.revised_domain : sheet.domain;
    if (rows_change) {
        if (auto error = check_unique(transaction, target.revised_domain,
                                      "row domain " + std::to_string(target.revised_domain))) {
            return error;
        }
    }
    for (auto* column : rewritten) {
        if (column->successor_version == 0 || column->successor_chunk == 0) {
            return fail(DetailCode::duplicate_id,
                        "a successor is published under the non-zero identities the caller named",
                        "column " + foundation::to_string(ObjectId{column->binding}));
        }
        if (auto error = check_unique(transaction, column->successor_version,
                                      "version " + std::to_string(column->successor_version))) {
            return error;
        }
    }

    std::shared_ptr<const RowDomain> revised;
    if (rows_change) {
        auto created = RowDomain::create(row_domain_id(target.revised_domain), target.rows->rows);
        if (!created.ok()) {
            return created.diagnostic().to_error();
        }
        revised = std::make_shared<const RowDomain>(created.take());
        ParameterMap properties;
        set_id(properties, property::kRowCount, revised->row_count());
        set_text(properties, property::kContentHash, revised->content_hash().hex());
        if (auto error = create_object(transaction, revised->id().raw(), object_type::kRowDomain,
                                       std::move(properties),
                                       "row domain " + to_string(revised->id()))) {
            return error;
        }
        store.stage(context.transaction_id, revised);
    }

    for (auto* column : rewritten) {
        const auto cells_per_row = column->to.size();
        auto chunk = build_chunk(column->field, chunk_id(column->successor_chunk), column->to);
        if (!chunk.ok()) {
            return annotate(chunk.diagnostic(), label + " column " +
                                                     foundation::to_string(
                                                         ObjectId{column->binding}));
        }
        auto schema = DataStore::schema({column->field});
        if (!schema.ok()) {
            return annotate(schema.diagnostic(), label + " column " +
                                                     foundation::to_string(
                                                         ObjectId{column->binding}));
        }
        Provenance record;
        record.operation_id = target.operation;
        record.operation_version = 1;
        record.canonical_parameters = target.parameters;
        record.unit_policy = "declared";
        record.parents.push_back(column->current->id());
        auto created = DatasetVersion::create(dataset_version_id(column->successor_version),
                                             dataset_id(column->dataset), schema.take(),
                                             row_domain_id(domain_id), {chunk.take()},
                                             std::move(record), cells_per_row,
                                             foundation::kInitialRevision);
        if (!created.ok()) {
            return annotate(created.diagnostic(), label + " column " +
                                                         foundation::to_string(
                                                             ObjectId{column->binding}));
        }
        const auto published = created.take();
        const std::string column_label =
            label + " column " + foundation::to_string(ObjectId{column->binding});
        if (auto error = publish_version(transaction, store, context.transaction_id, published,
                                         column->current->id().raw(), column_label)) {
            return error;
        }
        if (auto error = write_property(transaction, column->binding, property::kVersion,
                                        std::to_string(published->id().raw()), column_label)) {
            return error;
        }
        if (column->field.canonical_text() !=
            column->current->schema()->fields.front().canonical_text()) {
            const std::string unit_text =
                column->field.unit.has_value() ? column->field.unit->source_text() : std::string{};
            if (auto error =
                    write_property(transaction, column->binding, property::kUnit, unit_text,
                                   column_label)) {
                return error;
            }
        }
    }
    if (rows_change) {
        if (auto error = write_property(transaction, target.worksheet, property::kRowDomain,
                                        std::to_string(domain_id),
                                        "worksheet " + std::to_string(target.worksheet))) {
            return error;
        }
    }
    return std::nullopt;
}

// ---------------------------------------------------------------------------
// The declared target type of a column type change
// ---------------------------------------------------------------------------

std::optional<Error> read_target_type(const Args& args, LogicalType& out, std::string context) {
    std::string spelling;
    if (auto error = args.text(argument::kTargetType, spelling)) {
        return error;
    }
    auto parsed = logical_type_from_text(spelling);
    if (!parsed.ok()) {
        return annotate(parsed.diagnostic(), std::move(context));
    }
    out = parsed.take();
    return std::nullopt;
}

std::optional<Error> read_policy(const Args& args, ConversionPolicy& out, std::string context) {
    std::optional<std::string> spelling;
    if (auto error = args.optional_text(argument::kConversion, spelling)) {
        return error;
    }
    if (!spelling.has_value()) {
        return std::nullopt;
    }
    auto parsed = conversion_policy_from_text(*spelling);
    if (!parsed.ok()) {
        return annotate(parsed.diagnostic(), std::move(context));
    }
    out = parsed.take();
    return std::nullopt;
}

// The argument shape both type commands share. A type name or a conversion policy the table does
// not have is refused before the transaction reads any column, so a mistyped request never
// reaches a stored value.
std::optional<Error> check_type_change_arguments(const Args& args) {
    std::uint64_t column = 0;
    if (auto error = args.id(argument::kColumn, column)) {
        return error;
    }
    std::string spelling;
    if (auto error = args.text(argument::kTargetType, spelling)) {
        return error;
    }
    auto type = logical_type_from_text(spelling);
    if (!type.ok()) {
        return type.diagnostic().to_error();
    }
    std::optional<std::string> conversion;
    if (auto error = args.optional_text(argument::kConversion, conversion)) {
        return error;
    }
    if (conversion.has_value()) {
        auto policy = conversion_policy_from_text(*conversion);
        if (!policy.ok()) {
            return policy.diagnostic().to_error();
        }
    }
    for (std::string_view key :
         {std::string_view{argument::kUnit}, std::string_view{argument::kDictionary},
          std::string_view{argument::kZone}}) {
        std::optional<std::string> unused;
        if (auto error = args.optional_text(key, unused)) {
            return error;
        }
    }
    return std::nullopt;
}

// The successor field a type change declares. The identity and the name of the field survive,
// because a type change restates the values, not the column. A unit is kept for a numeric
// target and dropped only when it is absent or dimensionless, so a measurement is never
// quietly reinterpreted; a dictionary is required by a categorical target and accepted by no
// other; a zone is declared only by a zoned timestamp.
std::optional<Error> build_typed_field(const FieldSchema& current, LogicalType target,
                                       const std::optional<Unit>& declared_unit,
                                       const std::optional<std::vector<std::string>>& labels,
                                       const std::optional<std::string>& zone, FieldSchema& out,
                                       std::string context) {
    out = current;
    out.type = target;
    out.zone_id = std::nullopt;
    out.dictionary = nullptr;
    if (target != LogicalType::categorical && labels.has_value()) {
        return fail(DetailCode::type_mismatch,
                    "a dictionary is declared only by a categorical field", std::move(context));
    }
    if (target != LogicalType::timestamp_zoned && zone.has_value()) {
        return fail(DetailCode::type_mismatch,
                    "a zone is declared only by a zoned timestamp field", std::move(context));
    }
    if (target == LogicalType::categorical) {
        std::vector<std::string> chosen = labels.value_or(std::vector<std::string>{});
        if (chosen.empty()) {
            if (current.type != LogicalType::categorical || current.dictionary == nullptr) {
                return fail(DetailCode::unknown_category,
                            "a categorical target needs the dictionary labels it resolves against",
                            std::move(context));
            }
            chosen = current.dictionary->labels;
        }
        auto created = Dictionary::create(std::move(chosen));
        if (!created.ok()) {
            return annotate(created.diagnostic(), std::move(context));
        }
        out.dictionary = std::make_shared<const Dictionary>(created.take());
    }
    if (target == LogicalType::timestamp_zoned) {
        const std::string chosen = zone.value_or(current.zone_id.value_or(std::string{}));
        if (chosen.empty()) {
            return fail(DetailCode::unknown_zone,
                        "a zoned timestamp target must declare its zone identifier",
                        std::move(context));
        }
        if (!TimeZoneRules::builtin().contains(chosen)) {
            return fail(DetailCode::unknown_zone,
                        "the declared zone is not in the versioned rule table",
                        std::move(context) + ": " + chosen);
        }
        out.zone_id = chosen;
    }
    if (is_numeric(target)) {
        if (declared_unit.has_value()) {
            if (current.unit.has_value() && !(current.unit->canonical_text() ==
                                              declared_unit->canonical_text())) {
                return fail(DetailCode::incompatible_unit,
                            "a type change restates the values and never rescales them, so the "
                            "unit it declares must be the unit the field already holds; converting "
                            "a unit is its own operation",
                            std::move(context) + ": " + current.unit->canonical_text() + " to " +
                                declared_unit->canonical_text());
            }
            out.unit = declared_unit;
        }
        return std::nullopt;
    }
    if (current.unit.has_value() && !current.unit->is_dimensionless()) {
        return fail(DetailCode::incompatible_unit,
                    "a text, categorical, temporal or flag field carries no measurement unit, so a "
                    "typed column holding a dimensioned unit cannot change into one without "
                    "dropping what its values mean",
                    std::move(context) + ": " + current.unit->canonical_text());
    }
    out.unit = declared_unit;
    if (out.unit.has_value() && !out.unit->is_dimensionless()) {
        return fail(DetailCode::incompatible_unit,
                    "a non-numeric field declares only a dimensionless unit", std::move(context));
    }
    return std::nullopt;
}

// The dictionary labels of a categorical target, spelled as a one-row value block of text forms.
std::optional<Error> read_labels(const Args& args, std::optional<std::vector<std::string>>& out,
                                 std::string context) {
    std::optional<std::string> spelling;
    if (auto error = args.optional_text(argument::kDictionary, spelling)) {
        return error;
    }
    if (!spelling.has_value()) {
        return std::nullopt;
    }
    auto parsed = parse_grid(*spelling);
    if (!parsed.ok()) {
        return annotate(parsed.diagnostic(), std::move(context));
    }
    std::vector<std::string> labels;
    for (const auto& entry : parsed.value()) {
        if (entry.kind != 't') {
            return fail(DetailCode::type_mismatch,
                        "a dictionary label is a text form, so an unseen category cannot be spelled "
                        "as a number or a code",
                        std::move(context));
        }
        labels.push_back(entry.payload);
    }
    out = std::move(labels);
    return std::nullopt;
}

std::optional<Error> read_unit_argument(const Args& args, std::optional<Unit>& out,
                                        std::string context) {
    std::optional<std::string> spelling;
    if (auto error = args.optional_text(argument::kUnit, spelling)) {
        return error;
    }
    if (!spelling.has_value()) {
        return std::nullopt;
    }
    auto parsed = UnitRegistry::builtin().parse(*spelling);
    if (!parsed.ok()) {
        return annotate(parsed.diagnostic(), std::move(context));
    }
    out = parsed.take();
    return std::nullopt;
}

// What one column type change declares: the column it reads, the field the successor holds, and
// the policy for a value the target cannot hold exactly.
struct TypeChange {
    std::uint64_t worksheet{0};
    std::uint64_t column_id{0};
    ColumnEdit column;
    FieldSchema field;
    ConversionPolicy policy{ConversionPolicy::reject};
};

std::optional<Error> read_type_change(const Args& args, DataStore& store,
                                      const CommandTransaction& transaction, TypeChange& out,
                                      std::string context) {
    if (auto error = args.id(argument::kColumn, out.column_id)) {
        return error;
    }
    out.field = FieldSchema{};
    if (auto error = load_one_column(transaction, store, out.column_id, out.column)) {
        return error;
    }
    const ObjectState* state = transaction.find(ObjectId{out.column_id});
    if (state == nullptr) {
        return fail(DetailCode::dangling_reference, "the column of a type change is a document object",
                    "column " + std::to_string(out.column_id));
    }
    if (auto error = require_property(*state, property::kParent, out.worksheet,
                                      "column " + std::to_string(out.column_id))) {
        return error;
    }
    LogicalType target = out.column.field.type;
    if (auto error = read_target_type(args, target, context)) {
        return error;
    }
    if (auto error = read_policy(args, out.policy, context)) {
        return error;
    }
    std::optional<Unit> declared_unit;
    if (auto error = read_unit_argument(args, declared_unit, context)) {
        return error;
    }
    std::optional<std::vector<std::string>> labels;
    if (auto error = read_labels(args, labels, context)) {
        return error;
    }
    std::optional<std::string> zone;
    if (auto error = args.optional_text(argument::kZone, zone)) {
        return error;
    }
    return build_typed_field(out.column.field, target, declared_unit, labels, zone, out.field,
                             std::move(context));
}

// Resolves every stored value against the declared target field. The preview command runs
// exactly this and writes nothing, so a preview and a rejected change report the same first
// value that cannot arrive.
std::optional<Error> resolve_type_change(TypeChange& change) {
    change.column.to.clear();
    change.column.to.reserve(change.column.from.size());
    for (std::size_t index = 0; index < change.column.from.size(); ++index) {
        auto cell = convert_cell(change.column.from[index], change.column.field, change.field,
                                 change.policy, static_cast<std::uint64_t>(index));
        if (!cell.ok()) {
            return annotate(
                cell.diagnostic(),
                "column " + foundation::to_string(ObjectId{change.column_id}) + ", row " +
                    std::to_string(index));
        }
        change.column.to.push_back(cell.take());
    }
    return std::nullopt;
}

// Reads the row identity a single-cell edit addresses and reports its index in the worksheet's
// own row domain, so an edit never travels as an array position a re-sort could invalidate.
std::optional<Error> row_index_of(const Worksheet& sheet, std::uint64_t row, std::size_t& out,
                                  std::string context) {
    const auto found = sheet.rows->index_of(row_id(row));
    if (!found.has_value()) {
        return fail(DetailCode::dangling_reference,
                    "this row identity is not in the worksheet's row domain",
                    std::move(context) + "; row " + std::to_string(row) + " of domain " +
                        to_string(sheet.rows->id()));
    }
    out = static_cast<std::size_t>(*found);
    return std::nullopt;
}

std::optional<Error> unique_ids(const std::vector<std::uint64_t>& ids, std::string context) {
    std::vector<std::uint64_t> sorted = ids;
    std::sort(sorted.begin(), sorted.end());
    if (std::unique(sorted.begin(), sorted.end()) != sorted.end()) {
        return fail(DetailCode::duplicate_id, "a list of identities names each one once",
                    std::move(context));
    }
    return std::nullopt;
}

std::string id_list_text(const std::vector<std::uint64_t>& ids) {
    std::string text;
    for (const std::uint64_t id : ids) {
        if (!text.empty()) {
            text += ";";
        }
        text += std::to_string(id);
    }
    return text;
}

void register_definition(application::CommandRegistry& registry,
                         application::CommandDefinition definition, std::optional<Error>& first) {
    if (first.has_value()) {
        return;
    }
    definition.supported_schema_versions = {kSchemaVersion};
    const std::string name = definition.command_id;
    auto error = registry.register_command(std::move(definition));
    if (error.has_value()) {
        first = fail(DetailCode::duplicate_id, error->message, name);
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// The declared text form of a typed value
// ---------------------------------------------------------------------------

std::string ValueForm::encode() const {
    std::string text;
    text += kind;
    text += std::to_string(payload.size());
    text += ':';
    text += payload;
    return text;
}

ValueForm missing_form() {
    return form('n', std::string{});
}

ValueForm signed_form(std::int64_t value) {
    return form('i', std::to_string(value));
}

ValueForm unsigned_form(std::uint64_t value) {
    return form('u', std::to_string(value));
}

ValueForm float_form(double value) {
    return form('f', float_text(value));
}

ValueForm bits_form(std::uint64_t value) {
    return form('b', hex_text(value));
}

ValueForm text_form(std::string value) {
    return form('t', std::move(value));
}

ValueForm category_form(std::string label) {
    return form('c', std::move(label));
}

ValueForm code_form(std::int64_t code) {
    return form('q', std::to_string(code));
}

std::string encode_value(const ValueForm& value) {
    return value.encode();
}

std::string encode_grid(std::uint64_t rows, std::uint64_t columns, std::vector<ValueForm> values) {
    std::string text = std::to_string(rows) + "x" + std::to_string(columns) + ";";
    for (const auto& value : values) {
        text += value.encode();
    }
    return text;
}

Result<ValueForm> parse_value(std::string_view text) {
    std::string_view remainder = text;
    ValueForm out;
    if (auto refusal = read_form(remainder, out)) {
        return Result<ValueForm>::failure(*refusal);
    }
    if (!remainder.empty()) {
        return Result<ValueForm>::failure(
            reject(DetailCode::type_mismatch, "one value form was expected and text follows it",
                   std::string{text}));
    }
    return out;
}

Result<std::vector<ValueForm>> parse_grid(std::string_view text) {
    std::uint64_t rows = 0;
    std::uint64_t columns = 0;
    std::string_view remainder = text;
    if (auto refusal = read_grid_header(remainder, rows, columns)) {
        return Result<std::vector<ValueForm>>::failure(*refusal);
    }
    const std::size_t expected = static_cast<std::size_t>(rows * columns);
    std::vector<ValueForm> cells;
    cells.reserve(expected);
    while (!remainder.empty()) {
        ValueForm value;
        if (auto refusal = read_form(remainder, value)) {
            return Result<std::vector<ValueForm>>::failure(*refusal);
        }
        cells.push_back(std::move(value));
        if (cells.size() > expected) {
            break;
        }
    }
    if (!remainder.empty() || cells.size() != expected) {
        return Result<std::vector<ValueForm>>::failure(reject(
            DetailCode::unmapped_row, "a value grid of " + std::to_string(expected) +
                                          " cells holds " + std::to_string(cells.size()) +
                                          " complete forms",
            std::string{text}));
    }
    return cells;
}

Result<LogicalType> logical_type_from_text(std::string_view text) {
    static constexpr std::array<std::pair<std::string_view, LogicalType>, 17> kTypes{{
        {std::string_view{"int8"}, LogicalType::int8},
        {std::string_view{"int16"}, LogicalType::int16},
        {std::string_view{"int32"}, LogicalType::int32},
        {std::string_view{"int64"}, LogicalType::int64},
        {std::string_view{"uint8"}, LogicalType::uint8},
        {std::string_view{"uint16"}, LogicalType::uint16},
        {std::string_view{"uint32"}, LogicalType::uint32},
        {std::string_view{"uint64"}, LogicalType::uint64},
        {std::string_view{"float32"}, LogicalType::float32},
        {std::string_view{"float64"}, LogicalType::float64},
        {std::string_view{"boolean"}, LogicalType::boolean},
        {std::string_view{"text"}, LogicalType::text},
        {std::string_view{"categorical"}, LogicalType::categorical},
        {std::string_view{"date"}, LogicalType::date},
        {std::string_view{"duration"}, LogicalType::duration},
        {std::string_view{"timestamp_naive"}, LogicalType::timestamp_naive},
        {std::string_view{"timestamp_zoned"}, LogicalType::timestamp_zoned},
    }};
    for (const auto& entry : kTypes) {
        if (entry.first == text) {
            return entry.second;
        }
    }
    return Result<LogicalType>::failure(
        reject(DetailCode::type_mismatch, "no declared logical type is spelled like this",
               std::string{text}));
}

const char* to_string(ConversionPolicy policy) noexcept {
    switch (policy) {
        case ConversionPolicy::reject:
            return "reject";
        case ConversionPolicy::round:
            return "round";
    }
    return "unknown";
}

Result<ConversionPolicy> conversion_policy_from_text(std::string_view text) {
    if (text == "reject") {
        return ConversionPolicy::reject;
    }
    if (text == "round") {
        return ConversionPolicy::round;
    }
    return Result<ConversionPolicy>::failure(
        reject(DetailCode::type_mismatch, "a conversion policy is `reject` or `round`",
               std::string{text}));
}

std::optional<Diagnostic> register_worksheet_edit_commands(
    application::CommandRegistry& registry, DataStore& store, const DraftRegistry& drafts) {
    const auto report = [](std::optional<Error> error) -> std::optional<Diagnostic> {
        if (!error.has_value()) {
            return std::nullopt;
        }
        Diagnostic diagnostic;
        diagnostic.detail = DetailCode::type_mismatch;
        diagnostic.outer = error->code;
        diagnostic.severity = Severity::error;
        diagnostic.location.description = error->message;
        return diagnostic;
    };
    std::optional<Error> first;

    // The row-shape family: one command states what the user did, and this is where the revised
    // row domain, the successor chunk and the successor version of every same-domain column are
    // built, so a table view never assembles immutable payloads itself.

    // data.worksheet_row_insert
    {
        application::CommandDefinition definition;
        definition.command_id = command_id::kWorksheetRowInsert;
        definition.effect = application::EffectClass::reversible_document_write;
        definition.repeatability = application::Repeatability::not_repeatable;
        definition.validate_arguments = [](const ParameterMap& arguments) -> std::optional<Error> {
            const Args args{arguments};
            if (auto error = args.all_ids({argument::kWorksheet, argument::kRevisedDomain})) {
                return error;
            }
            std::uint64_t row_index = 0;
            if (auto error = args.position(argument::kRowIndex, row_index)) {
                return error;
            }
            std::vector<std::uint64_t> rows;
            if (auto error = args.ids(argument::kRows, rows)) {
                return error;
            }
            if (rows.empty()) {
                return fail(DetailCode::unmapped_row,
                            "an insert names at least one new row identity",
                            "row index " + std::to_string(row_index));
            }
            if (auto error = unique_ids(rows, "row insert")) {
                return error;
            }
            std::vector<std::uint64_t> versions;
            std::vector<std::uint64_t> chunks;
            if (auto error = args.ids(argument::kSuccessorVersions, versions)) {
                return error;
            }
            if (auto error = args.ids(argument::kSuccessorChunks, chunks)) {
                return error;
            }
            if (versions.size() != chunks.size()) {
                return fail(DetailCode::unmapped_row,
                            "an insert names one successor version and one successor chunk per "
                            "column, so its two identity lists must be the same length",
                            command_id::kWorksheetRowInsert);
            }
            return std::nullopt;
        };
        definition.execute = [&store](CommandTransaction& transaction,
                                       const CommandRequest& request,
                                       const ExecutionContext& context) -> std::optional<Error> {
            const Args args{request.arguments};
            std::uint64_t worksheet = 0;
            std::uint64_t revised_domain = 0;
            std::uint64_t row_index = 0;
            std::vector<std::uint64_t> rows;
            std::optional<std::string> block;
            if (auto error = args.id(argument::kWorksheet, worksheet)) {
                return error;
            }
            if (auto error = args.id(argument::kRevisedDomain, revised_domain)) {
                return error;
            }
            if (auto error = args.position(argument::kRowIndex, row_index)) {
                return error;
            }
            if (auto error = args.ids(argument::kRows, rows)) {
                return error;
            }
            if (auto error = args.optional_text(argument::kValues, block)) {
                return error;
            }
            Worksheet sheet;
            if (auto error = open_worksheet(transaction, store, worksheet, sheet)) {
                return error;
            }
            std::vector<ColumnEdit> columns;
            if (auto error = load_columns(transaction, store, sheet, columns)) {
                return error;
            }
            if (auto error = read_successor_ids(args, 0, columns.size(),
                                               "worksheet " + std::to_string(worksheet), columns)) {
                return error;
            }
            const std::size_t current = static_cast<std::size_t>(sheet.rows->row_count());
            if (row_index > sheet.rows->row_count()) {
                return fail(DetailCode::index_out_of_range,
                            "a row insert addresses a position between the first row and one past "
                            "the last, and this one names " +
                                std::to_string(row_index) + " of " + std::to_string(current),
                            "worksheet " + std::to_string(worksheet));
            }
            for (const std::uint64_t row : rows) {
                if (sheet.rows->index_of(row_id(row)).has_value()) {
                    return fail(DetailCode::duplicate_id,
                                "an inserted row takes a new identity, because a row identity is "
                                "stable within a dataset lineage and cannot name two rows",
                                "row " + std::to_string(row));
                }
            }
            RowEdit edit;
            const auto& existing = sheet.rows->rows();
            const auto at = static_cast<std::size_t>(row_index);
            // The copy plan is one entry per revised row: its origin index in the old domain, or
            // no origin for a row this command introduces.
            for (std::size_t index = 0; index < at; ++index) {
                edit.rows.push_back(existing[index]);
                edit.origins.push_back(index);
            }
            for (const std::uint64_t row : rows) {
                edit.rows.push_back(row_id(row));
                edit.origins.push_back(std::nullopt);
            }
            for (std::size_t index = at; index < existing.size(); ++index) {
                edit.rows.push_back(existing[index]);
                edit.origins.push_back(index);
            }
            if (auto error = remap_rows(edit, columns,
                                        "worksheet " + std::to_string(worksheet))) {
                return error;
            }
            if (block.has_value()) {
                BlockTarget target;
                target.first_row = row_index;
                target.first_column = 0;
                target.rows = static_cast<std::uint64_t>(rows.size());
                target.columns = static_cast<std::uint64_t>(columns.size());
                target.row_count = static_cast<std::uint64_t>(edit.rows.size());
                if (auto error = write_block(*block, target, columns,
                                             "worksheet " + std::to_string(worksheet))) {
                    return error;
                }
            }
            EditTarget out;
            out.worksheet = worksheet;
            out.revised_domain = revised_domain;
            out.rows_named = true;
            out.rows = &edit;
            out.operation = command_id::kWorksheetRowInsert;
            out.parameters = "row_index=" + std::to_string(row_index) + ";rows=" +
                             id_list_text(rows) + (block.has_value() ? ";values" : "");
            return publish_edit(transaction, store, context, sheet, out, columns, "row insert");
        };
        register_definition(registry, std::move(definition), first);
    }

    // data.worksheet_row_delete
    {
        application::CommandDefinition definition;
        definition.command_id = command_id::kWorksheetRowDelete;
        definition.effect = application::EffectClass::reversible_document_write;
        definition.repeatability = application::Repeatability::not_repeatable;
        definition.validate_arguments = [](const ParameterMap& arguments) -> std::optional<Error> {
            const Args args{arguments};
            if (auto error = args.all_ids({argument::kWorksheet, argument::kRevisedDomain})) {
                return error;
            }
            std::vector<std::uint64_t> rows;
            if (auto error = args.ids(argument::kRows, rows)) {
                return error;
            }
            if (rows.empty()) {
                return fail(DetailCode::unmapped_row,
                            "a delete names at least one row identity to remove", "rows");
            }
            return unique_ids(rows, "row delete");
        };
        definition.execute = [&store](CommandTransaction& transaction,
                                     const CommandRequest& request,
                                     const ExecutionContext& context) -> std::optional<Error> {
            const Args args{request.arguments};
            std::uint64_t worksheet = 0;
            std::uint64_t revised_domain = 0;
            std::vector<std::uint64_t> rows;
            if (auto error = args.id(argument::kWorksheet, worksheet)) {
                return error;
            }
            if (auto error = args.id(argument::kRevisedDomain, revised_domain)) {
                return error;
            }
            if (auto error = args.ids(argument::kRows, rows)) {
                return error;
            }
            Worksheet sheet;
            if (auto error = open_worksheet(transaction, store, worksheet, sheet)) {
                return error;
            }
            std::vector<ColumnEdit> columns;
            if (auto error = load_columns(transaction, store, sheet, columns)) {
                return error;
            }
            if (auto error = read_successor_ids(args, 0, columns.size(),
                                               "worksheet " + std::to_string(worksheet), columns)) {
                return error;
            }
            std::vector<std::size_t> removed;
            for (const std::uint64_t row : rows) {
                const auto index = sheet.rows->index_of(row_id(row));
                if (!index.has_value()) {
                    return fail(DetailCode::dangling_reference,
                                "a deleted row must be an identity of this worksheet's row domain",
                                "row " + std::to_string(row) + " of domain " +
                                    to_string(sheet.rows->id()));
                }
                removed.push_back(static_cast<std::size_t>(*index));
            }
            std::sort(removed.begin(), removed.end());
            RowEdit edit;
            const auto& existing = sheet.rows->rows();
            for (std::size_t index = 0; index < existing.size(); ++index) {
                if (std::binary_search(removed.begin(), removed.end(), index)) {
                    continue;
                }
                edit.rows.push_back(existing[index]);
                edit.origins.push_back(index);
            }
            if (auto error = remap_rows(edit, columns,
                                        "worksheet " + std::to_string(worksheet))) {
                return error;
            }
            EditTarget out;
            out.worksheet = worksheet;
            out.revised_domain = revised_domain;
            out.rows_named = true;
            out.rows = &edit;
            out.operation = command_id::kWorksheetRowDelete;
            out.parameters = "rows=" + id_list_text(rows);
            return publish_edit(transaction, store, context, sheet, out, columns, "row delete");
        };
        register_definition(registry, std::move(definition), first);
    }

    // data.worksheet_row_move
    {
        application::CommandDefinition definition;
        definition.command_id = command_id::kWorksheetRowMove;
        definition.effect = application::EffectClass::reversible_document_write;
        // A second move of the same positions moves what now sits there, so the request is not
        // idempotent and repeating it would be a different edit rather than a no-op.
        definition.repeatability = application::Repeatability::not_repeatable;
        definition.validate_arguments = [](const ParameterMap& arguments) -> std::optional<Error> {
            const Args args{arguments};
            std::uint64_t position = 0;
            if (auto error = args.all_ids({argument::kWorksheet, argument::kRevisedDomain})) {
                return error;
            }
            if (auto error = args.position(argument::kRowIndex, position)) {
                return error;
            }
            if (auto error = args.position(argument::kLength, position)) {
                return error;
            }
            return args.position(argument::kTargetRowIndex, position);
        };
        definition.execute = [&store](CommandTransaction& transaction,
                                     const CommandRequest& request,
                                     const ExecutionContext& context) -> std::optional<Error> {
            const Args args{request.arguments};
            std::uint64_t worksheet = 0;
            std::uint64_t revised_domain = 0;
            std::uint64_t row_index = 0;
            std::uint64_t length = 0;
            std::uint64_t target_row_index = 0;
            if (auto error = args.id(argument::kWorksheet, worksheet)) {
                return error;
            }
            if (auto error = args.id(argument::kRevisedDomain, revised_domain)) {
                return error;
            }
            if (auto error = args.position(argument::kRowIndex, row_index)) {
                return error;
            }
            if (auto error = args.position(argument::kLength, length)) {
                return error;
            }
            if (auto error = args.position(argument::kTargetRowIndex, target_row_index)) {
                return error;
            }
            Worksheet sheet;
            if (auto error = open_worksheet(transaction, store, worksheet, sheet)) {
                return error;
            }
            std::vector<ColumnEdit> columns;
            if (auto error = load_columns(transaction, store, sheet, columns)) {
                return error;
            }
            if (auto error = read_successor_ids(args, 0, columns.size(),
                                               "worksheet " + std::to_string(worksheet), columns)) {
                return error;
            }
            const std::size_t count = static_cast<std::size_t>(sheet.rows->row_count());
            const std::size_t start = static_cast<std::size_t>(row_index);
            const std::size_t moved = static_cast<std::size_t>(length);
            // Each bound is a subtraction, never a sum: two positions the caller names can add
            // past the width of the count and wrap back into range, which would reach the row
            // list below instead of being refused here.
            if (moved == 0 || moved > count || start > count - moved) {
                return fail(DetailCode::index_out_of_range,
                            "a row move names a run of at least one row inside this worksheet",
                            "worksheet " + std::to_string(worksheet) + ": " +
                                std::to_string(row_index) + " plus " + std::to_string(length) +
                                " of " + std::to_string(count));
            }
            if (target_row_index > count - moved) {
                return fail(DetailCode::index_out_of_range,
                            "the destination is a position in the row list without the moved run, "
                            "so it cannot pass one past its last row",
                            "worksheet " + std::to_string(worksheet) + ": " +
                                std::to_string(target_row_index) + " of " +
                                std::to_string(count - moved));
            }
            const auto& existing = sheet.rows->rows();
            std::vector<std::optional<std::size_t>> rest;
            rest.reserve(count - moved);
            for (std::size_t index = 0; index < count; ++index) {
                if (index < start || index >= start + moved) {
                    rest.push_back(index);
                }
            }
            const auto into = static_cast<std::size_t>(target_row_index);
            RowEdit edit;
            const auto append = [&edit, &existing](std::optional<std::size_t> origin) {
                edit.origins.push_back(origin);
                edit.rows.push_back(existing[*origin]);
            };
            for (std::size_t index = 0; index < into; ++index) {
                append(rest[index]);
            }
            for (std::size_t index = start; index < start + moved; ++index) {
                append(index);
            }
            for (std::size_t index = into; index < rest.size(); ++index) {
                append(rest[index]);
            }
            if (auto error = remap_rows(edit, columns,
                                        "worksheet " + std::to_string(worksheet))) {
                return error;
            }
            EditTarget out;
            out.worksheet = worksheet;
            out.revised_domain = revised_domain;
            out.rows_named = true;
            out.rows = &edit;
            out.operation = command_id::kWorksheetRowMove;
            out.parameters = "row_index=" + std::to_string(row_index) + ";length=" +
                             std::to_string(length) + ";target_row_index=" +
                             std::to_string(target_row_index);
            return publish_edit(transaction, store, context, sheet, out, columns, "row move");
        };
        register_definition(registry, std::move(definition), first);
    }

    // data.worksheet_resize
    {
        application::CommandDefinition definition;
        definition.command_id = command_id::kWorksheetResize;
        definition.effect = application::EffectClass::reversible_document_write;
        definition.repeatability = application::Repeatability::repeatable;
        definition.validate_arguments = [](const ParameterMap& arguments) -> std::optional<Error> {
            const Args args{arguments};
            std::uint64_t target = 0;
            if (auto error = args.all_ids({argument::kWorksheet, argument::kRevisedDomain})) {
                return error;
            }
            if (auto error = args.position(argument::kTargetRowCount, target)) {
                return error;
            }
            std::vector<std::uint64_t> rows;
            if (auto error = args.optional_ids(argument::kRows, rows)) {
                return error;
            }
            std::vector<std::uint64_t> versions;
            std::vector<std::uint64_t> chunks;
            if (auto error = args.ids(argument::kSuccessorVersions, versions)) {
                return error;
            }
            if (auto error = args.ids(argument::kSuccessorChunks, chunks)) {
                return error;
            }
            if (versions.size() != chunks.size()) {
                return fail(DetailCode::unmapped_row,
                            "a resize names one successor version and one successor chunk per "
                            "column, so its two identity lists must be the same length",
                            command_id::kWorksheetResize);
            }
            return std::nullopt;
        };
        definition.execute = [&store](CommandTransaction& transaction,
                                     const CommandRequest& request,
                                     const ExecutionContext& context) -> std::optional<Error> {
            const Args args{request.arguments};
            std::uint64_t worksheet = 0;
            std::uint64_t revised_domain = 0;
            std::uint64_t target_rows = 0;
            std::vector<std::uint64_t> new_rows;
            std::optional<std::string> block;
            if (auto error = args.id(argument::kWorksheet, worksheet)) {
                return error;
            }
            if (auto error = args.id(argument::kRevisedDomain, revised_domain)) {
                return error;
            }
            if (auto error = args.position(argument::kTargetRowCount, target_rows)) {
                return error;
            }
            if (auto error = args.optional_ids(argument::kRows, new_rows)) {
                return error;
            }
            if (auto error = args.optional_text(argument::kValues, block)) {
                return error;
            }
            Worksheet sheet;
            if (auto error = open_worksheet(transaction, store, worksheet, sheet)) {
                return error;
            }
            std::vector<ColumnEdit> columns;
            if (auto error = load_columns(transaction, store, sheet, columns)) {
                return error;
            }
            if (auto error = read_successor_ids(args, 0, columns.size(),
                                               "worksheet " + std::to_string(worksheet), columns)) {
                return error;
            }
            const std::size_t count = static_cast<std::size_t>(sheet.rows->row_count());
            const std::size_t wanted = static_cast<std::size_t>(target_rows);
            RowEdit edit;
            const auto& existing = sheet.rows->rows();
            std::vector<std::uint64_t> introduced;
            if (wanted <= count) {
                if (!new_rows.empty()) {
                    return fail(DetailCode::unmapped_row,
                                "only a resize that grows names new row identities, and this one "
                                "keeps or drops rows",
                                "worksheet " + std::to_string(worksheet));
                }
                for (std::size_t index = 0; index < wanted; ++index) {
                    edit.rows.push_back(existing[index]);
                    edit.origins.push_back(index);
                }
            }
            if (wanted > count) {
                if (new_rows.empty()) {
                    return fail(DetailCode::unmapped_row,
                                "growing a worksheet names one new row identity for each row it "
                                "adds, because a row identity is never invented by position",
                                "worksheet " + std::to_string(worksheet));
                }
                const std::vector<std::uint64_t>& named = new_rows;
                if (named.size() != wanted - count) {
                    return fail(DetailCode::unmapped_row,
                                "a resize that grows names exactly the " +
                                    std::to_string(wanted - count) + " new row identities it adds",
                                "worksheet " + std::to_string(worksheet));
                }
                if (auto error = unique_ids(named, "row identities of a resize")) {
                    return error;
                }
                for (std::size_t index = 0; index < count; ++index) {
                    edit.rows.push_back(existing[index]);
                    edit.origins.push_back(index);
                }
                for (const std::uint64_t row : named) {
                    if (sheet.rows->index_of(row_id(row)).has_value()) {
                        return fail(DetailCode::duplicate_id,
                                    "a new row takes an identity this worksheet does not already "
                                    "use",
                                    "row " + std::to_string(row));
                    }
                    edit.rows.push_back(row_id(row));
                    edit.origins.push_back(std::nullopt);
                    introduced.push_back(row);
                }
            }
            if (auto error = remap_rows(edit, columns,
                                        "worksheet " + std::to_string(worksheet))) {
                return error;
            }
            if (block.has_value()) {
                if (introduced.empty()) {
                    return fail(DetailCode::unmapped_row,
                                "a value block belongs to the rows a resize introduces, and this "
                                "one introduces none",
                                "worksheet " + std::to_string(worksheet));
                }
                BlockTarget target;
                target.first_row = count;
                target.first_column = 0;
                target.rows = static_cast<std::uint64_t>(introduced.size());
                target.columns = static_cast<std::uint64_t>(columns.size());
                target.row_count = static_cast<std::uint64_t>(edit.rows.size());
                if (auto error = write_block(*block, target, columns,
                                             "worksheet " + std::to_string(worksheet))) {
                    return error;
                }
            }
            EditTarget out;
            out.worksheet = worksheet;
            out.revised_domain = revised_domain;
            out.rows_named = true;
            out.rows = &edit;
            out.operation = command_id::kWorksheetResize;
            out.parameters = "target_row_count=" + std::to_string(target_rows) +
                             (introduced.empty() ? "" : ";rows=" + id_list_text(introduced)) +
                             (block.has_value() ? ";values" : "");
            return publish_edit(transaction, store, context, sheet, out, columns, "resize");
        };
        register_definition(registry, std::move(definition), first);
    }

    // data.worksheet_cell_set
    {
        application::CommandDefinition definition;
        definition.command_id = command_id::kWorksheetCellSet;
        definition.effect = application::EffectClass::reversible_document_write;
        definition.repeatability = application::Repeatability::repeatable;
        definition.validate_arguments = [](const ParameterMap& arguments) -> std::optional<Error> {
            const Args args{arguments};
            return args.all_ids({argument::kColumn, argument::kRow});
        };
        definition.execute = [&store](CommandTransaction& transaction,
                                     const CommandRequest& request,
                                     const ExecutionContext& context) -> std::optional<Error> {
            const Args args{request.arguments};
            std::uint64_t column = 0;
            std::uint64_t row = 0;
            std::string value;
            std::optional<std::string> unit_text;
            if (auto error = args.id(argument::kColumn, column)) {
                return error;
            }
            if (auto error = args.id(argument::kRow, row)) {
                return error;
            }
            if (auto error = args.text(argument::kValue, value)) {
                return error;
            }
            if (auto error = args.optional_text(argument::kSourceUnit, unit_text)) {
                return error;
            }
            ColumnEdit edit;
            if (auto error = load_one_column(transaction, store, column, edit)) {
                return error;
            }
            const ObjectState* binding = transaction.find(ObjectId{column});
            std::uint64_t worksheet = 0;
            if (binding != nullptr) {
                if (auto error = require_property(*binding, property::kParent, worksheet,
                                                  "column " + std::to_string(column))) {
                    return error;
                }
            }
            Worksheet sheet;
            if (auto error = open_worksheet(transaction, store, worksheet, sheet)) {
                return error;
            }
            std::vector<ColumnEdit> columns{std::move(edit)};
            if (auto error = read_successor_ids(args, 0, 1, "column " + std::to_string(column),
                                               columns)) {
                return error;
            }
            std::size_t index = 0;
            if (auto error = row_index_of(sheet, row, index, "column " + std::to_string(column))) {
                return error;
            }
            auto parsed = parse_value(value);
            if (!parsed.ok()) {
                return annotate(parsed.diagnostic(), "value of cell " + std::to_string(column) +
                                                          "-" + std::to_string(row));
            }
            Unit source_unit;
            const Unit* unit = nullptr;
            if (unit_text.has_value()) {
                auto parsed_unit = UnitRegistry::builtin().parse(*unit_text);
                if (!parsed_unit.ok()) {
                    return annotate(parsed_unit.diagnostic(),
                                    "source unit of cell " + std::to_string(column) + "-" +
                                        std::to_string(row));
                }
                source_unit = parsed_unit.take();
                unit = &source_unit;
            }
            auto cell = to_cell(columns.front().field, parsed.value(),
                                static_cast<std::uint64_t>(index), unit);
            if (!cell.ok()) {
                return annotate(cell.diagnostic(), "cell " + std::to_string(column) + "-" +
                                                        std::to_string(row));
            }
            if (index >= columns.front().to.size()) {
                return fail(DetailCode::index_out_of_range,
                            "the row of this cell is beyond the values the column holds",
                            "column " + std::to_string(column));
            }
            columns.front().to[index] = cell.take();
            EditTarget target;
            target.worksheet = worksheet;
            target.rows_named = false;
            target.operation = command_id::kWorksheetCellSet;
            target.parameters = "column=" + std::to_string(column) + ";row=" + std::to_string(row) +
                                ";value=" + value + (unit_text.has_value() ? ";source_unit" : "");
            return publish_edit(transaction, store, context, sheet, target, columns, "cell set");
        };
        register_definition(registry, std::move(definition), first);
    }

    // data.worksheet_cell_paste
    {
        application::CommandDefinition definition;
        definition.command_id = command_id::kWorksheetCellPaste;
        definition.effect = application::EffectClass::reversible_document_write;
        definition.repeatability = application::Repeatability::repeatable;
        definition.validate_arguments = [](const ParameterMap& arguments) -> std::optional<Error> {
            const Args args{arguments};
            if (auto error = args.all_ids({argument::kWorksheet, argument::kColumn,
                                           argument::kRow})) {
                return error;
            }
            std::string block;
            return args.text(argument::kValues, block);
        };
        definition.execute = [&store](CommandTransaction& transaction,
                                     const CommandRequest& request,
                                     const ExecutionContext& context) -> std::optional<Error> {
            const Args args{request.arguments};
            std::uint64_t worksheet = 0;
            std::uint64_t anchor_column = 0;
            std::uint64_t anchor_row = 0;
            std::string block;
            if (auto error = args.id(argument::kWorksheet, worksheet)) {
                return error;
            }
            if (auto error = args.id(argument::kColumn, anchor_column)) {
                return error;
            }
            if (auto error = args.id(argument::kRow, anchor_row)) {
                return error;
            }
            if (auto error = args.text(argument::kValues, block)) {
                return error;
            }
            Worksheet sheet;
            if (auto error = open_worksheet(transaction, store, worksheet, sheet)) {
                return error;
            }
            std::vector<ColumnEdit> columns;
            if (auto error = load_columns(transaction, store, sheet, columns)) {
                return error;
            }
            std::size_t position = 0;
            bool anchored = false;
            for (std::size_t index = 0; index < columns.size(); ++index) {
                if (columns[index].binding == anchor_column) {
                    position = index;
                    anchored = true;
                    break;
                }
            }
            if (!anchored) {
                return fail(DetailCode::dangling_reference,
                            "the anchor of a block paste is an editable same-domain column of this "
                            "worksheet",
                            "column " + std::to_string(anchor_column));
            }
            std::size_t first_row = 0;
            if (auto error = row_index_of(sheet, anchor_row, first_row,
                                          "worksheet " + std::to_string(worksheet))) {
                return error;
            }
            std::uint64_t grid_rows = 0;
            std::uint64_t grid_columns = 0;
            if (auto error = grid_shape(block, grid_rows, grid_columns)) {
                return error;
            }
            if (auto error = read_successor_ids(args, position, grid_columns,
                                                "worksheet " + std::to_string(worksheet), columns)) {
                return error;
            }
            BlockTarget target;
            target.first_row = first_row;
            target.first_column = position;
            target.rows = grid_rows;
            target.columns = grid_columns;
            target.row_count = sheet.rows->row_count();
            if (auto error = write_block(block, target, columns,
                                         "worksheet " + std::to_string(worksheet))) {
                return error;
            }
            EditTarget out;
            out.worksheet = worksheet;
            out.rows_named = false;
            out.operation = command_id::kWorksheetCellPaste;
            out.parameters = "column=" + std::to_string(anchor_column) + ";row=" +
                             std::to_string(anchor_row) + ";block=" + std::to_string(grid_rows) +
                             "x" + std::to_string(grid_columns);
            return publish_edit(transaction, store, context, sheet, out, columns, "block paste");
        };
        register_definition(registry, std::move(definition), first);
    }

    // data.worksheet_column_type_preview: the same resolution, with no write at all.
    {
        application::CommandDefinition definition;
        definition.command_id = command_id::kWorksheetColumnTypePreview;
        definition.effect = application::EffectClass::read_only;
        definition.repeatability = application::Repeatability::repeatable;
        definition.validate_arguments = [](const ParameterMap& arguments) -> std::optional<Error> {
            const Args args{arguments};
            return check_type_change_arguments(args);
        };
        definition.execute = [&store](CommandTransaction& transaction,
                                     const CommandRequest& request,
                                     const ExecutionContext&) -> std::optional<Error> {
            const Args args{request.arguments};
            TypeChange change;
            if (auto error = read_type_change(args, store, transaction, change, "preview")) {
                return error;
            }
            return resolve_type_change(change);
        };
        register_definition(registry, std::move(definition), first);
    }

    // data.worksheet_column_type_set
    {
        application::CommandDefinition definition;
        definition.command_id = command_id::kWorksheetColumnTypeSet;
        definition.effect = application::EffectClass::reversible_document_write;
        definition.repeatability = application::Repeatability::repeatable;
        definition.validate_arguments = [](const ParameterMap& arguments) -> std::optional<Error> {
            const Args args{arguments};
            if (auto error = check_type_change_arguments(args)) {
                return error;
            }
            std::vector<std::uint64_t> versions;
            std::vector<std::uint64_t> chunks;
            if (auto error = args.ids(argument::kSuccessorVersions, versions)) {
                return error;
            }
            return args.ids(argument::kSuccessorChunks, chunks);
        };
        definition.execute = [&store](CommandTransaction& transaction,
                                     const CommandRequest& request,
                                     const ExecutionContext& context) -> std::optional<Error> {
            const Args args{request.arguments};
            TypeChange change;
            if (auto error = read_type_change(args, store, transaction, change, "type change")) {
                return error;
            }
            if (auto error = resolve_type_change(change)) {
                return error;
            }
            const std::uint64_t column = change.column_id;
            change.column.field = change.field;
            std::vector<ColumnEdit> columns{std::move(change.column)};
            if (auto error = read_successor_ids(args, 0, 1, "column " + std::to_string(column),
                                                columns)) {
                return error;
            }
            Worksheet sheet;
            if (auto error = open_worksheet(transaction, store, change.worksheet, sheet)) {
                return error;
            }
            EditTarget target;
            target.worksheet = change.worksheet;
            target.rows_named = false;
            target.operation = command_id::kWorksheetColumnTypeSet;
            target.parameters = "column=" + std::to_string(column) + ";target_type=" +
                                to_string(change.field.type) +
                                ";conversion=" + to_string(change.policy);
            return publish_edit(transaction, store, context, sheet, target, columns, "type change");
        };
        register_definition(registry, std::move(definition), first);
    }

    // data.worksheet_column_insert: a new column at a position, with the columns after it
    // renumbered in the same revision.
    {
        application::CommandDefinition definition;
        definition.command_id = command_id::kWorksheetColumnInsert;
        definition.effect = application::EffectClass::reversible_document_write;
        definition.repeatability = application::Repeatability::not_repeatable;
        definition.validate_arguments = [](const ParameterMap& arguments) -> std::optional<Error> {
            const Args args{arguments};
            return args.all_ids({argument::kWorksheet, argument::kColumn, argument::kDataset,
                                 argument::kVersionDraft, argument::kOrderIndex});
        };
        definition.execute = [&store, &drafts](CommandTransaction& transaction,
                                              const CommandRequest& request,
                                              const ExecutionContext& context)
            -> std::optional<Error> {
            const Args args{request.arguments};
            std::uint64_t worksheet = 0;
            std::uint64_t column = 0;
            std::uint64_t dataset = 0;
            std::uint64_t draft_id = 0;
            std::uint64_t order_index = 0;
            std::string label;
            std::optional<std::string> role;
            std::optional<std::string> unit_text;
            std::optional<std::string> format;
            if (auto error = args.id(argument::kWorksheet, worksheet)) {
                return error;
            }
            if (auto error = args.id(argument::kColumn, column)) {
                return error;
            }
            if (auto error = args.id(argument::kDataset, dataset)) {
                return error;
            }
            if (auto error = args.id(argument::kVersionDraft, draft_id)) {
                return error;
            }
            if (auto error = args.position(argument::kOrderIndex, order_index)) {
                return error;
            }
            if (auto error = args.text(argument::kLabel, label, false)) {
                return error;
            }
            if (auto error = args.optional_text(argument::kRole, role)) {
                return error;
            }
            if (auto error = args.optional_text(argument::kUnit, unit_text)) {
                return error;
            }
            if (auto error = args.optional_text(argument::kDisplayFormat, format)) {
                return error;
            }
            if (auto error = check_text(label, "label of column " + std::to_string(column))) {
                return error;
            }
            if (role.has_value()) {
                if (auto error = check_choice(*role,
                                              {"unspecified", "x", "y", "z", "error", "label"},
                                              "role of column " + std::to_string(column))) {
                    return error;
                }
            }
            if (unit_text.has_value()) {
                auto parsed = UnitRegistry::builtin().parse(*unit_text);
                if (!parsed.ok()) {
                    return annotate(parsed.diagnostic(),
                                    "unit of column " + std::to_string(column));
                }
            }
            Worksheet sheet;
            if (auto error = open_worksheet(transaction, store, worksheet, sheet)) {
                return error;
            }
            if (auto error = check_unique(transaction, column, "column " + std::to_string(column))) {
                return error;
            }
            const auto bindings = bindings_in_display_order(transaction, worksheet);
            if (order_index > bindings.size()) {
                return fail(DetailCode::index_out_of_range,
                            "a column inserts at a position between the first column and one past "
                            "the last, and this one names " +
                                std::to_string(order_index) + " of " + std::to_string(bindings.size()),
                            "worksheet " + std::to_string(worksheet));
            }
            auto version = drafts.version(draft_id);
            if (version == nullptr) {
                return fail(DetailCode::unknown_draft,
                            "a new column names the candidate version it publishes",
                            "version_draft " + std::to_string(draft_id));
            }
            if (version->dataset() != dataset_id(dataset)) {
                return fail(DetailCode::type_mismatch,
                            "the candidate version belongs to another dataset",
                            "column " + std::to_string(column));
            }
            if (version->row_domain() != sheet.rows->id()) {
                return fail(DetailCode::positional_mapping_required,
                            "this column's rows come from another row-identity domain, so it must "
                            "be bound through an explicit correspondence",
                            "column " + std::to_string(column));
            }
            if (version->row_count() != sheet.rows->row_count()) {
                return fail(DetailCode::unmapped_row,
                            "the column's " + std::to_string(version->row_count()) +
                                " rows do not match the worksheet domain's " +
                                std::to_string(sheet.rows->row_count()) +
                                " rows, so nothing is zipped or padded",
                            "column " + std::to_string(column));
            }
            // Every binding at or after the slot gains one order index, in the same revision as
            // the new binding, so the sheet never shows two columns in the same slot.
            for (const auto& state : bindings) {
                const auto index = read_id_property(state, property::kOrderIndex);
                const std::uint64_t current = index.has_value() ? *index : 0ULL;
                if (current < order_index) {
                    continue;
                }
                if (auto error = write_property(transaction, state.id.value, property::kOrderIndex,
                                                std::to_string(current + 1),
                                                "column " + foundation::to_string(state.id))) {
                    return error;
                }
            }
            ParameterMap properties;
            set_id(properties, property::kParent, worksheet);
            set_id(properties, property::kDataset, dataset);
            set_id(properties, property::kVersion, version->id().raw());
            set_id(properties, property::kOrderIndex, order_index);
            set_text(properties, property::kWritePolicy, "mutable");
            set_text(properties, property::kLink, "same_domain");
            set_text(properties, property::kLabel, label);
            if (role.has_value()) {
                set_text(properties, property::kRole, *role);
            }
            if (unit_text.has_value()) {
                set_text(properties, property::kUnit, *unit_text);
            }
            if (format.has_value()) {
                set_text(properties, property::kDisplayFormat, *format);
            }
            if (auto error = create_object(transaction, column, object_type::kColumnBinding,
                                           std::move(properties),
                                           "column " + std::to_string(column))) {
                return error;
            }
            return publish_version(transaction, store, context.transaction_id, version,
                                   std::nullopt, "column " + std::to_string(column));
        };
        register_definition(registry, std::move(definition), first);
    }

    // data.worksheet_column_order_set: one revision for a whole reorder, including the columns
    // an edit never rewrites.
    {
        application::CommandDefinition definition;
        definition.command_id = command_id::kWorksheetColumnOrderSet;
        definition.effect = application::EffectClass::reversible_document_write;
        definition.repeatability = application::Repeatability::repeatable;
        definition.validate_arguments = [](const ParameterMap& arguments) -> std::optional<Error> {
            const Args args{arguments};
            std::uint64_t worksheet = 0;
            if (auto error = args.id(argument::kWorksheet, worksheet)) {
                return error;
            }
            std::vector<std::uint64_t> columns;
            if (auto error = args.ids(argument::kColumns, columns)) {
                return error;
            }
            if (columns.empty()) {
                return fail(DetailCode::unmapped_row,
                            "a declared order names every column of the worksheet",
                            "worksheet " + std::to_string(worksheet));
            }
            return unique_ids(columns, "declared column order");
        };
        definition.execute = [](CommandTransaction& transaction, const CommandRequest& request,
                               const ExecutionContext&) -> std::optional<Error> {
            const Args args{request.arguments};
            std::uint64_t worksheet = 0;
            std::vector<std::uint64_t> order;
            if (auto error = args.id(argument::kWorksheet, worksheet)) {
                return error;
            }
            if (auto error = args.ids(argument::kColumns, order)) {
                return error;
            }
            const ObjectState* state = nullptr;
            if (auto error = require_object(transaction, worksheet, object_type::kWorksheet, state,
                                            "worksheet " + std::to_string(worksheet))) {
                return error;
            }
            const auto bindings = bindings_in_display_order(transaction, worksheet);
            if (bindings.empty()) {
                return fail(DetailCode::unknown_field,
                            "this worksheet has no column to order",
                            "worksheet " + std::to_string(worksheet));
            }
            std::vector<std::uint64_t> held;
            held.reserve(bindings.size());
            for (const auto& binding : bindings) {
                held.push_back(binding.id.value);
            }
            std::vector<std::uint64_t> sorted = order;
            std::sort(sorted.begin(), sorted.end());
            std::sort(held.begin(), held.end());
            if (sorted != held) {
                return fail(DetailCode::unmapped_row,
                            "a declared order names each column of this worksheet exactly once, so "
                            "no column is dropped and none is invented",
                            "worksheet " + std::to_string(worksheet));
            }
            for (std::size_t index = 0; index < order.size(); ++index) {
                const auto* binding = transaction.find(ObjectId{order[index]});
                if (binding == nullptr) {
                    continue;
                }
                const auto current = read_id_property(*binding, property::kOrderIndex);
                if (current.has_value() && *current == index) {
                    continue;
                }
                if (auto error = write_property(transaction, order[index], property::kOrderIndex,
                                                std::to_string(index),
                                                "column " + foundation::to_string(ObjectId{
                                                    order[index]}))) {
                    return error;
                }
            }
            return std::nullopt;
        };
        register_definition(registry, std::move(definition), first);
    }

    if (first.has_value()) {
        return report(first);
    }
    return std::nullopt;
}

}  // namespace everygraph::data
