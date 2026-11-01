#pragma once

#include <cstdint>
#include <optional>

namespace everygraph::data {

// The initial logical types named by the data model. The storage width and signedness
// are part of the type: a value is never reinterpreted because a later column happens
// to fit, and a declared float32 column never holds a silently demoted float64.
enum class LogicalType {
    int8,
    int16,
    int32,
    int64,
    uint8,
    uint16,
    uint32,
    uint64,
    float32,
    float64,
    boolean,
    text,
    categorical,
    date,
    duration,
    timestamp_naive,
    timestamp_zoned,
};

[[nodiscard]] const char* to_string(LogicalType type) noexcept;

[[nodiscard]] bool is_signed_integer(LogicalType type) noexcept;
[[nodiscard]] bool is_unsigned_integer(LogicalType type) noexcept;
[[nodiscard]] bool is_floating(LogicalType type) noexcept;
[[nodiscard]] bool is_integer(LogicalType type) noexcept;
[[nodiscard]] bool is_numeric(LogicalType type) noexcept;
[[nodiscard]] bool is_temporal(LogicalType type) noexcept;

// Declared storage width in bits; text, categorical and boolean report none.
[[nodiscard]] std::optional<std::uint32_t> storage_bits(LogicalType type) noexcept;

// Tick unit for the temporal types: date counts days, duration and timestamp count
// microseconds. Keeping this in the type table is what stops a date-only value from
// being read as an instant.
enum class TickUnit {
    none,
    day,
    microsecond,
};

[[nodiscard]] TickUnit tick_unit(LogicalType type) noexcept;
[[nodiscard]] const char* to_string(TickUnit unit) noexcept;

// Range check for an integer append. Out-of-range is a rejection, never a wrap.
[[nodiscard]] bool in_range(std::int64_t value, LogicalType type) noexcept;
[[nodiscard]] bool in_range(std::uint64_t value, LogicalType type) noexcept;

}  // namespace everygraph::data
