#include "everygraph/data/diagnostics.hpp"

#include <optional>
#include <string>

namespace everygraph::data {

const char* to_string(DetailCode code) noexcept {
    switch (code) {
        case DetailCode::stale_revision:
            return "data.stale_revision";
        case DetailCode::duplicate_id:
            return "data.duplicate_id";
        case DetailCode::dangling_reference:
            return "data.dangling_reference";
        case DetailCode::type_mismatch:
            return "data.type_mismatch";
        case DetailCode::unknown_unit:
            return "data.unknown_unit";
        case DetailCode::incompatible_unit:
            return "data.incompatible_unit";
        case DetailCode::offset_unit_arithmetic:
            return "data.offset_unit_arithmetic";
        case DetailCode::positional_mapping_required:
            return "data.positional_mapping_required";
        case DetailCode::duplicate_join_key:
            return "data.duplicate_join_key";
        case DetailCode::unmapped_row:
            return "data.unmapped_row";
        case DetailCode::mirror_cycle:
            return "data.mirror_cycle";
        case DetailCode::mirror_read_only:
            return "data.mirror_read_only";
        case DetailCode::number_overflow:
            return "data.number_overflow";
        case DetailCode::number_ambiguous:
            return "data.number_ambiguous";
        case DetailCode::invalid_text:
            return "data.invalid_text";
        case DetailCode::date_format:
            return "data.date_format";
        case DetailCode::date_ambiguous:
            return "data.date_ambiguous";
        case DetailCode::date_nonexistent:
            return "data.date_nonexistent";
        case DetailCode::unknown_zone:
            return "data.unknown_zone";
        case DetailCode::unknown_category:
            return "data.unknown_category";
        case DetailCode::unknown_field:
            return "data.unknown_field";
        case DetailCode::unknown_draft:
            return "data.unknown_draft";
        case DetailCode::index_out_of_range:
            return "data.index_out_of_range";
        case DetailCode::precision_loss:
            return "data.precision_loss";
        case DetailCode::overflow:
            return "data.overflow";
        case DetailCode::resource_limit:
            return "data.resource_limit";
    }
    return "data.unknown_detail";
}

foundation::ErrorCode outer_code(DetailCode code) noexcept {
    switch (code) {
        case DetailCode::stale_revision:
            return foundation::ErrorCode::revision_conflict;
        case DetailCode::resource_limit:
            return foundation::ErrorCode::resource_exhausted;
        case DetailCode::mirror_read_only:
            // The argument is well-formed and the value would have been legal on a mutable
            // column, so this is not an argument error: the operation is not offered for the
            // object the caller named. The conformance plan's mapping table has no row for
            // this code yet, which P2-002 requested as a plan revision.
            return foundation::ErrorCode::unsupported_feature;
        case DetailCode::precision_loss:
        case DetailCode::overflow:
            // The plan maps these to a warning detail when the operation continues and to
            // `numerical_failure` when it rejects the loss. The rejecting code is the
            // default here, and a warning diagnostic carries the same code as the name of
            // what it would have been.
            return foundation::ErrorCode::numerical_failure;
        default:
            return foundation::ErrorCode::invalid_argument;
    }
}

const char* to_string(Severity severity) noexcept {
    switch (severity) {
        case Severity::info:
            return "info";
        case Severity::warning:
            return "warning";
        case Severity::error:
            return "error";
        case Severity::fatal:
            return "fatal";
    }
    return "unknown";
}

std::string Location::describe() const {
    std::string text;
    const auto append = [&text](const char* label, const std::string& value) {
        if (!text.empty()) {
            text += "; ";
        }
        text += label;
        text += "=";
        text += value;
    };
    if (column.has_value()) {
        append("column", to_string(*column));
    }
    if (field.has_value()) {
        append("field", to_string(*field));
    }
    if (row.has_value()) {
        append("row", to_string(*row));
    }
    if (row_index.has_value()) {
        append("row_index", std::to_string(*row_index));
    }
    if (version.has_value()) {
        append("version", to_string(*version));
    }
    if (chunk.has_value()) {
        append("chunk", to_string(*chunk));
    }
    if (byte_offset.has_value()) {
        append("byte", std::to_string(*byte_offset));
    }
    if (!description.empty()) {
        if (!text.empty()) {
            text += "; ";
        }
        text += description;
    }
    return text;
}

Diagnostic Diagnostic::reject(DetailCode code, std::string message, Location location) {
    Diagnostic diagnostic;
    diagnostic.detail = code;
    diagnostic.outer = outer_code(code);
    diagnostic.severity = Severity::error;
    diagnostic.location = std::move(location);
    diagnostic.message = std::move(message);
    return diagnostic;
}

Diagnostic Diagnostic::warn(DetailCode code, std::string message, Location location) {
    Diagnostic diagnostic;
    diagnostic.detail = code;
    diagnostic.outer = outer_code(code);
    diagnostic.severity = Severity::warning;
    diagnostic.location = std::move(location);
    diagnostic.message = std::move(message);
    return diagnostic;
}

foundation::Error Diagnostic::to_error() const {
    // The kernel envelope carries one code and one English message, so the detail code and the
    // structured location travel inside that message. Tests assert the code, the severity and
    // the location rather than the prose.
    std::string text{to_string(detail)};
    text += ": ";
    text += message;
    const auto place = location.describe();
    if (!place.empty()) {
        text += " [at ";
        text += place;
        text += "]";
    }
    return foundation::Error{outer, std::move(text)};
}

}  // namespace everygraph::data
