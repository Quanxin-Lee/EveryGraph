#include "everygraph/data/cell.hpp"

#include <bit>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>

namespace everygraph::data {
namespace {

constexpr std::uint64_t low_bits(std::uint64_t value, std::uint32_t width) noexcept {
    if (width >= 64U) {
        return value;
    }
    return value & ((1ULL << width) - 1ULL);
}

[[nodiscard]] std::uint64_t float32_bits(float value) noexcept {
    return static_cast<std::uint64_t>(std::bit_cast<std::uint32_t>(value));
}

[[nodiscard]] std::uint64_t float64_bits(double value) noexcept {
    return std::bit_cast<std::uint64_t>(value);
}

[[nodiscard]] std::int64_t sign_extend(std::uint64_t masked, std::uint32_t width) noexcept {
    if (width >= 64U) {
        return static_cast<std::int64_t>(masked);
    }
    const std::uint64_t sign = 1ULL << (width - 1U);
    if ((masked & sign) == 0U) {
        return static_cast<std::int64_t>(masked);
    }
    return static_cast<std::int64_t>(masked | ~low_bits(~std::uint64_t{0}, width));
}

// One UTF-8 sequence check. Overlong forms, surrogates and code points above U+10FFFF are
// rejected with the byte offset, and no replacement character is ever substituted.
[[nodiscard]] std::optional<std::int64_t> invalid_utf8_offset(std::string_view text) noexcept {
    std::size_t index = 0;
    while (index < text.size()) {
        const auto lead = static_cast<unsigned char>(text[index]);
        if (lead < 0x80U) {
            ++index;
            continue;
        }
        std::size_t width = 0;
        std::uint32_t minimum = 0;
        std::uint32_t code_point = 0;
        if (lead >= 0xC2U && lead <= 0xDFU) {
            width = 2;
            code_point = lead & 0x1FU;
            minimum = 0x80U;
        } else if (lead >= 0xE0U && lead <= 0xEFU) {
            width = 3;
            code_point = lead & 0x0FU;
            minimum = 0x800U;
        } else if (lead >= 0xF0U && lead <= 0xF4U) {
            width = 4;
            code_point = lead & 0x07U;
            minimum = 0x10000U;
        } else {
            return static_cast<std::int64_t>(index);
        }
        if (index + width > text.size()) {
            return static_cast<std::int64_t>(index);
        }
        for (std::size_t offset = 1; offset < width; ++offset) {
            const auto continuation = static_cast<unsigned char>(text[index + offset]);
            if ((continuation & 0xC0U) != 0x80U) {
                return static_cast<std::int64_t>(index + offset);
            }
            code_point = (code_point << 6U) | (continuation & 0x3FU);
        }
        if (code_point < minimum) {
            return static_cast<std::int64_t>(index);
        }
        if (code_point >= 0xD800U && code_point <= 0xDFFFU) {
            return static_cast<std::int64_t>(index);
        }
        if (code_point > 0x10FFFFU) {
            return static_cast<std::int64_t>(index);
        }
        index += width;
    }
    return std::nullopt;
}

[[nodiscard]] bool structurally_equal_payload(const CellPayload& left,
                                              const CellPayload& right) noexcept {
    if (left.index() != right.index()) {
        return false;
    }
    if (std::holds_alternative<Float32Value>(left)) {
        const auto& a = std::get<Float32Value>(left).value;
        const auto& b = std::get<Float32Value>(right).value;
        if (std::isnan(a) && std::isnan(b)) {
            return true;
        }
        return float32_bits(a) == float32_bits(b);
    }
    if (std::holds_alternative<Float64Value>(left)) {
        const auto& a = std::get<Float64Value>(left).value;
        const auto& b = std::get<Float64Value>(right).value;
        if (std::isnan(a) && std::isnan(b)) {
            return true;
        }
        return float64_bits(a) == float64_bits(b);
    }
    return left == right;
}

[[nodiscard]] std::optional<bool> ieee_equal_payload(const CellPayload& left,
                                                    const CellPayload& right) noexcept {
    if (std::holds_alternative<Missing>(left) || std::holds_alternative<Missing>(right)) {
        return std::nullopt;
    }
    if (left.index() != right.index()) {
        return std::nullopt;
    }
    if (std::holds_alternative<Float32Value>(left)) {
        const auto a = std::get<Float32Value>(left).value;
        const auto b = std::get<Float32Value>(right).value;
        if (std::isnan(a) || std::isnan(b)) {
            return std::nullopt;
        }
        return a == b;
    }
    if (std::holds_alternative<Float64Value>(left)) {
        const auto a = std::get<Float64Value>(left).value;
        const auto b = std::get<Float64Value>(right).value;
        if (std::isnan(a) || std::isnan(b)) {
            return std::nullopt;
        }
        return a == b;
    }
    return left == right;
}

}  // namespace

Cell Cell::missing(LogicalType type) {
    return Cell{type, Missing{}};
}

std::optional<std::int64_t> Cell::int_value() const noexcept {
    if (const auto* value = std::get_if<IntValue>(&payload_)) {
        return value->value;
    }
    return std::nullopt;
}

std::optional<std::uint64_t> Cell::uint_value() const noexcept {
    if (const auto* value = std::get_if<UIntValue>(&payload_)) {
        return value->value;
    }
    return std::nullopt;
}

std::optional<float> Cell::float32_value() const noexcept {
    if (const auto* value = std::get_if<Float32Value>(&payload_)) {
        return value->value;
    }
    return std::nullopt;
}

std::optional<double> Cell::float64_value() const noexcept {
    if (const auto* value = std::get_if<Float64Value>(&payload_)) {
        return value->value;
    }
    return std::nullopt;
}

std::optional<bool> Cell::bool_value() const noexcept {
    if (const auto* value = std::get_if<BoolValue>(&payload_)) {
        return value->value;
    }
    return std::nullopt;
}

std::optional<std::string_view> Cell::text_value() const noexcept {
    if (const auto* value = std::get_if<TextValue>(&payload_)) {
        return std::string_view{value->value};
    }
    return std::nullopt;
}

std::optional<std::int64_t> Cell::category_code() const noexcept {
    if (const auto* value = std::get_if<CategoryValue>(&payload_)) {
        return value->code;
    }
    return std::nullopt;
}

std::optional<std::int64_t> Cell::tick_value() const noexcept {
    if (const auto* value = std::get_if<DateValue>(&payload_)) {
        return value->days;
    }
    if (const auto* value = std::get_if<DurationValue>(&payload_)) {
        return value->microseconds;
    }
    if (const auto* value = std::get_if<TimestampValue>(&payload_)) {
        return value->microseconds;
    }
    return std::nullopt;
}

bool Cell::is_nan() const noexcept {
    if (const auto* value = std::get_if<Float32Value>(&payload_)) {
        return std::isnan(value->value);
    }
    if (const auto* value = std::get_if<Float64Value>(&payload_)) {
        return std::isnan(value->value);
    }
    return false;
}

bool Cell::is_infinity() const noexcept {
    if (const auto* value = std::get_if<Float32Value>(&payload_)) {
        return std::isinf(value->value);
    }
    if (const auto* value = std::get_if<Float64Value>(&payload_)) {
        return std::isinf(value->value);
    }
    return false;
}

bool Cell::is_zero() const noexcept {
    if (const auto* value = std::get_if<Float32Value>(&payload_)) {
        return value->value == 0.0F;
    }
    if (const auto* value = std::get_if<Float64Value>(&payload_)) {
        return value->value == 0.0;
    }
    if (const auto* value = std::get_if<IntValue>(&payload_)) {
        return value->value == 0;
    }
    if (const auto* value = std::get_if<UIntValue>(&payload_)) {
        return value->value == 0U;
    }
    return false;
}

bool Cell::sign_bit() const noexcept {
    if (const auto* value = std::get_if<Float32Value>(&payload_)) {
        return (float32_bits(value->value) & (1ULL << 31U)) != 0ULL;
    }
    if (const auto* value = std::get_if<Float64Value>(&payload_)) {
        return float64_bits(value->value) >> 63U != 0ULL;
    }
    if (const auto* value = std::get_if<IntValue>(&payload_)) {
        return value->value < 0;
    }
    return false;
}

std::optional<std::uint64_t> Cell::bit_pattern() const noexcept {
    if (const auto* value = std::get_if<IntValue>(&payload_)) {
        return static_cast<std::uint64_t>(value->value);
    }
    if (const auto* value = std::get_if<UIntValue>(&payload_)) {
        return value->value;
    }
    if (const auto* value = std::get_if<Float32Value>(&payload_)) {
        return float32_bits(value->value);
    }
    if (const auto* value = std::get_if<Float64Value>(&payload_)) {
        return float64_bits(value->value);
    }
    if (const auto* value = std::get_if<BoolValue>(&payload_)) {
        return value->value ? std::uint64_t{1} : std::uint64_t{0};
    }
    if (const auto* value = std::get_if<CategoryValue>(&payload_)) {
        return static_cast<std::uint64_t>(value->code);
    }
    if (const auto* value = std::get_if<DateValue>(&payload_)) {
        return static_cast<std::uint64_t>(value->days);
    }
    if (const auto* value = std::get_if<DurationValue>(&payload_)) {
        return static_cast<std::uint64_t>(value->microseconds);
    }
    if (const auto* value = std::get_if<TimestampValue>(&payload_)) {
        return static_cast<std::uint64_t>(value->microseconds);
    }
    return std::nullopt;
}

Cell Cell::from_bits(LogicalType type, std::uint64_t bits) {
    const auto width = storage_bits(type).value_or(std::uint32_t{64});
    const auto masked = low_bits(bits, width);
    if (is_signed_integer(type)) {
        return Cell{type, IntValue{sign_extend(masked, width)}};
    }
    if (is_unsigned_integer(type)) {
        return Cell{type, UIntValue{masked}};
    }
    switch (type) {
        case LogicalType::float32:
            return Cell{type, Float32Value{std::bit_cast<float>(static_cast<std::uint32_t>(masked))}};
        case LogicalType::float64:
            return Cell{type, Float64Value{std::bit_cast<double>(masked)}};
        case LogicalType::boolean:
            return Cell{type, BoolValue{(masked & 1ULL) != 0ULL}};
        case LogicalType::categorical:
            return Cell{type, CategoryValue{static_cast<std::int64_t>(masked)}};
        case LogicalType::date:
            return Cell{type, DateValue{static_cast<std::int64_t>(masked)}};
        case LogicalType::duration:
            return Cell{type, DurationValue{static_cast<std::int64_t>(masked)}};
        case LogicalType::timestamp_naive:
        case LogicalType::timestamp_zoned:
            return Cell{type, TimestampValue{static_cast<std::int64_t>(masked)}};
        default:
            // Text has no bit form; a caller that asks for one gets a missing cell rather
            // than a reinterpretation of unrelated bits.
            return Cell::missing(type);
    }
}

bool structurally_equal(const Cell& left, const Cell& right) noexcept {
    return left.type() == right.type() && structurally_equal_payload(left.payload(), right.payload());
}

std::optional<bool> ieee_equal(const Cell& left, const Cell& right) noexcept {
    if (left.type() != right.type()) {
        return std::nullopt;
    }
    return ieee_equal_payload(left.payload(), right.payload());
}

std::string describe(const Cell& cell) {
    std::string text = to_string(cell.type());
    text += ":";
    if (cell.is_missing()) {
        text += "missing";
        return text;
    }
    if (const auto value = cell.text_value()) {
        text += "text(\"";
        text += *value;
        text += "\")";
        return text;
    }
    const auto bits = cell.bit_pattern();
    if (!bits.has_value()) {
        text += "unreported";
        return text;
    }
    text += "0x";
    static constexpr char kDigits[] = "0123456789abcdef";
    for (int shift = 60; shift >= 0; shift -= 4) {
        text += kDigits[(*bits >> static_cast<unsigned>(shift)) & 0xFULL];
    }
    if (cell.is_nan()) {
        text += " (nan)";
    } else if (cell.is_infinity()) {
        text += cell.sign_bit() ? " (-inf)" : " (+inf)";
    }
    return text;
}

Result<std::string> validate_text(std::string_view text, TextNulPolicy policy) {
    if (const auto offset = invalid_utf8_offset(text)) {
        Location location;
        location.byte_offset = offset;
        return Result<std::string>::failure(
            Diagnostic::reject(DetailCode::invalid_text,
                               "text is not valid UTF-8 and was not replaced",
                               std::move(location)));
    }
    if (policy == TextNulPolicy::reject) {
        const auto nul = text.find('\0');
        if (nul != std::string_view::npos) {
            Location location;
            location.byte_offset = static_cast<std::int64_t>(nul);
            return Result<std::string>::failure(
                Diagnostic::reject(DetailCode::invalid_text,
                                   "embedded NUL is rejected by the declared text policy",
                                   std::move(location)));
        }
    }
    return std::string{text};
}

}  // namespace everygraph::data
