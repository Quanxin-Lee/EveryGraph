#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <variant>

#include "everygraph/data/ids.hpp"
#include "everygraph/foundation/error.hpp"

namespace everygraph::data {

// Test-facing detail codes for the `data.` family named by the P2 conformance plan.
// The plan freezes the outer application code, not these names, so this registry is the
// implementation-side list the plan asks the owning task to document.
enum class DetailCode {
    stale_revision,
    duplicate_id,
    dangling_reference,
    type_mismatch,
    unknown_unit,
    incompatible_unit,
    offset_unit_arithmetic,
    positional_mapping_required,
    duplicate_join_key,
    unmapped_row,
    mirror_cycle,
    mirror_read_only,
    number_overflow,
    number_ambiguous,
    invalid_text,
    date_format,
    date_ambiguous,
    date_nonexistent,
    unknown_zone,
    unknown_category,
    unknown_field,
    unknown_draft,
    index_out_of_range,
    precision_loss,
    overflow,
    resource_limit,
};

[[nodiscard]] const char* to_string(DetailCode code) noexcept;

// The outer application code the plan maps each detail code to. `precision_loss` and
// `overflow` are the two codes whose outer code depends on whether the operation
// continues or rejects, so they default to the rejecting outer code and a warning
// diagnostic overrides it. `mirror_read_only` is a registry code the plan's mapping table
// does not list yet, so its outer code is this module's declaration, not a plan citation.
[[nodiscard]] foundation::ErrorCode outer_code(DetailCode code) noexcept;

enum class Severity {
    info,
    warning,
    error,
    fatal,
};

[[nodiscard]] const char* to_string(Severity severity) noexcept;

// Structured location. A diagnostic for this module names the object it touched rather
// than a prose sentence, so tests assert the detail code, the severity and this location.
struct Location {
    std::optional<ColumnId> column;
    std::optional<RowId> row;
    std::optional<DatasetVersionId> version;
    std::optional<ChunkId> chunk;
    std::optional<FieldId> field;
    std::optional<std::int64_t> byte_offset;
    std::optional<std::int64_t> row_index;
    std::string description;

    [[nodiscard]] std::string describe() const;
};

struct Diagnostic {
    DetailCode detail{DetailCode::type_mismatch};
    foundation::ErrorCode outer{foundation::ErrorCode::invalid_argument};
    Severity severity{Severity::error};
    Location location;
    std::string message;

    [[nodiscard]] static Diagnostic reject(DetailCode code, std::string message,
                                            Location location = {});
    [[nodiscard]] static Diagnostic warn(DetailCode code, std::string message,
                                         Location location = {});

    [[nodiscard]] foundation::Error to_error() const;
};

// Structured result: a value or one diagnostic. Nothing here throws across a public
// surface, and a rejected caller never receives a partially built object.
template <typename T>
class Result {
public:
    Result(T value) : storage_(std::move(value)) {}  // NOLINT: intentional implicit

    [[nodiscard]] static Result<T> failure(Diagnostic diagnostic) {
        return Result<T>{FailureTag{}, std::move(diagnostic)};
    }

    [[nodiscard]] bool ok() const noexcept { return std::holds_alternative<T>(storage_); }
    explicit operator bool() const noexcept { return ok(); }

    [[nodiscard]] T& value() & { return std::get<T>(storage_); }
    [[nodiscard]] const T& value() const& { return std::get<T>(storage_); }
    [[nodiscard]] T take() { return std::get<T>(std::move(storage_)); }

    [[nodiscard]] const Diagnostic& diagnostic() const { return std::get<Diagnostic>(storage_); }
    [[nodiscard]] Diagnostic diagnostic_copy() const { return std::get<Diagnostic>(storage_); }

private:
    struct FailureTag {};
    Result(FailureTag, Diagnostic diagnostic) : storage_(std::move(diagnostic)) {}

    std::variant<T, Diagnostic> storage_;
};

}  // namespace everygraph::data
