// P1-005 worker handshake and cancellation prototype evidence: the line
// protocol codec, the scheduler's admission, progress, cancellation and
// supersession behavior, and the single commit gate that keeps a non-succeeded
// worker candidate out of the document. Every job here is a fake bounded job.
#include <algorithm>
#include <array>
#include <chrono>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "everygraph/application/job_scheduler.hpp"
#include "everygraph/application/command_gateway.hpp"
#include "everygraph/application/command_registry.hpp"
#include "everygraph/application/document.hpp"
#include "everygraph/application/event.hpp"
#include "everygraph/application/value.hpp"
#include "everygraph/application/worker_protocol.hpp"
#include "everygraph/foundation/error.hpp"
#include "everygraph/foundation/ids.hpp"

#ifndef EVERYGRAPH_TEST_WORKER_PROGRAM
#error "EVERYGRAPH_TEST_WORKER_PROGRAM must name the built test worker executable"
#endif

namespace {

using everygraph::application::AdmissionLedger;
using everygraph::application::ChangeKind;
using everygraph::application::CommandDefinition;
using everygraph::application::CommandGateway;
using everygraph::application::CommandOutcome;
using everygraph::application::CommandRequest;
using everygraph::application::CommandRegistry;
using everygraph::application::CommandStatus;
using everygraph::application::CommandTransaction;
using everygraph::application::commit_worker_result;
using everygraph::application::decode_worker_frame;
using everygraph::application::DocumentSnapshot;
using everygraph::application::EffectClass;
using everygraph::application::ExecutionContext;
using everygraph::application::encode_worker_frame;
using everygraph::application::JobOutcome;
using everygraph::application::JobRequest;
using everygraph::application::JobScheduler;
using everygraph::application::JobState;
using everygraph::application::ObjectState;
using everygraph::application::ParameterMap;
using everygraph::application::ProgressAggregator;
using everygraph::application::ProgressRecord;
using everygraph::application::Repeatability;
using everygraph::application::SchedulerConfig;
using everygraph::application::SubmitDisposition;
using everygraph::application::to_string;
using everygraph::application::Value;
using everygraph::application::WorkerBehavior;
using everygraph::application::WorkerCommitContext;
using everygraph::application::WorkerEnvelope;
using everygraph::application::WorkerFailureReason;
using everygraph::application::WorkerMessageKind;
using everygraph::application::WorkerProgress;
using everygraph::application::WorkerTransport;
using everygraph::application::kWorkerMaxFrameBytes;
using everygraph::application::kWorkerProtocolSchemaVersion;
using everygraph::application::kWorkerResultPayloadKey;
using everygraph::application::worker_behavior_from_string;
using everygraph::application::worker_failure_reason_from_string;
using everygraph::application::worker_message_kind_from_string;
using everygraph::foundation::Error;
using everygraph::foundation::ErrorCode;
using everygraph::foundation::ObjectId;

constexpr std::uint32_t kSchemaVersion = 1;
constexpr std::string_view kTargetProperty = "computed";
constexpr everygraph::foundation::ObjectId kTarget{1};

struct TestFailure : std::runtime_error {
    using std::runtime_error::runtime_error;
};

using TestFunction = std::function<void()>;

struct TestCase {
    std::string name;
    TestFunction function;
};

std::vector<TestCase>& test_cases() {
    static std::vector<TestCase> cases;
    return cases;
}

struct TestRegistrar {
    TestRegistrar(std::string name, TestFunction function) {
        test_cases().push_back(TestCase{std::move(name), std::move(function)});
    }
};

#define EG_TEST(name)                                                          \
    static void name();                                                        \
    [[maybe_unused]] static const TestRegistrar name##_registrar(#name, name); \
    static void name()

template <typename Actual, typename Expected>
void require_equal(const Actual& actual, const Expected& expected, const char* actual_expression,
                   const char* expected_expression, const char* file, int line) {
    if (!(actual == expected)) {
        std::ostringstream stream;
        stream << file << ':' << line << ": expected " << actual_expression << " == "
               << expected_expression;
        throw TestFailure(stream.str());
    }
}

#define EG_REQUIRE(expression)                                                          \
    do {                                                                                \
        if (!(expression)) {                                                            \
            std::ostringstream stream;                                                  \
            stream << __FILE__ << ':' << __LINE__ << ": requirement failed: " #expression; \
            throw TestFailure(stream.str());                                            \
        }                                                                               \
    } while (false)

#define EG_REQUIRE_EQ(actual, expected) \
    require_equal((actual), (expected), #actual, #expected, __FILE__, __LINE__)

[[nodiscard]] std::string child_program() {
    return EVERYGRAPH_TEST_WORKER_PROGRAM;
}

[[nodiscard]] JobRequest in_process_request(std::string job_id, std::int64_t work_units,
                                           WorkerBehavior behavior = WorkerBehavior::cooperative) {
    JobRequest request;
    request.job_id = std::move(job_id);
    request.work_units = work_units;
    request.checkpoint_every_units = 1;
    request.behavior = behavior;
    // Jobs that only hold a slot are not timing tests, so their deadline has to
    // stay wide: a slow machine must not flip a held job from succeeded to timed
    // out and make an admission assertion depend on load. Deadline behavior itself
    // is the test that overrides this with a short one.
    request.deadline_milliseconds = 8000;
    return request;
}

[[nodiscard]] JobRequest child_request(std::string job_id, WorkerBehavior behavior,
                                      std::int64_t work_units = 3) {
    JobRequest request;
    request.job_id = std::move(job_id);
    request.transport = WorkerTransport::child_process;
    request.behavior = behavior;
    request.child_program = child_program();
    request.work_units = work_units;
    request.checkpoint_every_units = 1;
    // These tests assert protocol and process behavior, not latency, and a loaded
    // sanitizer run starts a second executable slowly. Deadline behavior itself is
    // the test that overrides this with a short one.
    request.deadline_milliseconds = 8000;
    return request;
}

[[nodiscard]] std::int64_t in_process_candidate(std::int64_t work_units) {
    return work_units * 3 + static_cast<std::int64_t>(kTarget.value);
}

[[nodiscard]] std::int64_t child_candidate(std::int64_t work_units) {
    return work_units * 3 + 1000;
}

void wait_until_running(JobScheduler& scheduler, const std::string& job_id) {
    for (int attempt = 0; attempt < 1000; ++attempt) {
        if (!scheduler.progress(job_id).updates.empty()) {
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{2});
    }
    throw TestFailure("job " + job_id + " never reported a progress update");
}

[[nodiscard]] JobOutcome terminal_outcome(JobScheduler& scheduler, const std::string& job_id) {
    if (!scheduler.await_terminal(job_id, std::chrono::milliseconds{20000})) {
        throw TestFailure("job " + job_id + " did not reach a terminal state");
    }
    const auto found = scheduler.outcome(job_id);
    if (!found.has_value()) {
        throw TestFailure("job " + job_id + " recorded no outcome");
    }
    return *found;
}

// Supersession is decided when the replacement is admitted, but how the old
// worker actually left is only observed when it is reaped, so the exit evidence
// arrives on its own schedule. Waiting for it keeps a test that asserts the
// cooperative stop from racing the observation it depends on.
[[nodiscard]] JobOutcome stopped_superseded_child(
    JobScheduler& scheduler, const std::string& job_id) {
    for (int attempt = 0; attempt < 5000; ++attempt) {
        const auto found = scheduler.superseded_outcome(job_id);
        if (found.has_value() &&
            (found->process_exit_code.has_value() || found->process_signal.has_value())) {
            return *found;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{2});
    }
    throw TestFailure("superseded child " + job_id + " never reported how its process stopped");
}

// Child-process tests depend on how fast this machine spawns and runs a second
// executable, so a failure has to name the state and the host's reason instead
// of only showing a failed comparison.
[[nodiscard]] std::string describe_outcome(const JobOutcome& outcome) {
    std::ostringstream stream;
    stream << "state=" << to_string(outcome.state);
    if (outcome.failure_reason.has_value()) {
        stream << " reason=" << to_string(*outcome.failure_reason);
    }
    if (outcome.error.has_value()) {
        stream << " error=" << everygraph::foundation::to_string(outcome.error->code) << " \""
               << outcome.error->message << '"';
    }
    if (outcome.process_signal.has_value()) {
        stream << " signal=" << *outcome.process_signal;
    }
    if (outcome.process_exit_code.has_value()) {
        stream << " exit=" << *outcome.process_exit_code;
    }
    stream << " checkpoints=" << outcome.checkpoints_observed;
    return stream.str();
}

#define EG_REQUIRE_STATE(outcome, expected)                                                    \
    do {                                                                                       \
        if ((outcome).state != (expected)) {                                                   \
            throw TestFailure(std::string{__FILE__ ":"} + std::to_string(__LINE__) +           \
                              ": expected state " + to_string(expected) + ", got " +           \
                              describe_outcome(outcome));                                      \
        }                                                                                      \
    } while (false)

[[nodiscard]] CommandRegistry compute_registry() {
    CommandDefinition definition;
    definition.command_id = "demo.compute_value";
    definition.supported_schema_versions = {kSchemaVersion};
    definition.effect = EffectClass::reversible_document_write;
    definition.repeatability = Repeatability::repeatable;
    definition.validate_arguments = [](const ParameterMap& arguments) -> std::optional<Error> {
        const auto found = arguments.find(kWorkerResultPayloadKey);
        if (found == arguments.end() || !std::holds_alternative<std::int64_t>(found->second)) {
            return Error{ErrorCode::invalid_argument, "value must be an integer"};
        }
        return std::nullopt;
    };
    definition.execute = [](CommandTransaction& transaction, const CommandRequest& request,
                            const ExecutionContext&) -> std::optional<Error> {
        const auto found = request.arguments.find(kWorkerResultPayloadKey);
        if (found == request.arguments.end()) {
            return Error{ErrorCode::invalid_argument, "value is missing"};
        }
        return transaction.set_property(
            kTarget, std::string{kTargetProperty}, Value{found->second});
    };

    CommandRegistry registry;
    const auto error = registry.register_command(std::move(definition));
    if (error.has_value()) {
        throw TestFailure("registration failed: " + error->message);
    }
    return registry;
}

[[nodiscard]] DocumentSnapshot target_document() {
    ObjectState state;
    state.id = kTarget;
    state.type = "graph";
    DocumentSnapshot snapshot;
    snapshot.objects[kTarget] = state;
    return snapshot;
}

[[nodiscard]] std::optional<std::int64_t> computed_value(const DocumentSnapshot& snapshot) {
    const auto* object = snapshot.find(kTarget);
    if (object == nullptr) {
        return std::nullopt;
    }
    const auto found = object->properties.find(kTargetProperty);
    if (found == object->properties.end()) {
        return std::nullopt;
    }
    const auto* value = std::get_if<std::int64_t>(&found->second);
    if (value == nullptr) {
        return std::nullopt;
    }
    return *value;
}

[[nodiscard]] everygraph::foundation::Revision revision_of(const DocumentSnapshot& snapshot) {
    const auto* object = snapshot.find(kTarget);
    return object == nullptr ? everygraph::foundation::kInitialRevision : object->revision;
}

struct PublishedTracker {
    std::size_t events{0};
    std::size_t changes{0};
};

[[nodiscard]] CommandRequest manual_edit_request(std::string request_id, std::int64_t value) {
    CommandRequest request;
    request.command_id = "demo.compute_value";
    request.schema_version = kSchemaVersion;
    request.request_id = std::move(request_id);
    request.principal = "editor";
    request.project_id = "project-1";
    request.expected_revisions.reads[kTarget] = everygraph::foundation::kInitialRevision;
    request.expected_revisions.writes[kTarget] = everygraph::foundation::kInitialRevision;
    request.arguments[std::string{kWorkerResultPayloadKey}] = value;
    return request;
}

[[nodiscard]] WorkerEnvelope full_submit_frame() {
    WorkerEnvelope frame;
    frame.kind = WorkerMessageKind::submit;
    frame.job_id = "job-1";
    frame.command_id = "demo.compute_value";
    frame.job_mode = "cooperative";
    frame.base_revision = 4;
    frame.reservation_bytes = 2048;
    frame.deadline_milliseconds = 1500;
    frame.work_units = 7;
    frame.checkpoint_every_units = 2;
    return frame;
}

EG_TEST(codec_round_trip_preserves_every_field) {
    const auto original = full_submit_frame();
    const auto decoded = decode_worker_frame(original.encode());
    EG_REQUIRE(decoded.ok());
    const auto& frame = *decoded.envelope;
    EG_REQUIRE_EQ(frame.schema_version, kWorkerProtocolSchemaVersion);
    EG_REQUIRE(frame.kind == WorkerMessageKind::submit);
    EG_REQUIRE_EQ(frame.job_id, original.job_id);
    EG_REQUIRE_EQ(frame.command_id, original.command_id);
    EG_REQUIRE_EQ(frame.job_mode, original.job_mode);
    EG_REQUIRE_EQ(frame.base_revision, original.base_revision);
    EG_REQUIRE_EQ(frame.reservation_bytes, original.reservation_bytes);
    EG_REQUIRE_EQ(frame.deadline_milliseconds, original.deadline_milliseconds);
    EG_REQUIRE_EQ(frame.work_units, original.work_units);
    EG_REQUIRE_EQ(frame.checkpoint_every_units, original.checkpoint_every_units);
    EG_REQUIRE(!frame.total_is_unknown);
}

EG_TEST(codec_round_trip_covers_progress_failure_and_payload_text) {
    WorkerEnvelope progress;
    progress.kind = WorkerMessageKind::progress;
    progress.job_id = "job-2";
    WorkerProgress measured;
    measured.stage = "import";
    measured.completed_units = 3;
    measured.total_units = 9;
    progress.progress = measured;
    const auto decoded_progress = decode_worker_frame(progress.encode());
    EG_REQUIRE(decoded_progress.ok());
    EG_REQUIRE(decoded_progress.envelope->progress.has_value());
    EG_REQUIRE_EQ(decoded_progress.envelope->progress->stage, std::string{"import"});
    EG_REQUIRE_EQ(decoded_progress.envelope->progress->completed_units, std::int64_t{3});
    EG_REQUIRE(decoded_progress.envelope->progress->total_units.has_value());
    EG_REQUIRE_EQ(*decoded_progress.envelope->progress->total_units, std::int64_t{9});
    EG_REQUIRE(!decoded_progress.envelope->progress->indeterminate());

    WorkerEnvelope failure;
    failure.kind = WorkerMessageKind::failure;
    failure.job_id = "job-2";
    failure.failure_reason = WorkerFailureReason::numerical;
    failure.detail = "diverged at unit 4";
    failure.payload_text = "trace-reference";
    const auto decoded_failure = decode_worker_frame(failure.encode());
    EG_REQUIRE(decoded_failure.ok());
    EG_REQUIRE(decoded_failure.envelope->failure_reason.has_value());
    EG_REQUIRE(*decoded_failure.envelope->failure_reason == WorkerFailureReason::numerical);
    EG_REQUIRE_EQ(decoded_failure.envelope->detail, std::string{"diverged at unit 4"});
    EG_REQUIRE(decoded_failure.envelope->payload_text.has_value());
    EG_REQUIRE_EQ(*decoded_failure.envelope->payload_text, std::string{"trace-reference"});

    WorkerEnvelope result;
    result.kind = WorkerMessageKind::result;
    result.job_id = "job-2";
    result.payload_key = std::string{kWorkerResultPayloadKey};
    result.payload_number = -17;
    const auto decoded_result = decode_worker_frame(result.encode());
    EG_REQUIRE(decoded_result.ok());
    EG_REQUIRE(decoded_result.envelope->payload_number.has_value());
    EG_REQUIRE_EQ(*decoded_result.envelope->payload_number, std::int64_t{-17});
}

EG_TEST(codec_rejects_frames_without_identity_or_kind) {
    EG_REQUIRE(!decode_worker_frame("").ok());
    EG_REQUIRE(!decode_worker_frame("k=accepted;j=job-1").ok());
    EG_REQUIRE(!decode_worker_frame("v=1;j=job-1").ok());
    EG_REQUIRE(!decode_worker_frame("v=1;k=accepted").ok());
    const auto rejected = decode_worker_frame("v=1;k=accepted;j=");
    EG_REQUIRE(!rejected.ok());
    EG_REQUIRE(rejected.error->code == ErrorCode::invalid_request);
}

EG_TEST(codec_rejects_unknown_duplicate_and_malformed_fields) {
    EG_REQUIRE(!decode_worker_frame("v=1;k=teleport;j=job-1").ok());
    EG_REQUIRE(!decode_worker_frame("v=1;k=accepted;j=job-1;extra=1").ok());
    EG_REQUIRE(!decode_worker_frame("v=1;k=accepted;j=a;j=b").ok());
    EG_REQUIRE(!decode_worker_frame("v=one;k=accepted;j=job-1").ok());
    EG_REQUIRE(!decode_worker_frame("v=1;k=accepted;j=job-1;w=7.5").ok());
    EG_REQUIRE(!decode_worker_frame("v=1;k=accepted;j=job-1;f=exploded").ok());
    EG_REQUIRE(!decode_worker_frame("v=1;k=accepted;j=job%zz").ok());
    EG_REQUIRE(!decode_worker_frame("v=1;accepted").ok());
}

EG_TEST(codec_rejects_raw_and_escaped_control_characters) {
    const auto raw = decode_worker_frame(std::string{"v=1;k=accepted;j=job"} + '\x01' + "-1");
    EG_REQUIRE(!raw.ok());
    EG_REQUIRE(raw.error.has_value());
    EG_REQUIRE(raw.error->code == ErrorCode::invalid_request);
    EG_REQUIRE(raw.error->message.find("control character") != std::string::npos);
    // An escape sequence decoding to a control byte is refused in every field, not
    // only in the identity, so it cannot forge a log or progress line.
    EG_REQUIRE(!decode_worker_frame("v=1;k=accepted;j=job%09-1").ok());
    EG_REQUIRE(!decode_worker_frame("v=1;k=accepted;j=job%7F-1").ok());
    WorkerEnvelope escaped_detail;
    escaped_detail.kind = WorkerMessageKind::failure;
    escaped_detail.job_id = "job-esc";
    escaped_detail.failure_reason = WorkerFailureReason::protocol_violation;
    escaped_detail.detail = std::string{"note"} + '\x1b' + "tail";
    EG_REQUIRE(!decode_worker_frame(escaped_detail.encode()).ok());
    // Printable text that needs escaping still round-trips.
    escaped_detail.detail = "note;v=2:tail";
    const auto printable = decode_worker_frame(escaped_detail.encode());
    EG_REQUIRE(printable.ok());
    EG_REQUIRE_EQ(printable.envelope->detail, escaped_detail.detail);
}

EG_TEST(codec_bounds_progress_units) {
    WorkerEnvelope progress;
    progress.kind = WorkerMessageKind::progress;
    progress.job_id = "job-bounded";
    WorkerProgress negative_completed;
    negative_completed.stage = "compute";
    negative_completed.completed_units = -1;
    progress.progress = negative_completed;
    const auto completed = decode_worker_frame(progress.encode());
    EG_REQUIRE(!completed.ok());
    EG_REQUIRE(completed.error.has_value());
    EG_REQUIRE(completed.error->code == ErrorCode::invalid_request);

    WorkerProgress zero_total;
    zero_total.stage = "compute";
    zero_total.completed_units = 1;
    zero_total.total_units = 0;
    progress.progress = zero_total;
    EG_REQUIRE(!decode_worker_frame(progress.encode()).ok());

    WorkerProgress accepted;
    accepted.stage = "compute";
    accepted.completed_units = 0;
    accepted.total_units = 4;
    progress.progress = accepted;
    EG_REQUIRE(decode_worker_frame(progress.encode()).ok());
}

EG_TEST(codec_keeps_a_separator_inside_one_frame) {
    auto frame = full_submit_frame();
    frame.job_id = std::string{"job;1:v=9~k=result"};
    frame.detail = std::string{"a=b;c%7Fd~e:f"};
    const auto encoded = encode_worker_frame(frame);
    EG_REQUIRE(encoded.find('\n') == std::string::npos);
    EG_REQUIRE(encoded.find('\r') == std::string::npos);
    const auto decoded = decode_worker_frame(encoded);
    EG_REQUIRE(decoded.ok());
    EG_REQUIRE_EQ(decoded.envelope->job_id, frame.job_id);
    EG_REQUIRE_EQ(decoded.envelope->detail, frame.detail);
}

EG_TEST(codec_refuses_a_field_value_carrying_a_line_break) {
    // Escaping would keep one frame on the wire, but a decoded control byte could
    // forge a later log or progress line, so the line protocol has no such values.
    auto frame = full_submit_frame();
    frame.job_id = std::string{"job"} + '\n' + "1";
    const auto encoded = encode_worker_frame(frame);
    EG_REQUIRE(encoded.find('\n') == std::string::npos);
    EG_REQUIRE(!decode_worker_frame(encoded).ok());

    auto detail = full_submit_frame();
    detail.detail = std::string{"note"} + '\r' + "tail";
    EG_REQUIRE(!decode_worker_frame(encode_worker_frame(detail)).ok());
}

EG_TEST(codec_rejects_oversized_and_multiline_frames) {
    const std::string oversized(kWorkerMaxFrameBytes + 1, 'v');
    EG_REQUIRE(!decode_worker_frame(oversized).ok());
    EG_REQUIRE(!decode_worker_frame("v=1;k=accepted;j=job-1\nv=1;k=accepted;j=job-2").ok());
}

EG_TEST(codec_rejects_an_unsupported_protocol_schema) {
    auto frame = full_submit_frame();
    frame.schema_version = kWorkerProtocolSchemaVersion + 1;
    const auto decoded = decode_worker_frame(encode_worker_frame(frame));
    EG_REQUIRE(!decoded.ok());
    EG_REQUIRE(decoded.error.has_value());
    EG_REQUIRE(decoded.error->code == ErrorCode::unsupported_schema_version);
}

EG_TEST(codec_enforces_per_kind_requirements) {
    auto submit = full_submit_frame();
    submit.command_id.clear();
    EG_REQUIRE(!decode_worker_frame(submit.encode()).ok());
    submit = full_submit_frame();
    submit.work_units = 0;
    EG_REQUIRE(!decode_worker_frame(submit.encode()).ok());

    WorkerEnvelope progress;
    progress.kind = WorkerMessageKind::progress;
    progress.job_id = "job-3";
    EG_REQUIRE(!decode_worker_frame(progress.encode()).ok());
    WorkerProgress stageless;
    stageless.completed_units = 1;
    progress.progress = stageless;
    EG_REQUIRE(!decode_worker_frame(progress.encode()).ok());

    WorkerEnvelope result;
    result.kind = WorkerMessageKind::result;
    result.job_id = "job-3";
    result.payload_number = 5;
    EG_REQUIRE(!decode_worker_frame(result.encode()).ok());

    WorkerEnvelope failure;
    failure.kind = WorkerMessageKind::failure;
    failure.job_id = "job-3";
    EG_REQUIRE(!decode_worker_frame(failure.encode()).ok());
    failure.failure_reason = WorkerFailureReason::none;
    EG_REQUIRE(!decode_worker_frame(failure.encode()).ok());

    for (const auto kind : {WorkerMessageKind::accepted, WorkerMessageKind::cancel,
                            WorkerMessageKind::superseded, WorkerMessageKind::shutdown}) {
        WorkerEnvelope control;
        control.kind = kind;
        control.job_id = "job-3";
        EG_REQUIRE(decode_worker_frame(control.encode()).ok());
    }
}

EG_TEST(protocol_names_are_stable_for_every_kind_and_reason) {
    constexpr std::array<std::string_view, 8> kKinds{
        "submit", "accepted", "progress", "cancel", "result", "failure", "superseded", "shutdown"};
    for (const auto name : kKinds) {
        const auto kind = worker_message_kind_from_string(name);
        EG_REQUIRE(kind.has_value());
        EG_REQUIRE_EQ(std::string_view{to_string(*kind)}, name);
    }
    EG_REQUIRE(!worker_message_kind_from_string("progress ").has_value());

    constexpr std::array<std::string_view, 11> kReasons{
        "none",           "timed_out",          "process_crashed",    "process_exit",
        "protocol_violation", "unsupported_schema", "canceled",       "superseded",
        "stale_input",    "numerical",          "resource_exhausted"};
    for (const auto name : kReasons) {
        const auto reason = worker_failure_reason_from_string(name);
        EG_REQUIRE(reason.has_value());
        EG_REQUIRE_EQ(std::string_view{to_string(*reason)}, name);
    }
    EG_REQUIRE(!worker_failure_reason_from_string("permission_denied").has_value());

    for (const auto behavior : {WorkerBehavior::cooperative, WorkerBehavior::numerical_failure,
                                WorkerBehavior::unresponsive, WorkerBehavior::process_crash,
                                WorkerBehavior::process_exit, WorkerBehavior::ignores_cancel,
                                WorkerBehavior::malformed_frame, WorkerBehavior::wrong_schema,
                                WorkerBehavior::late_result, WorkerBehavior::wrong_job_identity,
                                WorkerBehavior::echoes_host_frame,
                                WorkerBehavior::progress_overrun}) {
        const auto parsed = worker_behavior_from_string(to_string(behavior));
        EG_REQUIRE(parsed.has_value());
        EG_REQUIRE(*parsed == behavior);
    }
    EG_REQUIRE(!worker_behavior_from_string("not_a_worker_mode").has_value());
}

EG_TEST(progress_total_policy_survives_the_wire) {
    WorkerEnvelope known;
    known.kind = WorkerMessageKind::progress;
    known.job_id = "job-4";
    WorkerProgress measured;
    measured.stage = "compute";
    measured.completed_units = 2;
    measured.total_units = 8;
    known.progress = measured;
    EG_REQUIRE(!decode_worker_frame(known.encode()).envelope->total_is_unknown);

    WorkerEnvelope unknown = known;
    unknown.total_is_unknown = true;
    unknown.progress->total_units = std::nullopt;
    const auto decoded = decode_worker_frame(unknown.encode());
    EG_REQUIRE(decoded.ok());
    EG_REQUIRE(decoded.envelope->total_is_unknown);
    EG_REQUIRE(decoded.envelope->progress.has_value());
    EG_REQUIRE(decoded.envelope->progress->indeterminate());
}

EG_TEST(admission_ledger_reserves_releases_and_rejects) {
    AdmissionLedger ledger{1000};
    EG_REQUIRE(!ledger.reserve("a", 600).has_value());
    const auto over_budget = ledger.reserve("b", 600);
    EG_REQUIRE(over_budget.has_value());
    EG_REQUIRE(over_budget->code == ErrorCode::resource_exhausted);
    ledger.release("a");
    EG_REQUIRE(!ledger.reserve("b", 600).has_value());
    EG_REQUIRE_EQ(ledger.reservation_count(), std::size_t{1});
    EG_REQUIRE_EQ(ledger.committed_bytes(), std::uint64_t{600});
    EG_REQUIRE_EQ(ledger.available_bytes(), std::uint64_t{400});
    const auto duplicate = ledger.reserve("b", 100);
    EG_REQUIRE(duplicate.has_value());
    EG_REQUIRE(duplicate->code == ErrorCode::already_exists);
    ledger.release("never-held");
    EG_REQUIRE_EQ(ledger.reservation_count(), std::size_t{1});
    const auto larger_than_budget = ledger.reserve("c", 5000);
    EG_REQUIRE(larger_than_budget.has_value());
    EG_REQUIRE(larger_than_budget->code == ErrorCode::resource_exhausted);
    EG_REQUIRE(!ledger.holds("c"));
}

EG_TEST(progress_aggregator_bounds_and_coalesces_updates) {
    ProgressAggregator aggregator{2};
    aggregator.push(ProgressRecord{"stage-a", 1, std::optional<std::int64_t>{10}});
    aggregator.push(ProgressRecord{"stage-b", 2, std::optional<std::int64_t>{10}});
    aggregator.push(ProgressRecord{"stage-c", 3, std::optional<std::int64_t>{10}});
    EG_REQUIRE_EQ(aggregator.size(), std::size_t{2});
    EG_REQUIRE_EQ(aggregator.coalesced_count(), std::size_t{1});
    aggregator.push(ProgressRecord{"stage-c", 4, std::optional<std::int64_t>{10}});
    EG_REQUIRE_EQ(aggregator.size(), std::size_t{2});
    EG_REQUIRE_EQ(aggregator.coalesced_count(), std::size_t{2});
    const auto snapshot = aggregator.snapshot();
    EG_REQUIRE_EQ(snapshot.size(), std::size_t{2});
    EG_REQUIRE_EQ(snapshot[0].stage, std::string{"stage-b"});
    EG_REQUIRE_EQ(snapshot[1].completed_units, std::int64_t{4});
    EG_REQUIRE_EQ(aggregator.capacity(), std::size_t{2});
}

EG_TEST(duplicate_submission_is_rejected_without_a_second_reservation) {
    JobScheduler scheduler{SchedulerConfig{}};
    const auto accepted = scheduler.submit(in_process_request("dup-1", 40));
    EG_REQUIRE(accepted.disposition == SubmitDisposition::accepted);
    const auto duplicate = scheduler.submit(in_process_request("dup-1", 40));
    EG_REQUIRE(duplicate.disposition == SubmitDisposition::duplicate_rejected);
    EG_REQUIRE(duplicate.error.has_value());
    EG_REQUIRE(duplicate.error->code == ErrorCode::already_exists);
    EG_REQUIRE_EQ(scheduler.known_job_count(), std::size_t{1});
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{1});
    (void)scheduler.request_cancellation("dup-1");
    EG_REQUIRE(terminal_outcome(scheduler, "dup-1").state == JobState::canceled);
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
}

EG_TEST(rejected_requests_never_reach_a_worker) {
    JobScheduler scheduler{SchedulerConfig{}};
    EG_REQUIRE(scheduler.submit(in_process_request("", 4)).disposition ==
               SubmitDisposition::rejected);
    JobRequest no_units = in_process_request("bad-units", 4);
    no_units.work_units = 0;
    EG_REQUIRE(scheduler.submit(no_units).disposition == SubmitDisposition::rejected);
    JobRequest no_deadline = in_process_request("bad-deadline", 4);
    no_deadline.deadline_milliseconds = 0;
    EG_REQUIRE(scheduler.submit(no_deadline).disposition == SubmitDisposition::rejected);
    JobRequest no_program = child_request("bad-program", WorkerBehavior::cooperative);
    no_program.child_program.clear();
    const auto program_error = scheduler.submit(no_program);
    EG_REQUIRE(program_error.disposition == SubmitDisposition::rejected);
    EG_REQUIRE(program_error.error->code == ErrorCode::invalid_request);
    JobRequest child_only = in_process_request("bad-behavior", 4);
    child_only.behavior = WorkerBehavior::process_crash;
    EG_REQUIRE(scheduler.submit(child_only).error->code == ErrorCode::unsupported_feature);
    EG_REQUIRE_EQ(scheduler.known_job_count(), std::size_t{0});
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
}

EG_TEST(over_budget_admission_is_rejected_before_execution) {
    JobScheduler scheduler{SchedulerConfig{}};
    JobRequest request = in_process_request("budget-1", 4);
    request.estimated_bytes = scheduler.config().memory_budget_bytes + 1;
    const auto rejected = scheduler.submit(request);
    EG_REQUIRE(rejected.disposition == SubmitDisposition::rejected);
    EG_REQUIRE(rejected.error.has_value());
    EG_REQUIRE(rejected.error->code == ErrorCode::resource_exhausted);
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
    EG_REQUIRE_EQ(scheduler.available_bytes(), scheduler.config().memory_budget_bytes);
}

EG_TEST(bounded_pending_queue_rejects_extra_jobs) {
    SchedulerConfig config;
    config.worker_threads = 1;
    config.max_pending_jobs = 2;
    JobScheduler scheduler{config};

    const auto blocker = scheduler.submit(in_process_request("queue-blocker", 100));
    EG_REQUIRE(blocker.disposition == SubmitDisposition::accepted);
    wait_until_running(scheduler, "queue-blocker");

    EG_REQUIRE(scheduler.submit(in_process_request("queue-1", 4)).accepted());
    EG_REQUIRE(scheduler.submit(in_process_request("queue-2", 4)).accepted());
    const auto overflow = scheduler.submit(in_process_request("queue-3", 4));
    EG_REQUIRE(overflow.disposition == SubmitDisposition::rejected);
    EG_REQUIRE(overflow.error->code == ErrorCode::resource_exhausted);
    EG_REQUIRE_EQ(scheduler.queue_depth(), std::size_t{2});

    (void)scheduler.request_cancellation("queue-1");
    (void)scheduler.request_cancellation("queue-2");
    EG_REQUIRE(terminal_outcome(scheduler, "queue-blocker").terminal());
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
}

EG_TEST(queued_cancellation_never_starts_a_worker) {
    SchedulerConfig config;
    config.worker_threads = 1;
    JobScheduler scheduler{config};
    const auto blocker = scheduler.submit(in_process_request("start-blocker", 100));
    EG_REQUIRE(blocker.disposition == SubmitDisposition::accepted);
    wait_until_running(scheduler, "start-blocker");

    EG_REQUIRE(scheduler.submit(in_process_request("queued-1", 4)).accepted());
    EG_REQUIRE(scheduler.request_cancellation("queued-1"));
    const auto outcome = terminal_outcome(scheduler, "queued-1");
    EG_REQUIRE(outcome.state == JobState::canceled);
    EG_REQUIRE_EQ(outcome.checkpoints_observed, std::int64_t{0});
    EG_REQUIRE(!outcome.result_value.has_value());
    EG_REQUIRE(outcome.error.has_value());
    EG_REQUIRE(outcome.error->code == ErrorCode::canceled);
    EG_REQUIRE_EQ(scheduler.queue_depth(), std::size_t{0});

    EG_REQUIRE(terminal_outcome(scheduler, "start-blocker").state == JobState::succeeded);
}

EG_TEST(cancellation_stops_an_in_process_job_at_a_checkpoint) {
    JobScheduler scheduler{SchedulerConfig{}};
    const auto accepted = scheduler.submit(in_process_request("cancel-1", 400));
    EG_REQUIRE(accepted.accepted());
    wait_until_running(scheduler, "cancel-1");
    EG_REQUIRE(scheduler.request_cancellation("cancel-1"));

    const auto outcome = terminal_outcome(scheduler, "cancel-1");
    EG_REQUIRE(outcome.state == JobState::canceled);
    EG_REQUIRE(outcome.cancel_requested);
    EG_REQUIRE(outcome.checkpoints_observed < 400);
    EG_REQUIRE(!outcome.result_value.has_value());
    EG_REQUIRE(outcome.error.has_value());
    EG_REQUIRE(outcome.error->code == ErrorCode::canceled);
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
    EG_REQUIRE_EQ(scheduler.available_bytes(), scheduler.config().memory_budget_bytes);
}

EG_TEST(in_process_numerical_failure_keeps_a_structured_error) {
    JobScheduler scheduler{SchedulerConfig{}};
    EG_REQUIRE(scheduler
                   .submit(in_process_request("numeric-1", 6, WorkerBehavior::numerical_failure))
                   .accepted());
    const auto outcome = terminal_outcome(scheduler, "numeric-1");
    EG_REQUIRE(outcome.state == JobState::failed);
    EG_REQUIRE(outcome.failure_reason.has_value());
    EG_REQUIRE(*outcome.failure_reason == WorkerFailureReason::numerical);
    EG_REQUIRE(outcome.error.has_value());
    EG_REQUIRE(outcome.error->code == ErrorCode::numerical_failure);
    EG_REQUIRE(!outcome.error->message.empty());
    EG_REQUIRE(!outcome.result_value.has_value());
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
}

EG_TEST(in_process_deadline_is_enforced) {
    JobScheduler scheduler{SchedulerConfig{}};
    JobRequest request = in_process_request("deadline-1", 200);
    request.deadline_milliseconds = 10;
    EG_REQUIRE(scheduler.submit(request).accepted());
    const auto outcome = terminal_outcome(scheduler, "deadline-1");
    EG_REQUIRE(outcome.state == JobState::failed);
    EG_REQUIRE(outcome.failure_reason.has_value());
    EG_REQUIRE(*outcome.failure_reason == WorkerFailureReason::timed_out);
    EG_REQUIRE(outcome.error.has_value());
    EG_REQUIRE(outcome.error->code == ErrorCode::io_error);
    EG_REQUIRE(!outcome.result_value.has_value());
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
}

EG_TEST(progress_updates_stay_bounded_end_to_end) {
    SchedulerConfig config;
    config.max_progress_updates_per_job = 2;
    JobScheduler scheduler{config};
    EG_REQUIRE(scheduler.submit(in_process_request("progress-1", 12)).accepted());
    EG_REQUIRE(terminal_outcome(scheduler, "progress-1").state == JobState::succeeded);

    const auto progress = scheduler.progress("progress-1");
    EG_REQUIRE(progress.updates.size() <= config.max_progress_updates_per_job);
    EG_REQUIRE(!progress.updates.empty());
    EG_REQUIRE_EQ(progress.coalesced_count, std::size_t{11});
    EG_REQUIRE(!progress.indeterminate);
    EG_REQUIRE_EQ(progress.updates[0].completed_units, std::int64_t{12});
}

EG_TEST(unknown_total_stays_indeterminate) {
    JobScheduler scheduler{SchedulerConfig{}};
    JobRequest request = in_process_request("indeterminate-1", 3);
    request.total_is_unknown = true;
    EG_REQUIRE(scheduler.submit(request).accepted());
    EG_REQUIRE(terminal_outcome(scheduler, "indeterminate-1").state == JobState::succeeded);

    const auto progress = scheduler.progress("indeterminate-1");
    EG_REQUIRE(progress.indeterminate);
    EG_REQUIRE(!progress.updates.empty());
    for (const auto& update : progress.updates) {
        EG_REQUIRE(!update.total_units.has_value());
        EG_REQUIRE(update.indeterminate());
    }
}

EG_TEST(task_identity_and_schema_travel_with_the_outcome) {
    JobScheduler scheduler{SchedulerConfig{}};
    JobRequest request = in_process_request("identity-1", 2);
    request.command_id = "demo.compute_value";
    request.command_schema_version = 1;
    request.target_object = ObjectId{7};
    request.base_revision = 3;
    EG_REQUIRE(scheduler.submit(request).accepted());
    const auto outcome = terminal_outcome(scheduler, "identity-1");
    EG_REQUIRE_EQ(outcome.job_id, std::string{"identity-1"});
    EG_REQUIRE_EQ(outcome.command_id, std::string{"demo.compute_value"});
    EG_REQUIRE_EQ(outcome.command_schema_version, std::uint32_t{1});
    EG_REQUIRE(outcome.target_object == ObjectId{7});
    EG_REQUIRE_EQ(outcome.base_revision, std::uint64_t{3});
    EG_REQUIRE_EQ(outcome.result_value.value_or(0), std::int64_t{2 * 3 + 7});
    EG_REQUIRE(to_string(outcome.state) == std::string_view{"succeeded"});
}

EG_TEST(reservations_release_on_every_terminal_state) {
    JobScheduler scheduler{SchedulerConfig{}};
    const auto budget = scheduler.config().memory_budget_bytes;
    EG_REQUIRE(scheduler.submit(in_process_request("release-ok", 2)).accepted());
    EG_REQUIRE_EQ(scheduler.available_bytes(), budget - 1024);
    EG_REQUIRE(terminal_outcome(scheduler, "release-ok").succeeded());
    EG_REQUIRE_EQ(scheduler.available_bytes(), budget);

    EG_REQUIRE(scheduler.submit(in_process_request("release-cancel", 200)).accepted());
    wait_until_running(scheduler, "release-cancel");
    (void)scheduler.request_cancellation("release-cancel");
    EG_REQUIRE(terminal_outcome(scheduler, "release-cancel").state == JobState::canceled);
    EG_REQUIRE_EQ(scheduler.available_bytes(), budget);

    EG_REQUIRE(scheduler.submit(
                   in_process_request("release-fail", 6, WorkerBehavior::numerical_failure))
                   .accepted());
    EG_REQUIRE(terminal_outcome(scheduler, "release-fail").state == JobState::failed);
    EG_REQUIRE_EQ(scheduler.available_bytes(), budget);
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
}

EG_TEST(superseding_submission_replaces_the_old_candidate) {
    JobScheduler scheduler{SchedulerConfig{}};
    EG_REQUIRE(scheduler.submit(in_process_request("super-1", 400)).accepted());
    wait_until_running(scheduler, "super-1");

    const auto replacement = scheduler.submit(in_process_request("super-1", 3));
    EG_REQUIRE(replacement.disposition == SubmitDisposition::superseded_previous);
    const auto superseded = scheduler.superseded_outcome("super-1");
    EG_REQUIRE(superseded.has_value());
    EG_REQUIRE(superseded->state == JobState::superseded);
    EG_REQUIRE(superseded->error.has_value());
    EG_REQUIRE(superseded->error->code == ErrorCode::canceled);
    EG_REQUIRE(!superseded->result_value.has_value());

    const auto outcome = terminal_outcome(scheduler, "super-1");
    EG_REQUIRE(outcome.state == JobState::succeeded);
    EG_REQUIRE_EQ(outcome.result_value.value_or(0), in_process_candidate(3));
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
    EG_REQUIRE_EQ(scheduler.known_job_count(), std::size_t{1});

    CommandGateway gateway{compute_registry(), target_document()};
    PublishedTracker published;
    gateway.subscribe([&published](const everygraph::application::DocumentEvent& event) {
        ++published.events;
        published.changes += event.changes.size();
    });
    const auto stale_commit =
        commit_worker_result(gateway, *superseded, WorkerCommitContext{"super-commit"});
    EG_REQUIRE(!stale_commit.succeeded());
    EG_REQUIRE_EQ(published.events, std::size_t{0});
    const auto commit =
        commit_worker_result(gateway, outcome, WorkerCommitContext{"super-commit-2"});
    EG_REQUIRE(commit.succeeded());
    EG_REQUIRE_EQ(published.events, std::size_t{1});
    EG_REQUIRE_EQ(computed_value(gateway.snapshot()).value_or(0), in_process_candidate(3));
}

// A terminal incumbent is not superseded: the identity is free, so the new
// submission is plain `accepted` and its record replaces the finished one. The
// commit-before-reuse ordering is the caller's obligation, pinned here rather
// than only in the interface comment.
EG_TEST(resubmitting_a_terminal_identity_starts_a_fresh_submission) {
    JobScheduler scheduler{SchedulerConfig{}};
    EG_REQUIRE(scheduler.submit(in_process_request("reuse-1", 2)).accepted());
    const auto first = terminal_outcome(scheduler, "reuse-1");
    EG_REQUIRE_STATE(first, JobState::succeeded);
    EG_REQUIRE_EQ(first.result_value.value_or(0), in_process_candidate(2));

    const auto again = scheduler.submit(in_process_request("reuse-1", 3));
    EG_REQUIRE(again.disposition == SubmitDisposition::accepted);
    EG_REQUIRE(!again.error.has_value());
    EG_REQUIRE(!scheduler.superseded_outcome("reuse-1").has_value());

    const auto second = terminal_outcome(scheduler, "reuse-1");
    EG_REQUIRE_STATE(second, JobState::succeeded);
    EG_REQUIRE_EQ(second.result_value.value_or(0), in_process_candidate(3));
    EG_REQUIRE_EQ(scheduler.known_job_count(), std::size_t{1});
    // Reuse is not a second reservation: the finished submission released its
    // own before the replacement was admitted.
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});

    // The *identical* payload of a terminal incumbent is still a duplicate, because
    // reservation keys are never pruned: only a different payload restarts the
    // identity, and a candidate must be committed before its identity is reused.
    const auto repeat = scheduler.submit(in_process_request("reuse-1", 3));
    EG_REQUIRE(repeat.disposition == SubmitDisposition::duplicate_rejected);
    EG_REQUIRE(repeat.error.has_value());
    EG_REQUIRE(repeat.error->code == ErrorCode::already_exists);
    EG_REQUIRE_EQ(scheduler.known_job_count(), std::size_t{1});
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
}

EG_TEST(superseding_a_queued_job_runs_the_replacement_in_the_same_slot) {
    SchedulerConfig config;
    config.worker_threads = 1;
    config.max_pending_jobs = 1;
    JobScheduler scheduler{config};

    // Occupy the only worker so the next job stays queued in the only slot.
    EG_REQUIRE(scheduler.submit(in_process_request("slot-holder", 300)).accepted());
    wait_until_running(scheduler, "slot-holder");
    EG_REQUIRE(scheduler.submit(in_process_request("slot-queued", 200)).accepted());
    EG_REQUIRE_EQ(scheduler.queue_depth(), std::size_t{1});

    // A full queue must not refuse a replacement: it takes over the slot the
    // submission it supersedes already holds.
    const auto replacement = scheduler.submit(in_process_request("slot-queued", 2));
    EG_REQUIRE(replacement.disposition == SubmitDisposition::superseded_previous);
    EG_REQUIRE_EQ(scheduler.queue_depth(), std::size_t{1});
    EG_REQUIRE_EQ(scheduler.known_job_count(), std::size_t{2});

    const auto superseded = scheduler.superseded_outcome("slot-queued");
    EG_REQUIRE(superseded.has_value());
    EG_REQUIRE(superseded->state == JobState::superseded);
    const auto outcome = terminal_outcome(scheduler, "slot-queued");
    EG_REQUIRE(outcome.state == JobState::succeeded);
    EG_REQUIRE_EQ(outcome.result_value.value_or(0), in_process_candidate(2));
    EG_REQUIRE(terminal_outcome(scheduler, "slot-holder").state == JobState::succeeded);
    EG_REQUIRE_EQ(scheduler.queue_depth(), std::size_t{0});
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
}

EG_TEST(a_replacement_over_budget_leaves_the_incumbent_running) {
    JobScheduler scheduler{SchedulerConfig{}};
    const auto budget = scheduler.config().memory_budget_bytes;
    JobRequest incumbent = in_process_request("budget-incumbent", 200);
    incumbent.estimated_bytes = budget / 2;
    EG_REQUIRE(scheduler.submit(incumbent).accepted());
    wait_until_running(scheduler, "budget-incumbent");

    JobRequest replacement = in_process_request("budget-incumbent", 2);
    replacement.estimated_bytes = budget + 1;
    const auto rejected = scheduler.submit(replacement);
    EG_REQUIRE(rejected.disposition == SubmitDisposition::rejected);
    EG_REQUIRE(rejected.error.has_value());
    EG_REQUIRE(rejected.error->code == ErrorCode::resource_exhausted);
    // A replacement the ledger refuses must not stop the work already in flight.
    EG_REQUIRE(!scheduler.superseded_outcome("budget-incumbent").has_value());
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{1});
    EG_REQUIRE_EQ(scheduler.committed_bytes(), budget / 2);
    const auto outcome = terminal_outcome(scheduler, "budget-incumbent");
    EG_REQUIRE(outcome.state == JobState::succeeded);
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
}

EG_TEST(a_replacement_counts_the_incumbents_reservation_as_headroom) {
    JobScheduler scheduler{SchedulerConfig{}};
    const auto budget = scheduler.config().memory_budget_bytes;
    JobRequest incumbent = in_process_request("headroom-1", 200);
    incumbent.estimated_bytes = budget / 2;
    EG_REQUIRE(scheduler.submit(incumbent).accepted());
    wait_until_running(scheduler, "headroom-1");

    // Larger than what is free, but affordable once the incumbent's own bytes are
    // returned to the ledger by supersession.
    JobRequest replacement = in_process_request("headroom-1", 2);
    replacement.estimated_bytes = budget / 2 + budget / 4;
    EG_REQUIRE(scheduler.submit(replacement).disposition == SubmitDisposition::superseded_previous);
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{1});
    EG_REQUIRE_EQ(scheduler.committed_bytes(), budget / 2 + budget / 4);
    EG_REQUIRE(scheduler.superseded_outcome("headroom-1").has_value());
    EG_REQUIRE(terminal_outcome(scheduler, "headroom-1").state == JobState::succeeded);
    EG_REQUIRE_EQ(scheduler.available_bytes(), budget);
}

EG_TEST(superseding_a_child_job_announces_the_stop_and_runs_the_replacement) {
    JobScheduler scheduler{SchedulerConfig{}};
    JobRequest incumbent = child_request("child-super", WorkerBehavior::cooperative, 200);
    // A grace longer than the worker's checkpoint interval keeps this a cooperative
    // stop: the escalation path is the test that uses a short one.
    incumbent.cancel_grace_milliseconds = 2000;
    EG_REQUIRE(scheduler.submit(incumbent).accepted());
    wait_until_running(scheduler, "child-super");

    const auto replacement =
        scheduler.submit(child_request("child-super", WorkerBehavior::cooperative, 2));
    EG_REQUIRE(replacement.disposition == SubmitDisposition::superseded_previous);

    const auto outcome = terminal_outcome(scheduler, "child-super");
    EG_REQUIRE_STATE(outcome, JobState::succeeded);
    EG_REQUIRE_EQ(outcome.result_value.value_or(0), child_candidate(2));
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});

    const auto superseded = stopped_superseded_child(scheduler, "child-super");
    EG_REQUIRE_STATE(superseded, JobState::superseded);
    EG_REQUIRE(!superseded.result_value.has_value());
    EG_REQUIRE(superseded.error.has_value());
    EG_REQUIRE(superseded.error->code == ErrorCode::canceled);
    // The stop has to reach the worker, and the way it left says whether it did:
    // a process that exited cleanly after reporting superseded proves the frame
    // arrived, where a killed one would only prove the host gave up on it.
    EG_REQUIRE(!superseded.escalated_termination);
    EG_REQUIRE(superseded.process_id.has_value());
    EG_REQUIRE(!superseded.process_signal.has_value());
    EG_REQUIRE(superseded.process_exit_code.has_value());
    EG_REQUIRE_EQ(*superseded.process_exit_code, 0);

    // The superseded child candidate stays out of the document.
    CommandGateway gateway{compute_registry(), target_document()};
    std::size_t published = 0;
    gateway.subscribe([&published](const everygraph::application::DocumentEvent&) { ++published; });
    const auto sequence = gateway.last_sequence();
    const auto commit =
        commit_worker_result(gateway, superseded, WorkerCommitContext{"child-super-commit"});
    EG_REQUIRE(commit.status == CommandStatus::canceled);
    EG_REQUIRE_EQ(published, std::size_t{0});
    EG_REQUIRE_EQ(gateway.last_sequence(), sequence);
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{0});
    EG_REQUIRE(!computed_value(gateway.snapshot()).has_value());
}

EG_TEST(superseding_a_child_job_that_ignores_the_stop_records_the_termination) {
    JobScheduler scheduler{SchedulerConfig{}};
    JobRequest incumbent = child_request("child-super-kill", WorkerBehavior::ignores_cancel, 200);
    // Shorter than the worker's own checkpoint loop, so the cooperative stop
    // cannot land inside the grace window and the host has to escalate.
    incumbent.cancel_grace_milliseconds = 150;
    EG_REQUIRE(scheduler.submit(incumbent).accepted());
    wait_until_running(scheduler, "child-super-kill");

    const auto replacement =
        scheduler.submit(child_request("child-super-kill", WorkerBehavior::cooperative, 2));
    EG_REQUIRE(replacement.disposition == SubmitDisposition::superseded_previous);

    const auto outcome = terminal_outcome(scheduler, "child-super-kill");
    EG_REQUIRE_STATE(outcome, JobState::succeeded);
    EG_REQUIRE_EQ(outcome.result_value.value_or(0), child_candidate(2));

    const auto superseded = stopped_superseded_child(scheduler, "child-super-kill");
    EG_REQUIRE_STATE(superseded, JobState::superseded);
    EG_REQUIRE(!superseded.result_value.has_value());
    // The record was published when the replacement was admitted, before the kill
    // was observed, so this is the assertion that proves the observation reached
    // the already-terminal record instead of being dropped.
    EG_REQUIRE(superseded.escalated_termination);
    EG_REQUIRE(superseded.process_id.has_value());
    EG_REQUIRE(superseded.process_signal.has_value());
    EG_REQUIRE_EQ(*superseded.process_signal, SIGKILL);
    EG_REQUIRE(!superseded.process_exit_code.has_value());
    EG_REQUIRE(superseded.error.has_value());
    EG_REQUIRE(superseded.error->code == ErrorCode::canceled);

    CommandGateway gateway{compute_registry(), target_document()};
    std::size_t published = 0;
    gateway.subscribe([&published](const everygraph::application::DocumentEvent&) { ++published; });
    const auto sequence = gateway.last_sequence();
    const auto commit =
        commit_worker_result(gateway, superseded, WorkerCommitContext{"child-super-kill-commit"});
    EG_REQUIRE(commit.status == CommandStatus::canceled);
    EG_REQUIRE_EQ(published, std::size_t{0});
    EG_REQUIRE_EQ(gateway.last_sequence(), sequence);
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{0});
    EG_REQUIRE(!computed_value(gateway.snapshot()).has_value());
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
}

EG_TEST(child_cooperative_job_completes_over_the_protocol) {
    JobScheduler scheduler{SchedulerConfig{}};
    const auto accepted =
        scheduler.submit(child_request("child-ok", WorkerBehavior::cooperative, 3));
    EG_REQUIRE(accepted.accepted());
    const auto outcome = terminal_outcome(scheduler, "child-ok");
    EG_REQUIRE_STATE(outcome, JobState::succeeded);
    EG_REQUIRE_EQ(outcome.result_value.value_or(0), child_candidate(3));
    EG_REQUIRE(outcome.process_id.has_value());
    EG_REQUIRE(outcome.checkpoints_observed >= 3);
    const auto progress = scheduler.progress("child-ok");
    EG_REQUIRE(!progress.updates.empty());
    EG_REQUIRE_EQ(progress.updates[0].stage, std::string{"child-compute"});
    EG_REQUIRE(!progress.indeterminate);
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
}

EG_TEST(child_result_commits_through_the_gateway_only_once) {
    JobScheduler scheduler{SchedulerConfig{}};
    EG_REQUIRE(scheduler.submit(
                   child_request("child-commit", WorkerBehavior::cooperative, 2))
                   .accepted());
    const auto outcome = terminal_outcome(scheduler, "child-commit");
    EG_REQUIRE(outcome.succeeded());

    CommandGateway gateway{compute_registry(), target_document()};
    PublishedTracker published;
    gateway.subscribe([&published](const everygraph::application::DocumentEvent& event) {
        ++published.events;
        published.changes += event.changes.size();
    });
    const auto first =
        commit_worker_result(gateway, outcome, WorkerCommitContext{"child-commit-1"});
    EG_REQUIRE(first.succeeded());
    EG_REQUIRE(first.changed);
    EG_REQUIRE_EQ(first.events.size(), std::size_t{1});
    EG_REQUIRE_EQ(first.revisions.at(kTarget), std::uint64_t{1});
    EG_REQUIRE_EQ(published.events, std::size_t{1});
    EG_REQUIRE_EQ(computed_value(gateway.snapshot()).value_or(0), child_candidate(2));
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{1});

    const auto replay =
        commit_worker_result(gateway, outcome, WorkerCommitContext{"child-commit-2"});
    EG_REQUIRE(!replay.succeeded());
    EG_REQUIRE(replay.error.has_value());
    EG_REQUIRE(replay.error->code == ErrorCode::revision_conflict);
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{1});
}

EG_TEST(child_cancel_stops_at_a_checkpoint_without_termination) {
    JobScheduler scheduler{SchedulerConfig{}};
    JobRequest request = child_request("child-cancel", WorkerBehavior::cooperative, 200);
    request.deadline_milliseconds = 5000;
    EG_REQUIRE(scheduler.submit(request).accepted());
    wait_until_running(scheduler, "child-cancel");
    EG_REQUIRE(scheduler.request_cancellation("child-cancel"));

    const auto outcome = terminal_outcome(scheduler, "child-cancel");
    EG_REQUIRE_STATE(outcome, JobState::canceled);
    EG_REQUIRE(!outcome.escalated_termination);
    EG_REQUIRE(outcome.checkpoints_observed < 200);
    EG_REQUIRE(!outcome.result_value.has_value());
    EG_REQUIRE(outcome.failure_reason.has_value());
    EG_REQUIRE(*outcome.failure_reason == WorkerFailureReason::canceled);
    EG_REQUIRE(outcome.error.has_value());
    EG_REQUIRE(outcome.error->code == ErrorCode::canceled);
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
}

EG_TEST(child_ignoring_cancellation_is_terminated) {
    JobScheduler scheduler{SchedulerConfig{}};
    JobRequest request = child_request("child-stubborn", WorkerBehavior::ignores_cancel, 200);
    request.cancel_grace_milliseconds = 40;
    request.deadline_milliseconds = 5000;
    EG_REQUIRE(scheduler.submit(request).accepted());
    wait_until_running(scheduler, "child-stubborn");
    EG_REQUIRE(scheduler.request_cancellation("child-stubborn"));

    const auto outcome = terminal_outcome(scheduler, "child-stubborn");
    EG_REQUIRE_STATE(outcome, JobState::canceled);
    EG_REQUIRE(outcome.escalated_termination);
    EG_REQUIRE(!outcome.result_value.has_value());
    EG_REQUIRE(outcome.process_signal.has_value());
    EG_REQUIRE_EQ(*outcome.process_signal, SIGKILL);
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
}

EG_TEST(late_candidate_after_cancellation_is_not_committable) {
    JobScheduler scheduler{SchedulerConfig{}};
    // The worker waits for the cancel frame and answers it with a result, so the
    // host sees a candidate that is already not committable without any race.
    JobRequest request = child_request("child-late", WorkerBehavior::late_result, 3);
    request.cancel_grace_milliseconds = 5000;
    request.deadline_milliseconds = 5000;
    EG_REQUIRE(scheduler.submit(request).accepted());
    wait_until_running(scheduler, "child-late");
    EG_REQUIRE(scheduler.request_cancellation("child-late"));

    const auto outcome = terminal_outcome(scheduler, "child-late");
    EG_REQUIRE_STATE(outcome, JobState::canceled);
    EG_REQUIRE(!outcome.escalated_termination);
    EG_REQUIRE(!outcome.result_value.has_value());
    EG_REQUIRE(outcome.error.has_value());
    EG_REQUIRE(outcome.error->message.find("committable") != std::string::npos);

    CommandGateway gateway{compute_registry(), target_document()};
    std::size_t published = 0;
    gateway.subscribe([&published](const everygraph::application::DocumentEvent&) { ++published; });
    const auto sequence = gateway.last_sequence();
    const auto commit =
        commit_worker_result(gateway, outcome, WorkerCommitContext{"child-late-commit"});
    EG_REQUIRE(commit.status == CommandStatus::canceled);
    EG_REQUIRE(!commit.succeeded());
    EG_REQUIRE_EQ(commit.events.size(), std::size_t{0});
    EG_REQUIRE_EQ(gateway.last_sequence(), sequence);
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{0});
    EG_REQUIRE_EQ(published, std::size_t{0});
    EG_REQUIRE(!computed_value(gateway.snapshot()).has_value());
}

EG_TEST(child_deadline_overrun_is_terminated) {
    JobScheduler scheduler{SchedulerConfig{}};
    JobRequest request = child_request("child-timeout", WorkerBehavior::unresponsive, 200);
    request.deadline_milliseconds = 60;
    request.cancel_grace_milliseconds = 40;
    EG_REQUIRE(scheduler.submit(request).accepted());

    const auto outcome = terminal_outcome(scheduler, "child-timeout");
    EG_REQUIRE_STATE(outcome, JobState::failed);
    EG_REQUIRE(outcome.escalated_termination);
    EG_REQUIRE(outcome.failure_reason.has_value());
    EG_REQUIRE(*outcome.failure_reason == WorkerFailureReason::timed_out);
    EG_REQUIRE(outcome.error.has_value());
    EG_REQUIRE(outcome.error->code == ErrorCode::io_error);
    EG_REQUIRE(!outcome.result_value.has_value());
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
}

EG_TEST(child_crash_is_observed_as_a_signal_death) {
    JobScheduler scheduler{SchedulerConfig{}};
    const auto request = child_request("child-crash", WorkerBehavior::process_crash);
    EG_REQUIRE(scheduler.submit(request).accepted());
    const auto outcome = terminal_outcome(scheduler, "child-crash");
    EG_REQUIRE_STATE(outcome, JobState::failed);
    EG_REQUIRE(outcome.failure_reason.has_value());
    EG_REQUIRE(*outcome.failure_reason == WorkerFailureReason::process_crashed);
    EG_REQUIRE(outcome.process_signal.has_value());
    EG_REQUIRE_EQ(*outcome.process_signal, SIGABRT);
    EG_REQUIRE(!outcome.escalated_termination);
    EG_REQUIRE(outcome.error.has_value());
    EG_REQUIRE(outcome.error->code == ErrorCode::io_error);
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
}

EG_TEST(child_unexpected_exit_is_observed_with_its_code) {
    JobScheduler scheduler{SchedulerConfig{}};
    const auto request = child_request("child-exit", WorkerBehavior::process_exit);
    EG_REQUIRE(scheduler.submit(request).accepted());
    const auto outcome = terminal_outcome(scheduler, "child-exit");
    EG_REQUIRE_STATE(outcome, JobState::failed);
    EG_REQUIRE(outcome.failure_reason.has_value());
    EG_REQUIRE(*outcome.failure_reason == WorkerFailureReason::process_exit);
    EG_REQUIRE(outcome.process_exit_code.has_value());
    EG_REQUIRE_EQ(*outcome.process_exit_code, 77);
    EG_REQUIRE(outcome.error.has_value());
    EG_REQUIRE(outcome.error->message.find("77") != std::string::npos);
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
}

EG_TEST(child_malformed_frame_is_rejected) {
    JobScheduler scheduler{SchedulerConfig{}};
    const auto request = child_request("child-noise", WorkerBehavior::malformed_frame);
    EG_REQUIRE(scheduler.submit(request).accepted());
    const auto outcome = terminal_outcome(scheduler, "child-noise");
    EG_REQUIRE_STATE(outcome, JobState::failed);
    EG_REQUIRE(outcome.failure_reason.has_value());
    EG_REQUIRE(*outcome.failure_reason == WorkerFailureReason::protocol_violation);
    EG_REQUIRE(outcome.error.has_value());
    EG_REQUIRE(outcome.error->code == ErrorCode::invalid_request);
    EG_REQUIRE(!outcome.result_value.has_value());
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
}

EG_TEST(child_unsupported_schema_is_rejected) {
    JobScheduler scheduler{SchedulerConfig{}};
    const auto request = child_request("child-schema", WorkerBehavior::wrong_schema);
    EG_REQUIRE(scheduler.submit(request).accepted());
    const auto outcome = terminal_outcome(scheduler, "child-schema");
    EG_REQUIRE_STATE(outcome, JobState::failed);
    EG_REQUIRE(outcome.failure_reason.has_value());
    EG_REQUIRE(*outcome.failure_reason == WorkerFailureReason::unsupported_schema);
    EG_REQUIRE(outcome.error.has_value());
    EG_REQUIRE(outcome.error->code == ErrorCode::unsupported_schema_version);
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
}

EG_TEST(child_frame_with_a_foreign_identity_is_rejected) {
    JobScheduler scheduler{SchedulerConfig{}};
    const auto request = child_request("child-foreign", WorkerBehavior::wrong_job_identity);
    EG_REQUIRE(scheduler.submit(request).accepted());
    const auto outcome = terminal_outcome(scheduler, "child-foreign");
    EG_REQUIRE_STATE(outcome, JobState::failed);
    EG_REQUIRE(outcome.failure_reason.has_value());
    EG_REQUIRE(*outcome.failure_reason == WorkerFailureReason::protocol_violation);
    EG_REQUIRE(outcome.error.has_value());
    EG_REQUIRE(outcome.error->code == ErrorCode::invalid_request);
    EG_REQUIRE(outcome.error->message.find("task identity") != std::string::npos);
    EG_REQUIRE(!outcome.result_value.has_value());
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
}

EG_TEST(child_echoing_a_host_frame_is_rejected) {
    JobScheduler scheduler{SchedulerConfig{}};
    const auto request = child_request("child-echo", WorkerBehavior::echoes_host_frame);
    EG_REQUIRE(scheduler.submit(request).accepted());
    const auto outcome = terminal_outcome(scheduler, "child-echo");
    EG_REQUIRE_STATE(outcome, JobState::failed);
    EG_REQUIRE(outcome.failure_reason.has_value());
    EG_REQUIRE(*outcome.failure_reason == WorkerFailureReason::protocol_violation);
    EG_REQUIRE(outcome.error.has_value());
    EG_REQUIRE(outcome.error->message.find("host-side frame") != std::string::npos);
    EG_REQUIRE(!outcome.result_value.has_value());
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
}

EG_TEST(child_progress_beyond_its_total_is_not_trusted) {
    JobScheduler scheduler{SchedulerConfig{}};
    const auto request = child_request("child-overrun", WorkerBehavior::progress_overrun, 4);
    EG_REQUIRE(scheduler.submit(request).accepted());
    const auto outcome = terminal_outcome(scheduler, "child-overrun");
    EG_REQUIRE_STATE(outcome, JobState::failed);
    EG_REQUIRE(outcome.failure_reason.has_value());
    EG_REQUIRE(*outcome.failure_reason == WorkerFailureReason::protocol_violation);
    EG_REQUIRE(outcome.error.has_value());
    EG_REQUIRE(outcome.error->message.find("reports") != std::string::npos);
    // The dishonest update never reaches the progress stream, so no percentage
    // built on it can appear in a display.
    EG_REQUIRE_EQ(outcome.checkpoints_observed, std::int64_t{0});
    EG_REQUIRE(scheduler.progress("child-overrun").updates.empty());
    EG_REQUIRE(!outcome.result_value.has_value());
    EG_REQUIRE_EQ(scheduler.reservation_count(), std::size_t{0});
}

EG_TEST(child_numerical_failure_maps_to_a_structured_error) {
    JobScheduler scheduler{SchedulerConfig{}};
    const auto request = child_request("child-numeric", WorkerBehavior::numerical_failure, 4);
    EG_REQUIRE(scheduler.submit(request).accepted());
    const auto outcome = terminal_outcome(scheduler, "child-numeric");
    EG_REQUIRE_STATE(outcome, JobState::failed);
    EG_REQUIRE(outcome.failure_reason.has_value());
    EG_REQUIRE(*outcome.failure_reason == WorkerFailureReason::numerical);
    EG_REQUIRE(outcome.error.has_value());
    EG_REQUIRE(outcome.error->code == ErrorCode::numerical_failure);
    EG_REQUIRE(outcome.error->message.find("fake kernel diverged") != std::string::npos);
}

EG_TEST(kernel_publishes_nothing_for_a_non_succeeded_candidate) {
    CommandGateway gateway{compute_registry(), target_document()};
    PublishedTracker published;
    gateway.subscribe([&published](const everygraph::application::DocumentEvent& event) {
        ++published.events;
        published.changes += event.changes.size();
    });
    const auto sequence = gateway.last_sequence();
    const auto snapshot = gateway.snapshot();

    JobOutcome canceled;
    canceled.state = JobState::canceled;
    canceled.job_id = "gate-cancel";
    const auto canceled_commit =
        commit_worker_result(gateway, canceled, WorkerCommitContext{"gate-1"});
    EG_REQUIRE(canceled_commit.status == CommandStatus::canceled);
    EG_REQUIRE(canceled_commit.error.has_value());
    EG_REQUIRE(canceled_commit.error->code == ErrorCode::canceled);

    JobOutcome timed_out;
    timed_out.state = JobState::failed;
    timed_out.job_id = "gate-timeout";
    timed_out.failure_reason = WorkerFailureReason::timed_out;
    timed_out.error = Error{ErrorCode::io_error, "worker passed its deadline"};
    timed_out.result_value = 999;
    EG_REQUIRE(commit_worker_result(gateway, timed_out, WorkerCommitContext{"gate-2"}).status ==
               CommandStatus::failed);

    JobOutcome crashed;
    crashed.state = JobState::failed;
    crashed.job_id = "gate-crash";
    crashed.failure_reason = WorkerFailureReason::process_crashed;
    EG_REQUIRE(commit_worker_result(gateway, crashed, WorkerCommitContext{"gate-3"}).status ==
               CommandStatus::failed);

    JobOutcome superseded;
    superseded.state = JobState::superseded;
    superseded.job_id = "gate-superseded";
    const auto superseded_commit =
        commit_worker_result(gateway, superseded, WorkerCommitContext{"gate-4"});
    EG_REQUIRE(superseded_commit.status == CommandStatus::canceled);
    EG_REQUIRE(superseded_commit.error.has_value());
    EG_REQUIRE(superseded_commit.error->code == ErrorCode::canceled);

    JobOutcome empty_candidate;
    empty_candidate.state = JobState::succeeded;
    empty_candidate.job_id = "gate-empty";
    const auto empty_commit =
        commit_worker_result(gateway, empty_candidate, WorkerCommitContext{"gate-5"});
    EG_REQUIRE(!empty_commit.succeeded());
    EG_REQUIRE(empty_commit.error.has_value());
    EG_REQUIRE(empty_commit.error->code == ErrorCode::internal_error);

    EG_REQUIRE_EQ(published.events, std::size_t{0});
    EG_REQUIRE_EQ(published.changes, std::size_t{0});
    EG_REQUIRE_EQ(gateway.last_sequence(), sequence);
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{0});
    EG_REQUIRE_EQ(gateway.snapshot().objects, snapshot.objects);
    EG_REQUIRE(!computed_value(gateway.snapshot()).has_value());
}

EG_TEST(stale_candidate_after_a_landing_edit_conflicts) {
    JobScheduler scheduler{SchedulerConfig{}};
    EG_REQUIRE(scheduler.submit(in_process_request("stale-1", 2)).accepted());
    const auto outcome = terminal_outcome(scheduler, "stale-1");
    EG_REQUIRE(outcome.succeeded());

    CommandGateway gateway{compute_registry(), target_document()};
    std::size_t published = 0;
    gateway.subscribe([&published](const everygraph::application::DocumentEvent&) { ++published; });
    const auto applied = gateway.execute(manual_edit_request("edit-1", 42));
    EG_REQUIRE(applied.succeeded());
    EG_REQUIRE_EQ(revision_of(gateway.snapshot()), std::uint64_t{1});
    const auto sequence = gateway.last_sequence();
    const auto undo_depth = gateway.undo_depth();

    const auto stale = commit_worker_result(gateway, outcome, WorkerCommitContext{"stale-commit"});
    EG_REQUIRE(!stale.succeeded());
    EG_REQUIRE(stale.error.has_value());
    EG_REQUIRE(stale.error->code == ErrorCode::revision_conflict);
    EG_REQUIRE_EQ(stale.events.size(), std::size_t{0});
    EG_REQUIRE_EQ(gateway.last_sequence(), sequence);
    EG_REQUIRE_EQ(gateway.undo_depth(), undo_depth);
    EG_REQUIRE_EQ(published, std::size_t{1});
    EG_REQUIRE_EQ(computed_value(gateway.snapshot()).value_or(0), std::int64_t{42});

    JobRequest refreshed = in_process_request("stale-2", 2);
    refreshed.base_revision = revision_of(gateway.snapshot());
    EG_REQUIRE(scheduler.submit(refreshed).accepted());
    const auto second = terminal_outcome(scheduler, "stale-2");
    const auto committed =
        commit_worker_result(gateway, second, WorkerCommitContext{"stale-commit-2"});
    EG_REQUIRE(committed.succeeded());
    EG_REQUIRE_EQ(computed_value(gateway.snapshot()).value_or(0), in_process_candidate(2));
    EG_REQUIRE_EQ(revision_of(gateway.snapshot()), std::uint64_t{2});
}

}  // namespace

int main() {
    // SIGPIPE is left at its default disposition on purpose. The scheduler blocks
    // it on the worker threads that own the pipes, so a broken pipe has to reach
    // the transport as an EPIPE; if that guarantee ever regressed, this whole
    // suite would die on the signal instead of passing on the host's goodwill.
    const auto& cases = test_cases();
    std::size_t failures = 0;
    for (const auto& test_case : cases) {
        try {
            test_case.function();
            std::cout << "[PASS] " << test_case.name << '\n';
        } catch (const std::exception& exception) {
            ++failures;
            std::cerr << "[FAIL] " << test_case.name << ": " << exception.what() << '\n';
        } catch (...) {
            ++failures;
            std::cerr << "[FAIL] " << test_case.name << ": unknown exception\n";
        }
    }
    std::cout << (cases.size() - failures) << '/' << cases.size()
              << " worker-handshake tests passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
