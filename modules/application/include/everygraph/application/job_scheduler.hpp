#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <stop_token>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "everygraph/application/command.hpp"
#include "everygraph/application/worker_protocol.hpp"
#include "everygraph/foundation/error.hpp"
#include "everygraph/foundation/ids.hpp"

namespace everygraph::application {

class CommandGateway;

enum class WorkerTransport {
    in_process,
    child_process,
};

// The injected fault the prototype worker exhibits. Every mode is a fake bounded
// job; none of them runs Python, a plugin, an AI provider or a numerical kernel.
enum class WorkerBehavior {
    cooperative,
    numerical_failure,
    unresponsive,
    process_crash,
    process_exit,
    ignores_cancel,
    malformed_frame,
    wrong_schema,
    // Waits for the cancel frame and answers it with a result anyway, so the
    // "candidate after cancellation" rule is tested without a timing margin.
    late_result,
    wrong_job_identity,
    echoes_host_frame,
    // Reports more work done than the total it claimed, so an out-of-range
    // progress number is caught before it reaches a display.
    progress_overrun,
};

enum class JobState {
    queued,
    running,
    succeeded,
    failed,
    canceled,
    superseded,
};

[[nodiscard]] const char* to_string(JobState state) noexcept;
[[nodiscard]] const char* to_string(WorkerBehavior behavior) noexcept;
[[nodiscard]] std::optional<WorkerBehavior> worker_behavior_from_string(
    std::string_view name) noexcept;
[[nodiscard]] const char* to_string(WorkerTransport transport) noexcept;

struct SchedulerConfig {
    std::size_t worker_threads{2};
    std::size_t max_pending_jobs{4};
    std::uint64_t memory_budget_bytes{1048576};
    std::size_t max_progress_updates_per_job{8};
    std::int64_t cancel_grace_milliseconds{250};
};

struct JobRequest {
    std::string job_id;
    std::string command_id{"demo.compute_value"};
    std::uint32_t command_schema_version{1};
    foundation::ObjectId target_object{1};
    foundation::Revision base_revision{foundation::kInitialRevision};
    std::uint64_t estimated_bytes{1024};
    std::int64_t work_units{4};
    std::int64_t checkpoint_every_units{1};
    bool total_is_unknown{false};
    WorkerTransport transport{WorkerTransport::in_process};
    WorkerBehavior behavior{WorkerBehavior::cooperative};
    std::int64_t deadline_milliseconds{2000};
    std::int64_t cancel_grace_milliseconds{-1};
    // Executable that speaks the worker line protocol on stdin and stdout. The
    // scheduler blocks SIGPIPE on the worker thread that owns the pipes and
    // restores the default signal dispositions in the child, so a closed pipe is
    // an EPIPE the transport handles rather than a signal that kills the host.
    // The child's stderr is discarded: a real worker transport needs its own
    // diagnostic channel before worker crash logs can be read.
    std::string child_program;
};

struct JobOutcome {
    JobState state{JobState::queued};
    std::string job_id;
    std::string command_id;
    std::uint32_t command_schema_version{1};
    foundation::ObjectId target_object{1};
    foundation::Revision base_revision{foundation::kInitialRevision};
    std::optional<WorkerFailureReason> failure_reason;
    std::optional<foundation::Error> error;
    std::optional<std::int64_t> result_value;
    std::optional<std::uint64_t> process_id;
    std::optional<int> process_signal;
    std::optional<int> process_exit_code;
    std::int64_t checkpoints_observed{0};
    bool cancel_requested{false};
    // True only when the host had to kill the worker because its stop frame went
    // unanswered. A kill used to tear down a worker that violated the protocol is
    // not an escalation of a stop request, so it reports through process_signal
    // and leaves this false.
    bool escalated_termination{false};

    [[nodiscard]] bool succeeded() const noexcept { return state == JobState::succeeded; }
    [[nodiscard]] bool terminal() const noexcept {
        return state != JobState::queued && state != JobState::running;
    }
};

enum class SubmitDisposition {
    accepted,
    duplicate_rejected,
    superseded_previous,
    rejected,
};

struct SubmitResult {
    SubmitDisposition disposition{SubmitDisposition::rejected};
    std::optional<foundation::Error> error;

    [[nodiscard]] bool accepted() const noexcept {
        return disposition == SubmitDisposition::accepted ||
               disposition == SubmitDisposition::superseded_previous;
    }
};

struct ProgressRecord {
    std::string stage;
    std::int64_t completed_units{0};
    std::optional<std::int64_t> total_units;

    [[nodiscard]] bool indeterminate() const noexcept { return !total_units.has_value(); }
};

struct ProgressState {
    std::vector<ProgressRecord> updates;
    std::size_t coalesced_count{0};
    bool indeterminate{false};
};

// Bounded, stage-coalescing buffer: a chatty worker cannot grow the host's
// progress queue without limit, and the newest value per stage survives.
class ProgressAggregator {
public:
    explicit ProgressAggregator(std::size_t capacity);

    void push(ProgressRecord record);
    [[nodiscard]] std::vector<ProgressRecord> snapshot() const;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] std::size_t capacity() const noexcept;
    [[nodiscard]] std::size_t coalesced_count() const noexcept;

private:
    std::size_t capacity_{1};
    std::vector<ProgressRecord> records_;
    std::size_t coalesced_{0};
};

// Reservation is taken before execution and released on any terminal state,
// including cancellation, worker death and supersession. The key is one
// submission, not one task identity, so a superseded submission and the
// submission that replaces it never share a ledger entry.
class AdmissionLedger {
public:
    explicit AdmissionLedger(std::uint64_t budget_bytes);

    // headroom_bytes is the reservation a superseded incumbent still holds and
    // is about to release: a replacement is admitted against the incumbent's
    // bytes, but only after the reservation succeeds, so a rejected replacement
    // never stops the work already in flight.
    [[nodiscard]] std::optional<foundation::Error> reserve(const std::string& key,
                                                           std::uint64_t bytes,
                                                           std::uint64_t headroom_bytes = 0);
    void release(const std::string& key);
    [[nodiscard]] std::uint64_t available_bytes() const noexcept;
    [[nodiscard]] std::uint64_t committed_bytes() const noexcept;
    [[nodiscard]] std::size_t reservation_count() const noexcept;
    [[nodiscard]] bool holds(const std::string& key) const;
    // Bytes a held key contributes to a replacement's headroom; zero when the
    // key is not reserved.
    [[nodiscard]] std::uint64_t reservation_bytes(const std::string& key) const noexcept;

private:
    std::uint64_t budget_bytes_;
    std::uint64_t committed_bytes_{0};
    std::vector<std::pair<std::string, std::uint64_t>> reservations_;
};

// Bounded queue with admission, progress and stop semantics. A job record stays
// known after it ends and nothing is pruned, so this transport is a lifetime-
// bounded prototype rather than a long-running scheduler. The child-process
// transport is POSIX-only crash containment and never a sandbox.
class JobScheduler {
public:
    explicit JobScheduler(SchedulerConfig config = {});
    JobScheduler(const JobScheduler&) = delete;
    JobScheduler& operator=(const JobScheduler&) = delete;
    ~JobScheduler();

    // One submission per task identity is live at a time. A different payload
    // under a known identity supersedes the incumbent; resubmitting a *different*
    // payload under an identity whose incumbent is already terminal is plain
    // `accepted` and replaces that record, while an identical payload stays
    // `duplicate_rejected` because reservation keys are never pruned. Either way
    // the replacement still passes the same queue and ledger admission bounds as a
    // first submission. A candidate must be committed before its identity is
    // reused.
    [[nodiscard]] SubmitResult submit(const JobRequest& request);
    [[nodiscard]] bool request_cancellation(const std::string& job_id);
    // True when the job reached a terminal state, false on timeout or when the
    // identity is unknown. A shutdown also ends the wait, and the caller then
    // reads the record's own outcome rather than treating the wake as success.
    [[nodiscard]] bool await_terminal(const std::string& job_id, std::chrono::milliseconds timeout);
    [[nodiscard]] std::optional<JobOutcome> outcome(const std::string& job_id) const;
    // A superseded submission keeps its own terminal record; the task identity
    // in the scheduler now belongs to the newer submission. A child job's record
    // also carries how that worker left, which is observed only after the
    // supersession itself was decided.
    [[nodiscard]] std::optional<JobOutcome> superseded_outcome(const std::string& job_id) const;
    [[nodiscard]] ProgressState progress(const std::string& job_id) const;
    [[nodiscard]] std::size_t queue_depth() const;
    [[nodiscard]] std::size_t known_job_count() const;
    // Read through the scheduler's own lock: the ledger is shared with the
    // worker threads, so no reference to it escapes.
    [[nodiscard]] std::uint64_t available_bytes() const;
    [[nodiscard]] std::uint64_t committed_bytes() const;
    [[nodiscard]] std::size_t reservation_count() const;

    [[nodiscard]] const SchedulerConfig& config() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

struct WorkerCommitContext {
    std::string request_id;
    std::string principal{"worker-proto"};
    std::string project_id{"project-1"};
};

// Only the application executor commits, and only a succeeded candidate reaches
// the gateway. Any other terminal state returns a failed outcome without a
// gateway call, so no revision and no event can appear.
[[nodiscard]] CommandOutcome commit_worker_result(CommandGateway& gateway,
                                                 const JobOutcome& outcome,
                                                 const WorkerCommitContext& context,
                                                 std::stop_token stop_token = {});

}  // namespace everygraph::application
