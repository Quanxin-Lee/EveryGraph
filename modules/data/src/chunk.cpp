#include "everygraph/data/chunk.hpp"

#include <bit>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

namespace everygraph::data {
namespace {

std::string canonical_chunk_bytes(const ColumnBuffer& buffer, const std::vector<bool>& validity,
                                 LogicalType type, std::uint64_t first_row_index, FieldId field) {
    std::string bytes;
    bytes += to_string(type);
    bytes += "|";
    bytes += to_string(field);
    bytes += "|";
    bytes += std::to_string(first_row_index);
    bytes += "|";
    for (std::size_t index = 0; index < validity.size(); ++index) {
        bytes += validity[index] ? "1" : "0";
        bytes += ",";
    }
    bytes += ";";
    std::visit(
        [&bytes](const auto& typed) {
            using Buffer = std::decay_t<decltype(typed)>;
            if constexpr (std::is_same_v<Buffer, TextBuffer>) {
                for (const auto& text : typed.values) {
                    bytes += std::to_string(text.size());
                    bytes += ":";
                    bytes += text;
                    bytes += ";";
                }
            } else if constexpr (std::is_same_v<Buffer, FlagBuffer>) {
                for (const bool flag : typed.values) {
                    bytes += flag ? "T" : "F";
                }
            } else if constexpr (std::is_same_v<Buffer, Float32Buffer>) {
                for (const float value : typed.values) {
                    bytes += std::to_string(static_cast<std::uint64_t>(
                        std::bit_cast<std::uint32_t>(value)));
                    bytes += ";";
                }
            } else if constexpr (std::is_same_v<Buffer, Float64Buffer>) {
                for (const double value : typed.values) {
                    bytes += std::to_string(std::bit_cast<std::uint64_t>(value));
                    bytes += ";";
                }
            } else if constexpr (std::is_same_v<Buffer, UnsignedBuffer>) {
                for (const std::uint64_t value : typed.values) {
                    bytes += std::to_string(value);
                    bytes += ";";
                }
            } else {
                for (const std::int64_t value : typed.values) {
                    bytes += std::to_string(static_cast<std::uint64_t>(value));
                    bytes += ";";
                }
            }
        },
        buffer);
    return bytes;
}

std::optional<Diagnostic> wrong_type(LogicalType wanted, LogicalType actual, FieldId field) {
    Location location;
    location.field = field;
    location.description = std::string{"appended a "} + to_string(wanted) + " to a " +
                           to_string(actual) + " field";
    return Diagnostic::reject(DetailCode::type_mismatch,
                              "value type does not match the declared column type",
                              std::move(location));
}

}  // namespace

ColumnBuffer make_buffer(LogicalType type, std::size_t reserve) noexcept {
    switch (type) {
        case LogicalType::int8:
        case LogicalType::int16:
        case LogicalType::int32:
        case LogicalType::int64: {
            SignedBuffer buffer;
            buffer.values.reserve(reserve);
            return buffer;
        }
        case LogicalType::uint8:
        case LogicalType::uint16:
        case LogicalType::uint32:
        case LogicalType::uint64: {
            UnsignedBuffer buffer;
            buffer.values.reserve(reserve);
            return buffer;
        }
        case LogicalType::float32: {
            Float32Buffer buffer;
            buffer.values.reserve(reserve);
            return buffer;
        }
        case LogicalType::float64: {
            Float64Buffer buffer;
            buffer.values.reserve(reserve);
            return buffer;
        }
        case LogicalType::boolean: {
            FlagBuffer buffer;
            buffer.values.reserve(reserve);
            return buffer;
        }
        case LogicalType::text: {
            TextBuffer buffer;
            buffer.values.reserve(reserve);
            return buffer;
        }
        case LogicalType::categorical: {
            CodeBuffer buffer;
            buffer.values.reserve(reserve);
            return buffer;
        }
        case LogicalType::date:
        case LogicalType::duration:
        case LogicalType::timestamp_naive:
        case LogicalType::timestamp_zoned: {
            TickBuffer buffer;
            buffer.values.reserve(reserve);
            return buffer;
        }
    }
    return SignedBuffer{};
}

std::uint64_t Chunk::stored_bytes() const noexcept {
    std::uint64_t bytes = 0;
    std::visit(
        [&bytes](const auto& typed) {
            using Buffer = std::decay_t<decltype(typed)>;
            if constexpr (std::is_same_v<Buffer, TextBuffer>) {
                for (const auto& text : typed.values) {
                    bytes += text.size();
                }
            } else if constexpr (std::is_same_v<Buffer, FlagBuffer>) {
                bytes += typed.values.size() / 8ULL + 1ULL;
            } else {
                bytes += typed.values.size() * sizeof(typename Buffer::values_type::value_type);
            }
        },
        buffer_);
    bytes += validity_.size() / 8ULL + 1ULL;
    return bytes;
}

Result<Cell> Chunk::cell_at(std::uint64_t local_index) const {
    if (local_index >= row_count()) {
        Location location;
        location.chunk = id_;
        location.field = field_;
        location.row_index = static_cast<std::int64_t>(local_index);
        return Result<Cell>::failure(Diagnostic::reject(
            DetailCode::index_out_of_range,
            "row index is outside the chunk range: " + std::to_string(local_index) + " of " +
                std::to_string(row_count()),
            std::move(location)));
    }
    if (!validity_[local_index]) {
        return Cell::missing(type_);
    }
    return std::visit(
        [this, local_index](const auto& typed) -> Cell {
            using Buffer = std::decay_t<decltype(typed)>;
            if constexpr (std::is_same_v<Buffer, SignedBuffer>) {
                return Cell{type_, IntValue{typed.values[local_index]}};
            } else if constexpr (std::is_same_v<Buffer, UnsignedBuffer>) {
                return Cell{type_, UIntValue{typed.values[local_index]}};
            } else if constexpr (std::is_same_v<Buffer, Float32Buffer>) {
                return Cell{type_, Float32Value{typed.values[local_index]}};
            } else if constexpr (std::is_same_v<Buffer, Float64Buffer>) {
                return Cell{type_, Float64Value{typed.values[local_index]}};
            } else if constexpr (std::is_same_v<Buffer, FlagBuffer>) {
                return Cell{type_, BoolValue{typed.values[local_index]}};
            } else if constexpr (std::is_same_v<Buffer, TextBuffer>) {
                return Cell{type_, TextValue{typed.values[local_index]}};
            } else if constexpr (std::is_same_v<Buffer, CodeBuffer>) {
                return Cell{type_, CategoryValue{typed.values[local_index]}};
            } else {
                const std::int64_t ticks = typed.values[local_index];
                switch (type_) {
                    case LogicalType::date:
                        return Cell{type_, DateValue{ticks}};
                    case LogicalType::duration:
                        return Cell{type_, DurationValue{ticks}};
                    default:
                        return Cell{type_, TimestampValue{ticks}};
                }
            }
        },
        buffer_);
}

Result<ChunkBuilder> ChunkBuilder::create(FieldId field, LogicalType type, ChunkOptions options) {
    if (!field.is_valid()) {
        Location location;
        location.field = field;
        return Result<ChunkBuilder>::failure(Diagnostic::reject(
            DetailCode::duplicate_id, "field identity must be a non-zero opaque id", std::move(location)));
    }
    if (options.max_rows == 0 || options.max_bytes == 0) {
        return Result<ChunkBuilder>::failure(Diagnostic::reject(
            DetailCode::resource_limit, "chunk row and byte budgets must both be positive", {}));
    }
    ChunkBuilder builder{field, type, options};
    builder.buffer_ = make_buffer(type, 0);
    return builder;
}

std::uint64_t ChunkBuilder::stored_bytes() const noexcept {
    return stored_bytes_;
}

std::optional<Diagnostic> ChunkBuilder::reject_type(LogicalType wanted) const {
    if (type_ == wanted) {
        return std::nullopt;
    }
    return wrong_type(wanted, type_, field_);
}

std::optional<Diagnostic> ChunkBuilder::check_capacity() const {
    if (validity_.size() >= options_.max_rows) {
        Location location;
        location.field = field_;
        location.row_index = static_cast<std::int64_t>(validity_.size());
        return Diagnostic::reject(DetailCode::resource_limit,
                                  "chunk row budget exhausted at " +
                                      std::to_string(options_.max_rows) + " rows",
                                  std::move(location));
    }
    if (stored_bytes_ >= options_.max_bytes) {
        Location location;
        location.field = field_;
        return Diagnostic::reject(DetailCode::resource_limit, "chunk byte budget exhausted",
                                  std::move(location));
    }
    return std::nullopt;
}

std::optional<Diagnostic> ChunkBuilder::append_missing() {
    if (auto capacity = check_capacity()) {
        return capacity;
    }
    // A missing slot still occupies its storage position, so the validity vector and the
    // value vector stay index-aligned without inventing a scientific value.
    std::visit(
        [this](auto& typed) {
            using Buffer = std::decay_t<decltype(typed)>;
            if constexpr (std::is_same_v<Buffer, TextBuffer>) {
                typed.values.emplace_back();
            } else if constexpr (std::is_same_v<Buffer, FlagBuffer>) {
                typed.values.push_back(false);
            } else {
                typed.values.push_back(typename Buffer::values_type::value_type{});
                stored_bytes_ += sizeof(typename Buffer::values_type::value_type);
            }
        },
        buffer_);
    commit_validity(false);
    return std::nullopt;
}

std::optional<Diagnostic> ChunkBuilder::append_int(std::int64_t value) {
    if (!is_signed_integer(type_)) {
        return wrong_type(LogicalType::int64, type_, field_);
    }
    if (!in_range(value, type_)) {
        Location location;
        location.field = field_;
        location.row_index = static_cast<std::int64_t>(validity_.size());
        location.description = std::to_string(value);
        return Diagnostic::reject(DetailCode::number_overflow,
                                  std::string{"value is outside the range of "} + to_string(type_),
                                  std::move(location));
    }
    if (auto capacity = check_capacity()) {
        return capacity;
    }
    std::get<SignedBuffer>(buffer_).values.push_back(value);
    stored_bytes_ += sizeof(std::int64_t);
    commit_validity(true);
    return std::nullopt;
}

std::optional<Diagnostic> ChunkBuilder::append_uint(std::uint64_t value) {
    if (!is_unsigned_integer(type_)) {
        return wrong_type(LogicalType::uint64, type_, field_);
    }
    if (!in_range(value, type_)) {
        Location location;
        location.field = field_;
        location.row_index = static_cast<std::int64_t>(validity_.size());
        location.description = std::to_string(value);
        return Diagnostic::reject(DetailCode::number_overflow,
                                  std::string{"value is outside the range of "} + to_string(type_),
                                  std::move(location));
    }
    if (auto capacity = check_capacity()) {
        return capacity;
    }
    std::get<UnsignedBuffer>(buffer_).values.push_back(value);
    stored_bytes_ += sizeof(std::uint64_t);
    commit_validity(true);
    return std::nullopt;
}

std::optional<Diagnostic> ChunkBuilder::append_float32(float value) {
    if (auto mismatch = reject_type(LogicalType::float32)) {
        return mismatch;
    }
    if (auto capacity = check_capacity()) {
        return capacity;
    }
    std::get<Float32Buffer>(buffer_).values.push_back(value);
    stored_bytes_ += sizeof(float);
    commit_validity(true);
    return std::nullopt;
}

std::optional<Diagnostic> ChunkBuilder::append_float64(double value) {
    if (auto mismatch = reject_type(LogicalType::float64)) {
        return mismatch;
    }
    if (auto capacity = check_capacity()) {
        return capacity;
    }
    std::get<Float64Buffer>(buffer_).values.push_back(value);
    stored_bytes_ += sizeof(double);
    commit_validity(true);
    return std::nullopt;
}

std::optional<Diagnostic> ChunkBuilder::append_bool(bool value) {
    if (auto mismatch = reject_type(LogicalType::boolean)) {
        return mismatch;
    }
    if (auto capacity = check_capacity()) {
        return capacity;
    }
    std::get<FlagBuffer>(buffer_).values.push_back(value);
    stored_bytes_ += 1ULL;
    commit_validity(true);
    return std::nullopt;
}

std::optional<Diagnostic> ChunkBuilder::append_text(std::string_view value) {
    if (auto mismatch = reject_type(LogicalType::text)) {
        return mismatch;
    }
    auto validated = validate_text(value, options_.text_policy);
    if (!validated.ok()) {
        Location location = validated.diagnostic().location;
        location.field = field_;
        location.row_index = static_cast<std::int64_t>(validity_.size());
        Diagnostic diagnostic = validated.diagnostic_copy();
        diagnostic.location = std::move(location);
        return diagnostic;
    }
    if (auto capacity = check_capacity()) {
        return capacity;
    }
    stored_bytes_ += validated.value().size();
    std::get<TextBuffer>(buffer_).values.push_back(validated.take());
    commit_validity(true);
    return std::nullopt;
}

std::optional<Diagnostic> ChunkBuilder::append_category_code(std::int64_t code) {
    if (auto mismatch = reject_type(LogicalType::categorical)) {
        return mismatch;
    }
    if (code < 0) {
        Location location;
        location.field = field_;
        location.description = std::to_string(code);
        return Diagnostic::reject(DetailCode::unknown_category,
                                  "category code must be a non-negative dictionary index",
                                  std::move(location));
    }
    if (auto capacity = check_capacity()) {
        return capacity;
    }
    std::get<CodeBuffer>(buffer_).values.push_back(code);
    stored_bytes_ += sizeof(std::int64_t);
    commit_validity(true);
    return std::nullopt;
}

std::optional<Diagnostic> ChunkBuilder::append_ticks(std::int64_t ticks) {
    if (!is_temporal(type_)) {
        return wrong_type(LogicalType::date, type_, field_);
    }
    if (auto capacity = check_capacity()) {
        return capacity;
    }
    std::get<TickBuffer>(buffer_).values.push_back(ticks);
    stored_bytes_ += sizeof(std::int64_t);
    commit_validity(true);
    return std::nullopt;
}

std::optional<Diagnostic> ChunkBuilder::append_bits(std::uint64_t bits) {
    if (!is_numeric(type_) && !is_temporal(type_) && type_ != LogicalType::boolean &&
        type_ != LogicalType::categorical) {
        Location location;
        location.field = field_;
        location.description = to_string(type_);
        return Diagnostic::reject(DetailCode::type_mismatch,
                                  "a bit form exists only for fixed-width storage types",
                                  std::move(location));
    }
    const auto cell = Cell::from_bits(type_, bits);
    // The bit form is a promise about stored bits, so it is only accepted when the declared
    // type can hold exactly those bits. A narrower column would otherwise silently drop the
    // high bits the caller named, and a boolean would read 2 as 0.
    if (cell.bit_pattern() != bits) {
        Location location;
        location.field = field_;
        location.row_index = static_cast<std::int64_t>(validity_.size());
        location.description = std::string{to_string(type_)} + " reports " +
                               std::to_string(cell.bit_pattern().value_or(0)) + " back";
        return Diagnostic::reject(DetailCode::number_overflow,
                                  "bit form does not fit the declared type: " + std::to_string(bits) +
                                      " for " + to_string(type_),
                                  std::move(location));
    }
    if (is_signed_integer(type_)) {
        return append_int(cell.int_value().value_or(0));
    }
    if (is_unsigned_integer(type_)) {
        return append_uint(cell.uint_value().value_or(0));
    }
    if (type_ == LogicalType::float32) {
        return append_float32(cell.float32_value().value_or(0.0F));
    }
    if (type_ == LogicalType::float64) {
        return append_float64(cell.float64_value().value_or(0.0));
    }
    if (type_ == LogicalType::boolean) {
        return append_bool(cell.bool_value().value_or(false));
    }
    if (type_ == LogicalType::categorical) {
        return append_category_code(cell.category_code().value_or(0));
    }
    return append_ticks(cell.tick_value().value_or(0));
}

Result<Chunk> ChunkBuilder::finish(ChunkId id, std::uint64_t first_row_index) const {
    if (!id.is_valid()) {
        Location location;
        location.chunk = id;
        return Result<Chunk>::failure(Diagnostic::reject(
            DetailCode::duplicate_id, "chunk identity must be a non-zero opaque id", std::move(location)));
    }
    const auto bytes = canonical_chunk_bytes(buffer_, validity_, type_, first_row_index, field_);
    return Chunk{id, field_, type_, first_row_index, buffer_, validity_, hash_bytes(bytes)};
}

}  // namespace everygraph::data
