#pragma once

#include <string>

namespace everygraph::foundation {

enum class ErrorCode {
    invalid_request,
    unknown_command,
    duplicate_command,
    unsupported_schema_version,
    invalid_argument,
    revision_conflict,
    permission_denied,
    resource_exhausted,
    canceled,
    numerical_failure,
    unsupported_feature,
    io_error,
    not_found,
    already_exists,
    not_repeatable,
    nothing_to_undo,
    nothing_to_redo,
    reentrant_mutation,
    internal_error,
};

[[nodiscard]] const char* to_string(ErrorCode code) noexcept;

struct Error {
    ErrorCode code{ErrorCode::internal_error};
    std::string message;
};

}  // namespace everygraph::foundation
