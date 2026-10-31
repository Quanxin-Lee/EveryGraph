#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "everygraph/foundation/error.hpp"
#include "everygraph/foundation/ids.hpp"

namespace everygraph::application {

// Wire contract between the application executor and an isolated worker. It is
// a crash-containment and scheduling protocol, not a sandbox boundary: a worker
// that obeys it is still a trusted program until platform sandbox tests say
// otherwise.
inline constexpr std::uint32_t kWorkerProtocolSchemaVersion = 1;
inline constexpr std::size_t kWorkerMaxFrameBytes = 4096;
inline constexpr std::string_view kWorkerResultPayloadKey = "value";

enum class WorkerMessageKind {
    submit,
    accepted,
    progress,
    cancel,
    result,
    failure,
    superseded,
    shutdown,
};

enum class WorkerFailureReason {
    none,
    timed_out,
    process_crashed,
    process_exit,
    protocol_violation,
    unsupported_schema,
    canceled,
    superseded,
    stale_input,
    numerical,
    resource_exhausted,
};

[[nodiscard]] const char* to_string(WorkerMessageKind kind) noexcept;
[[nodiscard]] const char* to_string(WorkerFailureReason reason) noexcept;
[[nodiscard]] std::optional<WorkerMessageKind> worker_message_kind_from_string(
    std::string_view name) noexcept;
[[nodiscard]] std::optional<WorkerFailureReason> worker_failure_reason_from_string(
    std::string_view name) noexcept;

struct WorkerProgress {
    std::string stage;
    std::int64_t completed_units{0};
    // Absent means the total is unknown, so the update stays indeterminate
    // instead of implying a percentage the worker never measured.
    std::optional<std::int64_t> total_units;

    [[nodiscard]] bool indeterminate() const noexcept { return !total_units.has_value(); }
};

struct WorkerEnvelope {
    std::uint32_t schema_version{kWorkerProtocolSchemaVersion};
    WorkerMessageKind kind{WorkerMessageKind::submit};
    std::string job_id;
    std::string command_id;
    std::string job_mode;
    foundation::Revision base_revision{foundation::kInitialRevision};
    std::uint64_t reservation_bytes{0};
    std::int64_t deadline_milliseconds{0};
    std::int64_t work_units{0};
    std::int64_t checkpoint_every_units{0};
    // Set when the job's total work is not known in advance, so every progress
    // update stays indeterminate instead of inventing a denominator.
    bool total_is_unknown{false};
    std::optional<WorkerProgress> progress;
    std::optional<WorkerFailureReason> failure_reason;
    std::string detail;
    std::string payload_key;
    std::optional<std::int64_t> payload_number;
    std::optional<std::string> payload_text;

    [[nodiscard]] std::string encode() const;
};

struct WorkerDecodeResult {
    std::optional<WorkerEnvelope> envelope;
    std::optional<foundation::Error> error;

    [[nodiscard]] bool ok() const noexcept { return envelope.has_value() && !error.has_value(); }
};

// One line, no embedded newline; every value is percent-encoded so the frame
// cannot smuggle a separator or a line break into the reader.
[[nodiscard]] std::string encode_worker_frame(const WorkerEnvelope& envelope);
[[nodiscard]] WorkerDecodeResult decode_worker_frame(std::string_view frame);

}  // namespace everygraph::application
