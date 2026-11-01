#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "everygraph/data/diagnostics.hpp"
#include "everygraph/data/logical_type.hpp"

namespace everygraph::data {

struct Missing {
    friend constexpr bool operator==(const Missing&, const Missing&) noexcept = default;
};
struct IntValue {
    std::int64_t value{0};
    friend constexpr bool operator==(const IntValue&, const IntValue&) noexcept = default;
};
struct UIntValue {
    std::uint64_t value{0};
    friend constexpr bool operator==(const UIntValue&, const UIntValue&) noexcept = default;
};
struct Float32Value {
    float value{0.0F};
    friend constexpr bool operator==(const Float32Value&, const Float32Value&) noexcept = default;
};
struct Float64Value {
    double value{0.0};
    friend constexpr bool operator==(const Float64Value&, const Float64Value&) noexcept = default;
};
struct BoolValue {
    bool value{false};
    friend constexpr bool operator==(const BoolValue&, const BoolValue&) noexcept = default;
};
struct TextValue {
    std::string value;
    friend bool operator==(const TextValue&, const TextValue&) = default;
};
struct CategoryValue {
    std::int64_t code{0};
    friend constexpr bool operator==(const CategoryValue&, const CategoryValue&) noexcept = default;
};
struct DateValue {
    std::int64_t days{0};
    friend constexpr bool operator==(const DateValue&, const DateValue&) noexcept = default;
};
struct DurationValue {
    std::int64_t microseconds{0};
    friend constexpr bool operator==(const DurationValue&, const DurationValue&) noexcept = default;
};
struct TimestampValue {
    std::int64_t microseconds{0};
    friend constexpr bool operator==(const TimestampValue&, const TimestampValue&) noexcept = default;
};

using CellPayload = std::variant<Missing, IntValue, UIntValue, Float32Value, Float64Value, BoolValue,
               TextValue, CategoryValue, DateValue, DurationValue, TimestampValue>;

class Cell;

// Change detection and structural identity: all NaN payloads compare equal, signed zeros
// stay distinct, and each state class stays distinguishable.
[[nodiscard]] bool structurally_equal(const Cell& left, const Cell& right) noexcept;

// IEEE numeric comparison. `nullopt` means unordered, which is what NaN and missing both
// produce; under IEEE rules negative zero and positive zero are equal.
[[nodiscard]] std::optional<bool> ieee_equal(const Cell& left, const Cell& right) noexcept;

// One stored value plus the logical type it is stored as. Missing, empty text, numeric
// zero, signed zero, NaN and infinity are distinct states here and are never
// interchangeable, so the payload is exposed rather than collapsed into a double.
class Cell {
public:
    Cell(LogicalType type, CellPayload payload) : type_(type), payload_(std::move(payload)) {}

    [[nodiscard]] static Cell missing(LogicalType type);

    [[nodiscard]] LogicalType type() const noexcept { return type_; }
    [[nodiscard]] const CellPayload& payload() const noexcept { return payload_; }
    [[nodiscard]] bool is_missing() const noexcept { return std::holds_alternative<Missing>(payload_); }

    [[nodiscard]] std::optional<std::int64_t> int_value() const noexcept;
    [[nodiscard]] std::optional<std::uint64_t> uint_value() const noexcept;
    [[nodiscard]] std::optional<float> float32_value() const noexcept;
    [[nodiscard]] std::optional<double> float64_value() const noexcept;
    [[nodiscard]] std::optional<bool> bool_value() const noexcept;
    [[nodiscard]] std::optional<std::string_view> text_value() const noexcept;
    [[nodiscard]] std::optional<std::int64_t> category_code() const noexcept;
    [[nodiscard]] std::optional<std::int64_t> tick_value() const noexcept;

    // IEEE observations for a floating payload. A NaN is a value, not missing and not an
    // error, and an infinity is never clipped.
    [[nodiscard]] bool is_nan() const noexcept;
    [[nodiscard]] bool is_infinity() const noexcept;
    [[nodiscard]] bool is_zero() const noexcept;
    [[nodiscard]] bool sign_bit() const noexcept;

    // The stored bit pattern, widened to 64 bits. A float32 pattern occupies the low 32
    // bits and reports its width through storage_bits(), so a payload never silently
    // promotes or demotes.
    [[nodiscard]] std::optional<std::uint64_t> bit_pattern() const noexcept;
    // Reads a pattern in the form bit_pattern() reports: the low storage bits are taken as the
    // type's value, so a signed narrow width arrives sign-extended. A builder refuses a form
    // whose high bits would be lost rather than masking them away.
    [[nodiscard]] static Cell from_bits(LogicalType type, std::uint64_t bits);

    friend bool operator==(const Cell& left, const Cell& right) noexcept {
        return structurally_equal(left, right);
    }

private:
    LogicalType type_{LogicalType::float64};
    CellPayload payload_{Missing{}};
};

[[nodiscard]] std::string describe(const Cell& cell);

enum class TextNulPolicy {
    reject,
    allow,
};

// Text storage keeps the exact code points: no trim, no case fold, no normalization.
// Invalid UTF-8 is rejected with its byte offset, and an embedded NUL is rejected unless
// a caller declares the allowing policy.
[[nodiscard]] Result<std::string> validate_text(std::string_view text,
                                                TextNulPolicy policy = TextNulPolicy::reject);

}  // namespace everygraph::data
