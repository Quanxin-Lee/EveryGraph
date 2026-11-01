#include "everygraph/data/logical_type.hpp"

#include <cstdint>
#include <limits>

namespace everygraph::data {
namespace {

constexpr std::int64_t kInt8Min = -128;
constexpr std::int64_t kInt8Max = 127;
constexpr std::int64_t kInt16Min = -32768;
constexpr std::int64_t kInt16Max = 32767;
constexpr std::int32_t kInt32BoundMin = std::numeric_limits<std::int32_t>::min();
constexpr std::int64_t kInt32Min = kInt32BoundMin;
constexpr std::int64_t kInt32Max = std::numeric_limits<std::int32_t>::max();
constexpr std::uint64_t kUint8Max = 255ULL;
constexpr std::uint64_t kUint16Max = 65535ULL;
constexpr std::uint64_t kUint32Max = 4294967295ULL;
constexpr std::uint64_t kUint64Max = std::numeric_limits<std::uint64_t>::max();

}  // namespace

const char* to_string(LogicalType type) noexcept {
    switch (type) {
        case LogicalType::int8:
            return "int8";
        case LogicalType::int16:
            return "int16";
        case LogicalType::int32:
            return "int32";
        case LogicalType::int64:
            return "int64";
        case LogicalType::uint8:
            return "uint8";
        case LogicalType::uint16:
            return "uint16";
        case LogicalType::uint32:
            return "uint32";
        case LogicalType::uint64:
            return "uint64";
        case LogicalType::float32:
            return "float32";
        case LogicalType::float64:
            return "float64";
        case LogicalType::boolean:
            return "boolean";
        case LogicalType::text:
            return "text";
        case LogicalType::categorical:
            return "categorical";
        case LogicalType::date:
            return "date";
        case LogicalType::duration:
            return "duration";
        case LogicalType::timestamp_naive:
            return "timestamp_naive";
        case LogicalType::timestamp_zoned:
            return "timestamp_zoned";
    }
    return "unknown";
}

bool is_signed_integer(LogicalType type) noexcept {
    switch (type) {
        case LogicalType::int8:
        case LogicalType::int16:
        case LogicalType::int32:
        case LogicalType::int64:
            return true;
        default:
            return false;
    }
}

bool is_unsigned_integer(LogicalType type) noexcept {
    switch (type) {
        case LogicalType::uint8:
        case LogicalType::uint16:
        case LogicalType::uint32:
        case LogicalType::uint64:
            return true;
        default:
            return false;
    }
}

bool is_integer(LogicalType type) noexcept {
    return is_signed_integer(type) || is_unsigned_integer(type);
}

bool is_floating(LogicalType type) noexcept {
    return type == LogicalType::float32 || type == LogicalType::float64;
}

bool is_numeric(LogicalType type) noexcept {
    return is_integer(type) || is_floating(type);
}

bool is_temporal(LogicalType type) noexcept {
    switch (type) {
        case LogicalType::date:
        case LogicalType::duration:
        case LogicalType::timestamp_naive:
        case LogicalType::timestamp_zoned:
            return true;
        default:
            return false;
    }
}

std::optional<std::uint32_t> storage_bits(LogicalType type) noexcept {
    switch (type) {
        case LogicalType::int8:
        case LogicalType::uint8:
            return std::uint32_t{8};
        case LogicalType::int16:
        case LogicalType::uint16:
            return std::uint32_t{16};
        case LogicalType::int32:
        case LogicalType::uint32:
        case LogicalType::float32:
            return std::uint32_t{32};
        case LogicalType::int64:
        case LogicalType::uint64:
        case LogicalType::float64:
            return std::uint32_t{64};
        default:
            return std::nullopt;
    }
}

TickUnit tick_unit(LogicalType type) noexcept {
    switch (type) {
        case LogicalType::date:
            return TickUnit::day;
        case LogicalType::duration:
        case LogicalType::timestamp_naive:
        case LogicalType::timestamp_zoned:
            return TickUnit::microsecond;
        default:
            return TickUnit::none;
    }
}

const char* to_string(TickUnit unit) noexcept {
    switch (unit) {
        case TickUnit::none:
            return "none";
        case TickUnit::day:
            return "day";
        case TickUnit::microsecond:
            return "microsecond";
    }
    return "unknown";
}

bool in_range(std::int64_t value, LogicalType type) noexcept {
    switch (type) {
        case LogicalType::int8:
            return value >= kInt8Min && value <= kInt8Max;
        case LogicalType::int16:
            return value >= kInt16Min && value <= kInt16Max;
        case LogicalType::int32:
            return value >= kInt32Min && value <= kInt32Max;
        case LogicalType::int64:
            return true;
        case LogicalType::uint8:
            return value >= 0 && static_cast<std::uint64_t>(value) <= kUint8Max;
        case LogicalType::uint16:
            return value >= 0 && static_cast<std::uint64_t>(value) <= kUint16Max;
        case LogicalType::uint32:
            return value >= 0 && static_cast<std::uint64_t>(value) <= kUint32Max;
        case LogicalType::uint64:
            return value >= 0;
        default:
            return false;
    }
}

bool in_range(std::uint64_t value, LogicalType type) noexcept {
    switch (type) {
        case LogicalType::int8:
            return value <= static_cast<std::uint64_t>(kInt8Max);
        case LogicalType::int16:
            return value <= static_cast<std::uint64_t>(kInt16Max);
        case LogicalType::int32:
            return value <= static_cast<std::uint64_t>(kInt32Max);
        case LogicalType::int64:
            return value <= std::numeric_limits<std::int64_t>::max();
        case LogicalType::uint8:
            return value <= kUint8Max;
        case LogicalType::uint16:
            return value <= kUint16Max;
        case LogicalType::uint32:
            return value <= kUint32Max;
        case LogicalType::uint64:
            return value <= kUint64Max;
        default:
            return false;
    }
}

}  // namespace everygraph::data
