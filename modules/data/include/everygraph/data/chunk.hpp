#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "everygraph/data/cell.hpp"
#include "everygraph/data/diagnostics.hpp"
#include "everygraph/data/ids.hpp"
#include "everygraph/data/logical_type.hpp"
#include "everygraph/data/provenance.hpp"

namespace everygraph::data {

// Chunk layout is an internal policy, not a persisted assumption. These are the declared
// starting values the data model names; the measured policy belongs to the performance
// baseline task.
inline constexpr std::uint64_t kDefaultChunkRowCount = 65536;
inline constexpr std::uint64_t kDefaultChunkByteCap = 8ULL * 1024ULL * 1024ULL;

struct SignedBuffer {
    using values_type = std::vector<std::int64_t>;
    values_type values;
};
struct UnsignedBuffer {
    using values_type = std::vector<std::uint64_t>;
    values_type values;
};
struct Float32Buffer {
    using values_type = std::vector<float>;
    values_type values;
};
struct Float64Buffer {
    using values_type = std::vector<double>;
    values_type values;
};
struct FlagBuffer {
    using values_type = std::vector<bool>;
    values_type values;
};
struct TextBuffer {
    using values_type = std::vector<std::string>;
    values_type values;
};
struct TickBuffer {
    using values_type = std::vector<std::int64_t>;
    values_type values;
};
struct CodeBuffer {
    using values_type = std::vector<std::int64_t>;
    values_type values;
};

using ColumnBuffer = std::variant<SignedBuffer, UnsignedBuffer, Float32Buffer, Float64Buffer,
                  FlagBuffer, TextBuffer, TickBuffer, CodeBuffer>;

[[nodiscard]] ColumnBuffer make_buffer(LogicalType type, std::size_t reserve) noexcept;

// An immutable block of one field's values for one contiguous row range. Nothing here has a
// setter: an edit produces a new chunk and a new version, and readers hold a shared
// reference to the block they resolved.
class Chunk {
public:
    [[nodiscard]] ChunkId id() const noexcept { return id_; }
    [[nodiscard]] FieldId field() const noexcept { return field_; }
    [[nodiscard]] LogicalType type() const noexcept { return type_; }
    [[nodiscard]] std::uint64_t first_row_index() const noexcept { return first_row_index_; }
    [[nodiscard]] std::uint64_t row_count() const noexcept { return validity_.size(); }
    [[nodiscard]] const ContentHash& content_hash() const noexcept { return content_hash_; }
    [[nodiscard]] std::uint64_t stored_bytes() const noexcept;

    [[nodiscard]] Result<Cell> cell_at(std::uint64_t local_index) const;

private:
    // The builder is the only writer: it fixes the content hash over the exact buffer and
    // validity it publishes, so a chunk cannot be assembled from fields that disagree with
    // their own digest.
    Chunk(ChunkId id, FieldId field, LogicalType type, std::uint64_t first_row_index,
          ColumnBuffer buffer, std::vector<bool> validity, ContentHash content_hash)
        : id_(id),
          field_(field),
          type_(type),
          first_row_index_(first_row_index),
          buffer_(std::move(buffer)),
          validity_(std::move(validity)),
          content_hash_(content_hash) {}

    friend class ChunkBuilder;

    ChunkId id_;
    FieldId field_;
    LogicalType type_{LogicalType::float64};
    std::uint64_t first_row_index_{0};
    ColumnBuffer buffer_;
    std::vector<bool> validity_;
    ContentHash content_hash_;
};

struct ChunkOptions {
    TextNulPolicy text_policy{TextNulPolicy::reject};
    std::uint64_t max_rows{kDefaultChunkRowCount};
    std::uint64_t max_bytes{kDefaultChunkByteCap};
};

// The private writer named by the data model: a writer owns a builder and can publish only
// through the application transaction, so a builder hands out an immutable chunk and never
// touches a store.
class ChunkBuilder {
public:
    static Result<ChunkBuilder> create(FieldId field, LogicalType type, ChunkOptions options = {});

    [[nodiscard]] FieldId field() const noexcept { return field_; }
    [[nodiscard]] LogicalType type() const noexcept { return type_; }
    [[nodiscard]] std::uint64_t row_count() const noexcept { return validity_.size(); }
    [[nodiscard]] std::uint64_t stored_bytes() const noexcept;

    [[nodiscard]] std::optional<Diagnostic> append_missing();
    [[nodiscard]] std::optional<Diagnostic> append_int(std::int64_t value);
    [[nodiscard]] std::optional<Diagnostic> append_uint(std::uint64_t value);
    [[nodiscard]] std::optional<Diagnostic> append_float32(float value);
    [[nodiscard]] std::optional<Diagnostic> append_float64(double value);
    [[nodiscard]] std::optional<Diagnostic> append_bool(bool value);
    [[nodiscard]] std::optional<Diagnostic> append_text(std::string_view value);
    [[nodiscard]] std::optional<Diagnostic> append_category_code(std::int64_t code);
    [[nodiscard]] std::optional<Diagnostic> append_ticks(std::int64_t ticks);
    // The exact-bit form, which is how a caller stores a chosen NaN payload or a signed zero
    // without going through a decimal conversion. It is accepted only when the declared type
    // reports exactly those bits back through Cell::bit_pattern(), so a narrower column never
    // silently masks the high bits a caller named.
    [[nodiscard]] std::optional<Diagnostic> append_bits(std::uint64_t bits);

    [[nodiscard]] Result<Chunk> finish(ChunkId id, std::uint64_t first_row_index = 0) const;

private:
    ChunkBuilder(FieldId field, LogicalType type, ChunkOptions options)
        : field_(field), type_(type), options_(options) {}

    [[nodiscard]] std::optional<Diagnostic> reject_type(LogicalType wanted) const;
    [[nodiscard]] std::optional<Diagnostic> check_capacity() const;
    void commit_validity(bool valid) noexcept { validity_.push_back(valid); }

    FieldId field_;
    LogicalType type_{LogicalType::float64};
    ChunkOptions options_;
    ColumnBuffer buffer_{SignedBuffer{}};
    std::vector<bool> validity_;
    std::uint64_t stored_bytes_{0};
};

}  // namespace everygraph::data
