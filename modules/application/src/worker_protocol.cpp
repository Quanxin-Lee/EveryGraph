#include "everygraph/application/worker_protocol.hpp"

#include <array>
#include <charconv>
#include <cstddef>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace everygraph::application {
namespace {

constexpr std::array<std::pair<std::string_view, WorkerMessageKind>, 8> kMessageKinds{{
    {"submit", WorkerMessageKind::submit},
    {"accepted", WorkerMessageKind::accepted},
    {"progress", WorkerMessageKind::progress},
    {"cancel", WorkerMessageKind::cancel},
    {"result", WorkerMessageKind::result},
    {"failure", WorkerMessageKind::failure},
    {"superseded", WorkerMessageKind::superseded},
    {"shutdown", WorkerMessageKind::shutdown},
}};

constexpr std::array<std::pair<std::string_view, WorkerFailureReason>, 11> kFailureReasons{{
    {"none", WorkerFailureReason::none},
    {"timed_out", WorkerFailureReason::timed_out},
    {"process_crashed", WorkerFailureReason::process_crashed},
    {"process_exit", WorkerFailureReason::process_exit},
    {"protocol_violation", WorkerFailureReason::protocol_violation},
    {"unsupported_schema", WorkerFailureReason::unsupported_schema},
    {"canceled", WorkerFailureReason::canceled},
    {"superseded", WorkerFailureReason::superseded},
    {"stale_input", WorkerFailureReason::stale_input},
    {"numerical", WorkerFailureReason::numerical},
    {"resource_exhausted", WorkerFailureReason::resource_exhausted},
}};

[[nodiscard]] foundation::Error protocol_error(foundation::ErrorCode code, std::string detail) {
    return foundation::Error{code, "worker frame: " + std::move(detail)};
}

[[nodiscard]] bool is_unreserved(char character) {
    return (character >= 'A' && character <= 'Z') || (character >= 'a' && character <= 'z') ||
           (character >= '0' && character <= '9') || character == '-' || character == '_' ||
           character == '.' || character == '~' || character == ':';
}

[[nodiscard]] std::string percent_encode(std::string_view value) {
    static constexpr char kDigits[] = "0123456789ABCDEF";
    std::string encoded;
    encoded.reserve(value.size());
    for (const char character : value) {
        if (is_unreserved(character)) {
            encoded.push_back(character);
            continue;
        }
        const auto byte = static_cast<unsigned char>(character);
        encoded.push_back('%');
        encoded.push_back(kDigits[byte >> 4U]);
        encoded.push_back(kDigits[byte & 0x0FU]);
    }
    return encoded;
}

[[nodiscard]] std::optional<std::string> percent_decode(std::string_view value) {
    std::string decoded;
    decoded.reserve(value.size());
    for (std::size_t index = 0; index < value.size(); ++index) {
        const char character = value[index];
        if (character != '%') {
            decoded.push_back(character);
            continue;
        }
        if (index + 2 >= value.size()) {
            return std::nullopt;
        }
        const auto high = value[index + 1];
        const auto low = value[index + 2];
        const auto digit = [](char input) -> std::optional<int> {
            if (input >= '0' && input <= '9') {
                return input - '0';
            }
            if (input >= 'a' && input <= 'f') {
                return input - 'a' + 10;
            }
            if (input >= 'A' && input <= 'F') {
                return input - 'A' + 10;
            }
            return std::nullopt;
        };
        const auto high_digit = digit(high);
        const auto low_digit = digit(low);
        if (!high_digit.has_value() || !low_digit.has_value()) {
            return std::nullopt;
        }
        decoded.push_back(static_cast<char>((*high_digit << 4) + *low_digit));
        index += 2;
    }
    return decoded;
}

[[nodiscard]] bool has_control_character(std::string_view value) {
    for (const char character : value) {
        const auto byte = static_cast<unsigned char>(character);
        if (byte < 0x20U || byte == 0x7FU) {
            return true;
        }
    }
    return false;
}

void append_text_field(std::ostringstream& stream, std::string_view key, const std::string& value) {
    if (value.empty()) {
        return;
    }
    stream << ';' << key << '=' << percent_encode(value);
}

void append_int_field(std::ostringstream& stream, std::string_view key, std::int64_t value) {
    stream << ';' << key << '=' << value;
}

void append_uint_field(std::ostringstream& stream, std::string_view key, std::uint64_t value) {
    stream << ';' << key << '=' << value;
}

template <typename Integer>
[[nodiscard]] std::optional<Integer> parse_integer(std::string_view text) {
    Integer value{0};
    const auto* begin = text.data();
    const auto* end = text.data() + text.size();
    const auto [pointer, error] = std::from_chars(begin, end, value);
    if (error != std::errc{} || pointer != end || text.empty()) {
        return std::nullopt;
    }
    return value;
}

[[nodiscard]] std::optional<foundation::Error> check_kind_requirements(
    const WorkerEnvelope& envelope) {
    switch (envelope.kind) {
    case WorkerMessageKind::submit:
        if (envelope.command_id.empty() || envelope.work_units <= 0 ||
            envelope.checkpoint_every_units <= 0) {
            return protocol_error(foundation::ErrorCode::invalid_request,
                                  "submit needs command, positive work units and checkpoint");
        }
        break;
    case WorkerMessageKind::progress:
        if (!envelope.progress.has_value() || envelope.progress->stage.empty() ||
            envelope.progress->completed_units < 0 ||
            (envelope.progress->total_units.has_value() && *envelope.progress->total_units <= 0)) {
            return protocol_error(foundation::ErrorCode::invalid_request,
                                  "progress needs a stage and a non-negative completed count");
        }
        break;
    case WorkerMessageKind::result:
        if (!envelope.payload_number.has_value() || envelope.payload_key.empty()) {
            return protocol_error(foundation::ErrorCode::invalid_request,
                                  "result needs a payload key and number");
        }
        break;
    case WorkerMessageKind::failure:
        if (!envelope.failure_reason.has_value() ||
            *envelope.failure_reason == WorkerFailureReason::none) {
            return protocol_error(foundation::ErrorCode::invalid_request,
                                  "failure needs a reason other than none");
        }
        break;
    case WorkerMessageKind::accepted:
    case WorkerMessageKind::cancel:
    case WorkerMessageKind::superseded:
    case WorkerMessageKind::shutdown:
        break;
    }
    return std::nullopt;
}

}  // namespace

const char* to_string(WorkerMessageKind kind) noexcept {
    for (const auto& [name, value] : kMessageKinds) {
        if (value == kind) {
            return name.data();
        }
    }
    return "unknown";
}

const char* to_string(WorkerFailureReason reason) noexcept {
    for (const auto& [name, value] : kFailureReasons) {
        if (value == reason) {
            return name.data();
        }
    }
    return "unknown";
}

std::optional<WorkerMessageKind> worker_message_kind_from_string(std::string_view name) noexcept {
    for (const auto& [candidate, value] : kMessageKinds) {
        if (candidate == name) {
            return value;
        }
    }
    return std::nullopt;
}

std::optional<WorkerFailureReason> worker_failure_reason_from_string(
    std::string_view name) noexcept {
    for (const auto& candidate : kFailureReasons) {
        if (candidate.first == name) {
            return candidate.second;
        }
    }
    return std::nullopt;
}

std::string WorkerEnvelope::encode() const { return encode_worker_frame(*this); }

std::string encode_worker_frame(const WorkerEnvelope& envelope) {
    std::ostringstream frame;
    frame << "v=" << envelope.schema_version;
    frame << ";k=" << to_string(envelope.kind);
    frame << ";j=" << percent_encode(envelope.job_id);
    append_text_field(frame, "c", envelope.command_id);
    append_text_field(frame, "g", envelope.job_mode);
    append_uint_field(frame, "r", envelope.base_revision);
    append_uint_field(frame, "b", envelope.reservation_bytes);
    append_int_field(frame, "d", envelope.deadline_milliseconds);
    append_int_field(frame, "w", envelope.work_units);
    append_int_field(frame, "p", envelope.checkpoint_every_units);
    if (envelope.total_is_unknown) {
        append_int_field(frame, "z", 1);
    }
    if (envelope.progress.has_value()) {
        const auto& progress = *envelope.progress;
        append_text_field(frame, "s", progress.stage);
        append_int_field(frame, "u", progress.completed_units);
        if (progress.total_units.has_value()) {
            append_int_field(frame, "t", *progress.total_units);
        }
    }
    if (envelope.failure_reason.has_value()) {
        append_text_field(frame, "f", to_string(*envelope.failure_reason));
    }
    append_text_field(frame, "m", envelope.detail);
    append_text_field(frame, "y", envelope.payload_key);
    if (envelope.payload_number.has_value()) {
        append_int_field(frame, "n", *envelope.payload_number);
    }
    if (envelope.payload_text.has_value()) {
        append_text_field(frame, "x", *envelope.payload_text);
    }
    return frame.str();
}

WorkerDecodeResult decode_worker_frame(std::string_view frame) {
    WorkerDecodeResult result;
    if (frame.empty()) {
        result.error = protocol_error(foundation::ErrorCode::invalid_request, "empty frame");
        return result;
    }
    if (frame.size() > kWorkerMaxFrameBytes || frame.find('\n') != std::string_view::npos ||
        frame.find('\r') != std::string_view::npos) {
        result.error = protocol_error(foundation::ErrorCode::invalid_request,
                                      "frame is oversized or spans more than one line");
        return result;
    }

    WorkerEnvelope envelope;
    std::unordered_map<std::string_view, std::size_t> seen;
    bool has_version = false;
    bool has_kind = false;

    std::size_t cursor = 0;
    while (cursor <= frame.size()) {
        const auto separator = frame.find(';', cursor);
        const std::string_view token = separator == std::string_view::npos
                                           ? frame.substr(cursor)
                                           : frame.substr(cursor, separator - cursor);
        cursor = separator == std::string_view::npos ? frame.size() + 1 : separator + 1;

        if (token.empty()) {
            continue;
        }
        const auto assignment = token.find('=');
        if (assignment == std::string_view::npos) {
            result.error = protocol_error(foundation::ErrorCode::invalid_request,
                                          "field without a value separator");
            return result;
        }
        const std::string_view key = token.substr(0, assignment);
        const std::string_view raw_value = token.substr(assignment + 1);
        if (key.empty()) {
            result.error =
                protocol_error(foundation::ErrorCode::invalid_request, "empty field name");
            return result;
        }
        if (++seen[key] > 1) {
            result.error = protocol_error(foundation::ErrorCode::invalid_request,
                                          "duplicate field " + std::string{key});
            return result;
        }
        const auto value = percent_decode(raw_value);
        if (!value.has_value()) {
            result.error = protocol_error(foundation::ErrorCode::invalid_request,
                                          "malformed escape in field " + std::string{key});
            return result;
        }
        // A percent-escaped control character would otherwise reach diagnostics
        // and progress records, where it can forge a log line.
        if (has_control_character(*value)) {
            result.error = protocol_error(
                foundation::ErrorCode::invalid_request,
                "field " + std::string{key} + " carries a control character");
            return result;
        }

        if (key == "v") {
            const auto parsed = parse_integer<std::uint32_t>(*value);
            if (!parsed.has_value()) {
                result.error = protocol_error(foundation::ErrorCode::invalid_request,
                                              "schema version is not an unsigned integer");
                return result;
            }
            envelope.schema_version = *parsed;
            has_version = true;
        } else if (key == "k") {
            const auto kind = worker_message_kind_from_string(*value);
            if (!kind.has_value()) {
                result.error = protocol_error(foundation::ErrorCode::invalid_request,
                                              "unknown message kind " + *value);
                return result;
            }
            envelope.kind = *kind;
            has_kind = true;
        } else if (key == "j") {
            envelope.job_id = *value;
        } else if (key == "c") {
            envelope.command_id = *value;
        } else if (key == "g") {
            envelope.job_mode = *value;
        } else if (key == "r") {
            const auto parsed = parse_integer<std::uint64_t>(*value);
            if (!parsed.has_value()) {
                result.error = protocol_error(foundation::ErrorCode::invalid_request,
                                              "base revision is not an unsigned integer");
                return result;
            }
            envelope.base_revision = *parsed;
        } else if (key == "b") {
            const auto parsed = parse_integer<std::uint64_t>(*value);
            if (!parsed.has_value()) {
                result.error = protocol_error(foundation::ErrorCode::invalid_request,
                                              "reservation is not an unsigned integer");
                return result;
            }
            envelope.reservation_bytes = *parsed;
        } else if (key == "d") {
            const auto parsed = parse_integer<std::int64_t>(*value);
            if (!parsed.has_value()) {
                result.error = protocol_error(foundation::ErrorCode::invalid_request,
                                              "deadline is not an integer");
                return result;
            }
            envelope.deadline_milliseconds = *parsed;
        } else if (key == "w") {
            const auto parsed = parse_integer<std::int64_t>(*value);
            if (!parsed.has_value()) {
                result.error = protocol_error(foundation::ErrorCode::invalid_request,
                                              "work units is not an integer");
                return result;
            }
            envelope.work_units = *parsed;
        } else if (key == "p") {
            const auto parsed = parse_integer<std::int64_t>(*value);
            if (!parsed.has_value()) {
                result.error = protocol_error(foundation::ErrorCode::invalid_request,
                                              "checkpoint interval is not an integer");
                return result;
            }
            envelope.checkpoint_every_units = *parsed;
        } else if (key == "z") {
            const auto parsed = parse_integer<std::int64_t>(*value);
            if (!parsed.has_value()) {
                result.error = protocol_error(foundation::ErrorCode::invalid_request,
                                              "total policy is not an integer");
                return result;
            }
            envelope.total_is_unknown = *parsed != 0;
        } else if (key == "s") {
            if (!envelope.progress.has_value()) {
                envelope.progress = WorkerProgress{};
            }
            envelope.progress->stage = *value;
        } else if (key == "u") {
            const auto parsed = parse_integer<std::int64_t>(*value);
            if (!parsed.has_value()) {
                result.error = protocol_error(foundation::ErrorCode::invalid_request,
                                              "completed units is not an integer");
                return result;
            }
            if (!envelope.progress.has_value()) {
                envelope.progress = WorkerProgress{};
            }
            envelope.progress->completed_units = *parsed;
        } else if (key == "t") {
            const auto parsed = parse_integer<std::int64_t>(*value);
            if (!parsed.has_value()) {
                result.error = protocol_error(foundation::ErrorCode::invalid_request,
                                              "total units is not an integer");
                return result;
            }
            if (!envelope.progress.has_value()) {
                envelope.progress = WorkerProgress{};
            }
            envelope.progress->total_units = *parsed;
        } else if (key == "f") {
            const auto reason = worker_failure_reason_from_string(*value);
            if (!reason.has_value()) {
                result.error = protocol_error(foundation::ErrorCode::invalid_request,
                                              "unknown failure reason " + *value);
                return result;
            }
            envelope.failure_reason = *reason;
        } else if (key == "m") {
            envelope.detail = *value;
        } else if (key == "y") {
            envelope.payload_key = *value;
        } else if (key == "n") {
            const auto parsed = parse_integer<std::int64_t>(*value);
            if (!parsed.has_value()) {
                result.error = protocol_error(foundation::ErrorCode::invalid_request,
                                              "payload number is not an integer");
                return result;
            }
            envelope.payload_number = *parsed;
        } else if (key == "x") {
            envelope.payload_text = *value;
        } else {
            result.error = protocol_error(foundation::ErrorCode::invalid_request,
                                          "unknown field " + std::string{key});
            return result;
        }
    }

    if (!has_version || !has_kind) {
        result.error = protocol_error(foundation::ErrorCode::invalid_request,
                                      "frame is missing the schema version or the kind");
        return result;
    }
    if (envelope.schema_version != kWorkerProtocolSchemaVersion) {
        result.error = protocol_error(foundation::ErrorCode::unsupported_schema_version,
                                      "worker protocol schema " +
                                          std::to_string(envelope.schema_version) + " is not " +
                                          std::to_string(kWorkerProtocolSchemaVersion));
        return result;
    }
    if (envelope.job_id.empty()) {
        result.error = protocol_error(foundation::ErrorCode::invalid_request,
                                      "frame has no task identity");
        return result;
    }

    const auto requirement_error = check_kind_requirements(envelope);
    if (requirement_error.has_value()) {
        result.error = requirement_error;
        return result;
    }
    result.envelope = envelope;
    return result;
}

}  // namespace everygraph::application
