#include "everygraph/foundation/error.hpp"

namespace everygraph::foundation {

const char* to_string(ErrorCode code) noexcept {
    switch (code) {
        case ErrorCode::invalid_request:
            return "invalid_request";
        case ErrorCode::unknown_command:
            return "unknown_command";
        case ErrorCode::duplicate_command:
            return "duplicate_command";
        case ErrorCode::unsupported_schema_version:
            return "unsupported_schema_version";
        case ErrorCode::invalid_argument:
            return "invalid_argument";
        case ErrorCode::revision_conflict:
            return "revision_conflict";
        case ErrorCode::permission_denied:
            return "permission_denied";
        case ErrorCode::resource_exhausted:
            return "resource_exhausted";
        case ErrorCode::canceled:
            return "canceled";
        case ErrorCode::numerical_failure:
            return "numerical_failure";
        case ErrorCode::unsupported_feature:
            return "unsupported_feature";
        case ErrorCode::io_error:
            return "io_error";
        case ErrorCode::not_found:
            return "not_found";
        case ErrorCode::already_exists:
            return "already_exists";
        case ErrorCode::not_repeatable:
            return "not_repeatable";
        case ErrorCode::nothing_to_undo:
            return "nothing_to_undo";
        case ErrorCode::nothing_to_redo:
            return "nothing_to_redo";
        case ErrorCode::reentrant_mutation:
            return "reentrant_mutation";
        case ErrorCode::internal_error:
            return "internal_error";
    }
    return "unknown_error";
}

}  // namespace everygraph::foundation
