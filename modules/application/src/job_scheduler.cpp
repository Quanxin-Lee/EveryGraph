#include "everygraph/application/job_scheduler.hpp"

#include <algorithm>
#include <condition_variable>
#include <csignal>
#include <deque>
#include <map>
#include <mutex>
#include <pthread.h>
#include <thread>
#include <utility>

#include "child_process.hpp"
#include "everygraph/application/command_gateway.hpp"

namespace everygraph::application {
namespace {

constexpr std::chrono::milliseconds kCheckpointSlice{2};
constexpr std::chrono::milliseconds kReaderSlice{10};
constexpr std::chrono::milliseconds kChildReapGrace{200};
constexpr const char* kInProcessStage = "compute";

[[nodiscard]] std::int64_t candidate_value(const JobRequest& request) {
    return request.work_units * 3 + static_cast<std::int64_t>(request.target_object.value);
}

[[nodiscard]] WorkerEnvelope make_submit_frame(const JobRequest& request) {
    WorkerEnvelope frame;
    frame.kind = WorkerMessageKind::submit;
    frame.job_id = request.job_id;
    frame.command_id = request.command_id;
    frame.job_mode = to_string(request.behavior);
    frame.base_revision = request.base_revision;
    frame.reservation_bytes = request.estimated_bytes;
    frame.deadline_milliseconds = request.deadline_milliseconds;
    frame.work_units = request.work_units;
    frame.checkpoint_every_units = request.checkpoint_every_units;
    frame.total_is_unknown = request.total_is_unknown;
    return frame;
}

[[nodiscard]] WorkerEnvelope make_progress_frame(const JobRequest& request, const char* stage,
                                                std::int64_t completed) {
    WorkerEnvelope frame;
    frame.kind = WorkerMessageKind::progress;
    frame.job_id = request.job_id;
    WorkerProgress progress;
    progress.stage = stage;
    progress.completed_units = completed;
    if (!request.total_is_unknown) {
        progress.total_units = request.work_units;
    }
    frame.progress = progress;
    return frame;
}

[[nodiscard]] WorkerEnvelope make_control_frame(const JobRequest& request, WorkerMessageKind kind) {
    WorkerEnvelope frame;
    frame.kind = kind;
    frame.job_id = request.job_id;
    return frame;
}

[[nodiscard]] std::chrono::milliseconds remaining_of(std::chrono::steady_clock::time_point started,
                                                    std::int64_t deadline_milliseconds) {
    const auto spent = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - started);
    if (deadline_milliseconds <= 0 || spent.count() >= deadline_milliseconds) {
        return std::chrono::milliseconds{0};
    }
    return std::chrono::milliseconds{deadline_milliseconds - spent.count()};
}

// How the host stopped a worker: it left on its own after the stop frame, or it
// ignored the stop and was killed at the end of the grace window. Named because
// a bare flag in this argument list would not say which of the two it means.
enum class WorkerStop {
    cooperative,
    killed,
};

struct ChildObservation {
    std::optional<std::uint64_t> process_id;
    std::optional<int> signal;
    std::optional<int> exit_code;
};

struct LiveFlags {
    bool finalized{false};
    bool cancel_requested{false};
    bool supersede_requested{false};
};

[[nodiscard]] ChildObservation observation_of(const ChildProcess& child,
                                              const ChildProcess::ExitStatus& status) {
    ChildObservation observation;
    observation.process_id = child.process_id();
    if (status.signalled) {
        observation.signal = status.signal;
    }
    if (status.exited && !status.signalled) {
        observation.exit_code = status.exit_code;
    }
    return observation;
}

[[nodiscard]] WorkerFailureReason classify_exit(const ChildProcess::ExitStatus& status) {
    if (status.signalled) {
        return WorkerFailureReason::process_crashed;
    }
    if (status.exited && status.exit_code != 0) {
        return WorkerFailureReason::process_exit;
    }
    return WorkerFailureReason::protocol_violation;
}

[[nodiscard]] std::string describe_exit(const ChildProcess::ExitStatus& status) {
    if (status.signalled) {
        return "worker process died with signal " + std::to_string(status.signal);
    }
    if (status.exited && status.exit_code != 0) {
        return "worker process exited with code " + std::to_string(status.exit_code);
    }
    return "worker process ended without a terminal frame";
}

[[nodiscard]] foundation::ErrorCode error_code_for(WorkerFailureReason reason) noexcept {
    switch (reason) {
    case WorkerFailureReason::timed_out:
    case WorkerFailureReason::process_crashed:
    case WorkerFailureReason::process_exit:
        return foundation::ErrorCode::io_error;
    case WorkerFailureReason::protocol_violation:
    case WorkerFailureReason::unsupported_schema:
        return foundation::ErrorCode::invalid_request;
    case WorkerFailureReason::canceled:
    case WorkerFailureReason::superseded:
        return foundation::ErrorCode::canceled;
    case WorkerFailureReason::stale_input:
        return foundation::ErrorCode::revision_conflict;
    case WorkerFailureReason::numerical:
        return foundation::ErrorCode::numerical_failure;
    case WorkerFailureReason::resource_exhausted:
        return foundation::ErrorCode::resource_exhausted;
    case WorkerFailureReason::none:
        break;
    }
    return foundation::ErrorCode::internal_error;
}

[[nodiscard]] std::chrono::milliseconds min_of(std::chrono::milliseconds left,
                                               std::chrono::milliseconds right) {
    return left < right ? left : right;
}

// A worker thread owns the pipes to a child process, so SIGPIPE is blocked on a
// thread the scheduler created and never unblocks: a worker that dies mid-write
// becomes an EPIPE the transport handles instead of a signal that terminates the
// host. The child gets the default dispositions back when it is spawned.
void block_sigpipe_on_this_thread() {
    sigset_t mask;
    // sigemptyset and sigaddset are macros on this platform, so they take no
    // qualification.
    sigemptyset(&mask);
    sigaddset(&mask, SIGPIPE);
    (void)::pthread_sigmask(SIG_BLOCK, &mask, nullptr);
}

}  // namespace

const char* to_string(JobState state) noexcept {
    switch (state) {
    case JobState::queued:
        return "queued";
    case JobState::running:
        return "running";
    case JobState::succeeded:
        return "succeeded";
    case JobState::failed:
        return "failed";
    case JobState::canceled:
        return "canceled";
    case JobState::superseded:
        return "superseded";
    }
    return "unknown";
}

const char* to_string(WorkerBehavior behavior) noexcept {
    switch (behavior) {
    case WorkerBehavior::cooperative:
        return "cooperative";
    case WorkerBehavior::numerical_failure:
        return "numerical_failure";
    case WorkerBehavior::unresponsive:
        return "unresponsive";
    case WorkerBehavior::process_crash:
        return "process_crash";
    case WorkerBehavior::process_exit:
        return "process_exit";
    case WorkerBehavior::ignores_cancel:
        return "ignores_cancel";
    case WorkerBehavior::malformed_frame:
        return "malformed_frame";
    case WorkerBehavior::wrong_schema:
        return "wrong_schema";
    case WorkerBehavior::late_result:
        return "late_result";
    case WorkerBehavior::wrong_job_identity:
        return "wrong_job_identity";
    case WorkerBehavior::echoes_host_frame:
        return "echoes_host_frame";
    case WorkerBehavior::progress_overrun:
        return "progress_overrun";
    }
    return "unknown";
}

const char* to_string(WorkerTransport transport) noexcept {
    switch (transport) {
    case WorkerTransport::in_process:
        return "in_process";
    case WorkerTransport::child_process:
        return "child_process";
    }
    return "unknown";
}

std::optional<WorkerBehavior> worker_behavior_from_string(std::string_view name) noexcept {
    for (const auto behavior : {WorkerBehavior::cooperative, WorkerBehavior::numerical_failure,
                                WorkerBehavior::unresponsive, WorkerBehavior::process_crash,
                                WorkerBehavior::process_exit, WorkerBehavior::ignores_cancel,
                                WorkerBehavior::malformed_frame, WorkerBehavior::wrong_schema,
                                WorkerBehavior::late_result, WorkerBehavior::wrong_job_identity,
                                WorkerBehavior::echoes_host_frame,
                                WorkerBehavior::progress_overrun}) {
        if (name == to_string(behavior)) {
            return behavior;
        }
    }
    return std::nullopt;
}

ProgressAggregator::ProgressAggregator(std::size_t capacity)
    : capacity_(capacity == 0 ? 1 : capacity) {}

void ProgressAggregator::push(ProgressRecord record) {
    for (auto& existing : records_) {
        if (existing.stage == record.stage) {
            existing = std::move(record);
            ++coalesced_;
            return;
        }
    }
    records_.push_back(std::move(record));
    if (records_.size() > capacity_) {
        records_.erase(records_.begin());
        ++coalesced_;
    }
}

std::vector<ProgressRecord> ProgressAggregator::snapshot() const { return records_; }

std::size_t ProgressAggregator::size() const noexcept { return records_.size(); }

std::size_t ProgressAggregator::capacity() const noexcept { return capacity_; }

std::size_t ProgressAggregator::coalesced_count() const noexcept { return coalesced_; }

AdmissionLedger::AdmissionLedger(std::uint64_t budget_bytes) : budget_bytes_(budget_bytes) {}

std::optional<foundation::Error> AdmissionLedger::reserve(const std::string& key,
                                                         std::uint64_t bytes,
                                                         std::uint64_t headroom_bytes) {
    for (const auto& reservation : reservations_) {
        if (reservation.first == key) {
            return foundation::Error{foundation::ErrorCode::already_exists,
                                     "reservation key is already held"};
        }
    }
    if (bytes > budget_bytes_ || bytes > available_bytes() + headroom_bytes) {
        return foundation::Error{
            foundation::ErrorCode::resource_exhausted,
            "job needs " + std::to_string(bytes) + " bytes, " + std::to_string(available_bytes()) +
                " of " + std::to_string(budget_bytes_) + " are available (" +
                std::to_string(headroom_bytes) +
                " more bytes are released as the incumbent stops)"};
    }
    reservations_.emplace_back(key, bytes);
    committed_bytes_ += bytes;
    return std::nullopt;
}

void AdmissionLedger::release(const std::string& key) {
    const auto found =
        std::find_if(reservations_.begin(), reservations_.end(),
                     [&key](const auto& reservation) { return reservation.first == key; });
    if (found == reservations_.end()) {
        return;
    }
    committed_bytes_ -= found->second;
    reservations_.erase(found);
}

std::uint64_t AdmissionLedger::available_bytes() const noexcept {
    // A replacement is admitted while the incumbent it supersedes still holds
    // its own reservation, so the committed total can pass the budget for as
    // long as the incumbent has not been released. Saturate instead of wrapping.
    return committed_bytes_ >= budget_bytes_ ? 0 : budget_bytes_ - committed_bytes_;
}

std::uint64_t AdmissionLedger::committed_bytes() const noexcept { return committed_bytes_; }

std::size_t AdmissionLedger::reservation_count() const noexcept { return reservations_.size(); }

bool AdmissionLedger::holds(const std::string& key) const {
    for (const auto& reservation : reservations_) {
        if (reservation.first == key) {
            return true;
        }
    }
    return false;
}

std::uint64_t AdmissionLedger::reservation_bytes(const std::string& key) const noexcept {
    for (const auto& reservation : reservations_) {
        if (reservation.first == key) {
            return reservation.second;
        }
    }
    return 0;
}

struct JobRecord {
    JobRecord(JobRequest job_request, std::string reservation, std::size_t progress_capacity)
        : request(std::move(job_request)),
          reservation_key(std::move(reservation)),
          progress(progress_capacity) {}

    JobRequest request;
    std::string reservation_key;
    std::stop_source stop_source;
    JobState state{JobState::queued};
    bool cancel_requested{false};
    bool supersede_requested{false};
    bool finalized{false};
    std::int64_t checkpoints{0};
    ProgressAggregator progress;
    std::optional<JobOutcome> outcome;
};

// The record a superseded terminal record came from, kept so a late process
// observation can be attached to the right one when the same identity has been
// superseded more than once.
struct SupersededRecord {
    std::weak_ptr<JobRecord> source;
    JobOutcome outcome;
};

class JobScheduler::Impl {
public:
    explicit Impl(SchedulerConfig job_config)
        : config_(job_config), ledger_(job_config.memory_budget_bytes) {
        if (config_.worker_threads == 0) {
            config_.worker_threads = 1;
        }
        for (std::size_t index = 0; index < config_.worker_threads; ++index) {
            workers_.emplace_back([this] { worker_loop(); });
        }
    }

    ~Impl() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            shutdown_ = true;
            for (auto& [job_id, record] : records_) {
                (void)job_id;
                if (!record->finalized) {
                    record->cancel_requested = true;
                    record->stop_source.request_stop();
                }
            }
        }
        queue_changed_.notify_all();
        terminal_changed_.notify_all();
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& [job_id, record] : records_) {
            (void)job_id;
            if (!record->finalized) {
                finalize_locked(record, JobState::canceled, WorkerFailureReason::canceled,
                                foundation::Error{foundation::ErrorCode::canceled,
                                                  "scheduler shut down before the job finished"},
                                std::nullopt, std::nullopt, WorkerStop::cooperative);
            }
        }
    }

    [[nodiscard]] SubmitResult submit(const JobRequest& request);
    [[nodiscard]] bool request_cancellation(const std::string& job_id);
    [[nodiscard]] bool await_terminal(const std::string& job_id, std::chrono::milliseconds timeout);
    [[nodiscard]] std::optional<JobOutcome> outcome(const std::string& job_id) const;
    [[nodiscard]] std::optional<JobOutcome> superseded_outcome(const std::string& job_id) const;
    [[nodiscard]] ProgressState progress(const std::string& job_id) const;
    [[nodiscard]] std::size_t queue_depth() const;
    [[nodiscard]] std::size_t known_job_count() const;
    [[nodiscard]] std::uint64_t available_bytes() const;
    [[nodiscard]] std::uint64_t committed_bytes() const;
    [[nodiscard]] std::size_t reservation_count() const;
    const SchedulerConfig& config() const noexcept { return config_; }

private:
    [[nodiscard]] static std::string fingerprint_of(const JobRequest& request);
    void worker_loop();
    void run_job(const std::shared_ptr<JobRecord>& record);
    void run_in_process(const std::shared_ptr<JobRecord>& record);
    void run_child(const std::shared_ptr<JobRecord>& record);
    LiveFlags live_flags(const std::shared_ptr<JobRecord>& record) const;
    bool finalize_locked(const std::shared_ptr<JobRecord>& record, JobState state,
                         std::optional<WorkerFailureReason> reason,
                         std::optional<foundation::Error> error,
                         std::optional<std::int64_t> result_value,
                         const std::optional<ChildObservation>& observation, WorkerStop stop);
    void annotate_locked(const std::shared_ptr<JobRecord>& record,
                         const ChildObservation& observation, WorkerStop stop);
    bool record_progress(const std::shared_ptr<JobRecord>& record, const WorkerEnvelope& frame);
    bool drop_from_pending_locked(const std::string& job_id);
    [[nodiscard]] bool is_pending_locked(const std::string& job_id) const;

    SchedulerConfig config_;
    AdmissionLedger ledger_;
    mutable std::mutex mutex_;
    std::condition_variable queue_changed_;
    std::condition_variable terminal_changed_;
    std::deque<std::string> pending_;
    std::map<std::string, std::shared_ptr<JobRecord>> records_;
    std::map<std::string, SupersededRecord> superseded_records_;
    std::vector<std::thread> workers_;
    bool shutdown_{false};
};

std::string JobScheduler::Impl::fingerprint_of(const JobRequest& request) {
    const auto frame = make_submit_frame(request);
    std::string key = frame.encode();
    key += ";transport=" + std::string{to_string(request.transport)};
    key += ";object=" + foundation::to_string(request.target_object);
    key += ";command_schema=" + std::to_string(request.command_schema_version);
    return key;
}

SubmitResult JobScheduler::Impl::submit(const JobRequest& request) {
    if (request.job_id.empty()) {
        return SubmitResult{SubmitDisposition::rejected,
                           foundation::Error{foundation::ErrorCode::invalid_request,
                                             "a worker job needs a task identity"}};
    }
    if (request.work_units <= 0 || request.checkpoint_every_units <= 0) {
        return SubmitResult{
            SubmitDisposition::rejected,
            foundation::Error{foundation::ErrorCode::invalid_request,
                              "work units and checkpoint interval must be positive"}};
    }
    if (request.deadline_milliseconds <= 0) {
        return SubmitResult{SubmitDisposition::rejected,
                           foundation::Error{foundation::ErrorCode::invalid_request,
                                             "a worker job needs a positive deadline"}};
    }
    if (request.transport == WorkerTransport::child_process && request.child_program.empty()) {
        return SubmitResult{SubmitDisposition::rejected,
                           foundation::Error{foundation::ErrorCode::invalid_request,
                                             "a child-process job needs a worker program path"}};
    }
    if (request.transport == WorkerTransport::in_process &&
        request.behavior != WorkerBehavior::cooperative &&
        request.behavior != WorkerBehavior::numerical_failure) {
        return SubmitResult{
            SubmitDisposition::rejected,
            foundation::Error{
                foundation::ErrorCode::unsupported_feature,
                "in-process jobs model cooperation and numerical failure only; "
                "process faults need the child transport"}};
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (shutdown_) {
        return SubmitResult{SubmitDisposition::rejected,
                           foundation::Error{foundation::ErrorCode::internal_error,
                                             "scheduler is shutting down"}};
    }

    const auto fingerprint = fingerprint_of(request);
    const auto existing = records_.find(request.job_id);
    std::uint64_t incumbent_bytes = 0;
    if (existing != records_.end()) {
        if (existing->second->reservation_key == fingerprint) {
            return SubmitResult{SubmitDisposition::duplicate_rejected,
                               foundation::Error{foundation::ErrorCode::already_exists,
                                                 "an identical submission is already known for " +
                                                     request.job_id}};
        }
        // A replacement takes over the incumbent's queue slot, so the queue bound
        // only stops an identity that has never been submitted.
        const bool reuses_slot = is_pending_locked(request.job_id);
        if (!reuses_slot && pending_.size() >= config_.max_pending_jobs) {
            return SubmitResult{SubmitDisposition::rejected,
                               foundation::Error{foundation::ErrorCode::resource_exhausted,
                                                 "pending queue is at capacity"}};
        }
        incumbent_bytes = ledger_.reservation_bytes(existing->second->reservation_key);
    } else if (pending_.size() >= config_.max_pending_jobs) {
        return SubmitResult{SubmitDisposition::rejected,
                           foundation::Error{foundation::ErrorCode::resource_exhausted,
                                             "pending queue is at capacity"}};
    }

    // The reservation is taken before the incumbent is stopped: a replacement the
    // ledger refuses must not leave the work already in flight destroyed. The
    // incumbent's bytes are counted as headroom because supersession releases them.
    const auto reservation_error =
        ledger_.reserve(fingerprint, request.estimated_bytes, incumbent_bytes);
    if (reservation_error.has_value()) {
        return SubmitResult{SubmitDisposition::rejected, reservation_error};
    }

    bool superseded_previous = false;
    if (existing != records_.end() && !existing->second->finalized) {
        // Different payload under one identity: the older submission stops being
        // the committable one before the newer one is accepted. A record that
        // already reached a terminal state is not touched again — its outcome is
        // fixed, so the replacement is a fresh submission rather than a
        // supersession.
        existing->second->supersede_requested = true;
        existing->second->stop_source.request_stop();
        superseded_previous = finalize_locked(
            existing->second, JobState::superseded, WorkerFailureReason::superseded,
            foundation::Error{foundation::ErrorCode::canceled,
                              "superseded by a newer submission with the same task identity"},
            std::nullopt, std::nullopt, WorkerStop::cooperative);
    }

    auto record =
        std::make_shared<JobRecord>(request, fingerprint, config_.max_progress_updates_per_job);
    records_[request.job_id] = record;
    if (!is_pending_locked(request.job_id)) {
        pending_.push_back(request.job_id);
    }
    queue_changed_.notify_one();

    return SubmitResult{superseded_previous ? SubmitDisposition::superseded_previous
                                            : SubmitDisposition::accepted,
                        std::nullopt};
}

bool JobScheduler::Impl::request_cancellation(const std::string& job_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto found = records_.find(job_id);
    if (found == records_.end() || found->second->finalized) {
        return false;
    }
    auto& record = found->second;
    record->cancel_requested = true;
    record->stop_source.request_stop();
    if (record->state == JobState::queued) {
        (void)drop_from_pending_locked(job_id);
        finalize_locked(record, JobState::canceled, WorkerFailureReason::canceled,
                        foundation::Error{foundation::ErrorCode::canceled,
                                          "canceled before the worker started"},
                        std::nullopt, std::nullopt, WorkerStop::cooperative);
    }
    return true;
}

bool JobScheduler::Impl::await_terminal(const std::string& job_id,
                                        std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    return terminal_changed_.wait_for(lock, timeout, [this, &job_id] {
        const auto found = records_.find(job_id);
        return (found != records_.end() && found->second->finalized) || shutdown_;
    });
}

std::optional<JobOutcome> JobScheduler::Impl::outcome(const std::string& job_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto found = records_.find(job_id);
    if (found == records_.end()) {
        return std::nullopt;
    }
    return found->second->outcome;
}

std::optional<JobOutcome> JobScheduler::Impl::superseded_outcome(const std::string& job_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto found = superseded_records_.find(job_id);
    if (found == superseded_records_.end()) {
        return std::nullopt;
    }
    return found->second.outcome;
}

ProgressState JobScheduler::Impl::progress(const std::string& job_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    ProgressState state;
    const auto found = records_.find(job_id);
    if (found == records_.end()) {
        return state;
    }
    state.updates = found->second->progress.snapshot();
    state.coalesced_count = found->second->progress.coalesced_count();
    state.indeterminate =
        std::any_of(state.updates.begin(), state.updates.end(),
                    [](const ProgressRecord& record) { return record.indeterminate(); });
    return state;
}

std::size_t JobScheduler::Impl::queue_depth() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pending_.size();
}

std::size_t JobScheduler::Impl::known_job_count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return records_.size();
}

std::uint64_t JobScheduler::Impl::available_bytes() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return ledger_.available_bytes();
}

std::size_t JobScheduler::Impl::reservation_count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return ledger_.reservation_count();
}

bool JobScheduler::Impl::drop_from_pending_locked(const std::string& job_id) {
    const auto found = std::find(pending_.begin(), pending_.end(), job_id);
    if (found == pending_.end()) {
        return false;
    }
    pending_.erase(found);
    return true;
}

bool JobScheduler::Impl::is_pending_locked(const std::string& job_id) const {
    return std::find(pending_.begin(), pending_.end(), job_id) != pending_.end();
}

LiveFlags JobScheduler::Impl::live_flags(const std::shared_ptr<JobRecord>& record) const {
    std::lock_guard<std::mutex> lock(mutex_);
    LiveFlags flags;
    flags.finalized = record->finalized;
    flags.cancel_requested = record->cancel_requested;
    flags.supersede_requested = record->supersede_requested;
    return flags;
}

bool JobScheduler::Impl::finalize_locked(
    const std::shared_ptr<JobRecord>& record, JobState state,
    std::optional<WorkerFailureReason> reason, std::optional<foundation::Error> error,
    std::optional<std::int64_t> result_value, const std::optional<ChildObservation>& observation,
    WorkerStop stop) {
    if (record->finalized) {
        return false;
    }
    record->finalized = true;

    // A candidate that arrives after the job stopped being committable is not a
    // success. The decision is taken here, under the scheduler lock, so it cannot
    // race with the flag the host set when it asked the worker to stop.
    if (state == JobState::succeeded && (record->cancel_requested || record->supersede_requested)) {
        state = JobState::canceled;
        reason = record->supersede_requested ? WorkerFailureReason::superseded
                                             : WorkerFailureReason::canceled;
        error = foundation::Error{foundation::ErrorCode::canceled,
                                  "worker returned a candidate after the job stopped "
                                  "being committable"};
        result_value = std::nullopt;
    }
    record->state = state;

    JobOutcome outcome;
    outcome.state = state;
    outcome.job_id = record->request.job_id;
    outcome.command_id = record->request.command_id;
    outcome.command_schema_version = record->request.command_schema_version;
    outcome.target_object = record->request.target_object;
    outcome.base_revision = record->request.base_revision;
    outcome.failure_reason = reason;
    outcome.error = std::move(error);
    outcome.result_value = result_value;
    outcome.checkpoints_observed = record->checkpoints;
    outcome.cancel_requested = record->cancel_requested;
    outcome.escalated_termination = stop == WorkerStop::killed;
    if (observation.has_value()) {
        outcome.process_id = observation->process_id;
        outcome.process_signal = observation->signal;
        outcome.process_exit_code = observation->exit_code;
    }

    record->outcome = outcome;
    if (state == JobState::superseded) {
        superseded_records_[record->request.job_id] = SupersededRecord{record, std::move(outcome)};
    }
    ledger_.release(record->reservation_key);
    queue_changed_.notify_all();
    terminal_changed_.notify_all();
    return true;
}

// Supersession is decided the moment the replacement is admitted, but how the
// old worker actually left is only known once it is reaped. The terminal record
// is already published, so the observation is attached to it instead of being
// dropped with the second finalize call that the stopped worker makes.
void JobScheduler::Impl::annotate_locked(const std::shared_ptr<JobRecord>& record,
                                         const ChildObservation& observation, WorkerStop stop) {
    if (!record->finalized || !record->outcome.has_value()) {
        return;
    }
    auto& outcome = *record->outcome;
    if (observation.process_id.has_value()) {
        outcome.process_id = observation.process_id;
    }
    if (observation.signal.has_value()) {
        outcome.process_signal = observation.signal;
    }
    if (observation.exit_code.has_value()) {
        outcome.process_exit_code = observation.exit_code;
    }
    // Escalation only ever upgrades: a worker that had to be killed is recorded as
    // killed even though the reason the incumbent published was its own.
    if (stop == WorkerStop::killed) {
        outcome.escalated_termination = true;
    }
    const auto found = superseded_records_.find(record->request.job_id);
    if (found == superseded_records_.end()) {
        return;
    }
    if (const auto source = found->second.source.lock(); source && source.get() == record.get()) {
        found->second.outcome = outcome;
    }
}

// Returns false when the worker's numbers cannot both be true: progress beyond
// the total it reported, or beyond the work the host asked for. Such a worker is
// not trustworthy about any of its counts, so the run is a protocol violation
// rather than something to clamp for display.
bool JobScheduler::Impl::record_progress(const std::shared_ptr<JobRecord>& record,
                                         const WorkerEnvelope& frame) {
    if (!frame.progress.has_value()) {
        return true;
    }
    const auto& progress = *frame.progress;
    if (progress.total_units.has_value() && progress.completed_units > *progress.total_units) {
        return false;
    }
    if (!record->request.total_is_unknown &&
        progress.completed_units > record->request.work_units) {
        return false;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    ++record->checkpoints;
    ProgressRecord entry;
    entry.stage = progress.stage;
    entry.completed_units = progress.completed_units;
    entry.total_units = progress.total_units;
    record->progress.push(std::move(entry));
    return true;
}

void JobScheduler::Impl::worker_loop() {
    block_sigpipe_on_this_thread();
    while (true) {
        std::shared_ptr<JobRecord> record;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            queue_changed_.wait(lock, [this] { return shutdown_ || !pending_.empty(); });
            // Shutting down means no new work: a child spawned here would be
            // killed by the destructor before it could report anything. The wait
            // predicate already holds the lock while it re-checks, so a non-empty
            // queue is guaranteed whenever shutdown is not set.
            if (shutdown_) {
                return;
            }
            const auto job_id = pending_.front();
            pending_.pop_front();
            const auto found = records_.find(job_id);
            if (found == records_.end() || found->second->finalized) {
                continue;
            }
            record = found->second;
            record->state = JobState::running;
        }
        run_job(record);
    }
}

void JobScheduler::Impl::run_job(const std::shared_ptr<JobRecord>& record) {
    if (record->request.transport == WorkerTransport::child_process) {
        run_child(record);
        return;
    }
    run_in_process(record);
}

void JobScheduler::Impl::run_in_process(const std::shared_ptr<JobRecord>& record) {
    const auto& request = record->request;
    const auto started = std::chrono::steady_clock::now();
    const auto token = record->stop_source.get_token();
    const auto failure_unit = std::max<std::int64_t>(1, request.work_units / 2);
    // An in-process job owns no process to observe and never needs a kill, so its
    // settle helper fixes both of those instead of repeating them at every call.
    const auto settle = [this, &record](JobState state, std::optional<WorkerFailureReason> reason,
                                       std::optional<foundation::Error> error,
                                       std::optional<std::int64_t> value) {
        std::lock_guard<std::mutex> lock(mutex_);
        (void)finalize_locked(record, state, reason, std::move(error), value, std::nullopt,
                              WorkerStop::cooperative);
    };

    for (std::int64_t unit = 0; unit < request.work_units; unit += request.checkpoint_every_units) {
        const auto flags = live_flags(record);
        if (flags.finalized) {
            return;
        }
        if (token.stop_requested()) {
            settle(JobState::canceled, WorkerFailureReason::canceled,
                   foundation::Error{foundation::ErrorCode::canceled,
                                     "worker stopped at a checkpoint"},
                   std::nullopt);
            return;
        }
        if (remaining_of(started, request.deadline_milliseconds).count() == 0) {
            settle(JobState::failed, WorkerFailureReason::timed_out,
                   foundation::Error{foundation::ErrorCode::io_error,
                                     "worker reached its deadline at a checkpoint"},
                   std::nullopt);
            return;
        }
        const auto completed = std::min(unit + request.checkpoint_every_units, request.work_units);
        (void)record_progress(record, make_progress_frame(request, kInProcessStage, completed));
        if (request.behavior == WorkerBehavior::numerical_failure && completed >= failure_unit) {
            settle(JobState::failed, WorkerFailureReason::numerical,
                   foundation::Error{foundation::ErrorCode::numerical_failure,
                                     "fake kernel failed at unit " + std::to_string(completed)},
                   std::nullopt);
            return;
        }
        std::this_thread::sleep_for(kCheckpointSlice);
    }

    settle(JobState::succeeded, WorkerFailureReason::none, std::nullopt, candidate_value(request));
}

void JobScheduler::Impl::run_child(const std::shared_ptr<JobRecord>& record) {
    const auto& request = record->request;
    const auto started = std::chrono::steady_clock::now();
    const std::chrono::milliseconds grace{
        request.cancel_grace_milliseconds >= 0 ? request.cancel_grace_milliseconds
                                                : config_.cancel_grace_milliseconds};
    ChildProcess child;
    std::optional<ChildObservation> observation;

    const auto settle = [this, &record, &observation](JobState state,
                                                     std::optional<WorkerFailureReason> reason,
                                                     std::optional<foundation::Error> error,
                                                     std::optional<std::int64_t> value,
                                                     WorkerStop stop) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (finalize_locked(record, state, reason, std::move(error), value, observation, stop)) {
            return;
        }
        if (observation.has_value()) {
            annotate_locked(record, *observation, stop);
        }
    };
    const auto reap = [&child, &observation](std::chrono::milliseconds timeout) {
        const auto status = child.wait(timeout);
        observation = observation_of(child, status);
        return status;
    };
    const auto stop_child = [&child, &reap]() {
        child.terminate_and_reap();
        (void)reap(std::chrono::milliseconds{0});
    };
    const auto reject = [&settle, &stop_child](std::string message) {
        stop_child();
        settle(JobState::failed, WorkerFailureReason::protocol_violation,
               foundation::Error{foundation::ErrorCode::invalid_request, std::move(message)},
               std::nullopt, WorkerStop::cooperative);
    };

    if (const auto error = child.start(request.child_program, {}); error.has_value()) {
        settle(JobState::failed, WorkerFailureReason::process_crashed, *error, std::nullopt,
               WorkerStop::cooperative);
        return;
    }
    if (!child.write_line(make_submit_frame(request).encode())) {
        stop_child();
        settle(JobState::failed, WorkerFailureReason::process_crashed,
               foundation::Error{foundation::ErrorCode::io_error, "worker stdin is not writable"},
               std::nullopt, WorkerStop::cooperative);
        return;
    }

    bool control_sent{false};
    bool timed_out{false};
    std::optional<std::chrono::steady_clock::time_point> control_deadline;

    while (true) {
        const auto flags = live_flags(record);
        // A record another owner already finalized is stopped exactly like one
        // that was asked to stop: announce the stop over the protocol, give the
        // worker the grace window, then escalate. The candidate it may still
        // produce is not committable either way.
        const bool stop_wanted =
            flags.finalized || flags.cancel_requested || flags.supersede_requested;
        if (stop_wanted && !control_sent) {
            // The stop is announced over the protocol first, so a cooperative
            // worker can leave on its own even when the record is already terminal.
            control_sent = true;
            (void)child.write_line(make_control_frame(
                request, flags.supersede_requested ? WorkerMessageKind::superseded
                                                   : WorkerMessageKind::cancel)
                                       .encode());
            control_deadline = std::chrono::steady_clock::now() + grace;
        }
        if (!timed_out && remaining_of(started, request.deadline_milliseconds).count() == 0) {
            timed_out = true;
            if (!control_sent) {
                control_sent = true;
                (void)child.write_line(
                    make_control_frame(request, WorkerMessageKind::cancel).encode());
            }
            control_deadline = std::chrono::steady_clock::now() + grace;
        }

        if (control_deadline.has_value() &&
            std::chrono::steady_clock::now() >= control_deadline.value()) {
            // Cooperative stop did not happen inside the grace window: escalate to
            // termination of the worker's own process, then record the stop.
            const auto reason =
                timed_out
                    ? WorkerFailureReason::timed_out
                    : (flags.supersede_requested ? WorkerFailureReason::superseded
                                                 : WorkerFailureReason::canceled);
            stop_child();
            settle(timed_out ? JobState::failed : JobState::canceled, reason,
                   foundation::Error{error_code_for(reason),
                                     timed_out ? "worker passed its deadline and was terminated"
                                               : "worker ignored cancellation and was terminated"},
                   std::nullopt, WorkerStop::killed);
            return;
        }

        auto budget = kReaderSlice;
        if (control_deadline.has_value()) {
            const auto left = std::chrono::duration_cast<std::chrono::milliseconds>(
                control_deadline.value() - std::chrono::steady_clock::now());
            const auto wait = left.count() > 0 ? left : std::chrono::milliseconds{0};
            budget = min_of(budget, wait);
        }

        const auto incoming = child.read_line(budget);
        if (incoming.status != ChildProcess::ReadStatus::line) {
            if (incoming.status == ChildProcess::ReadStatus::timeout) {
                continue;
            }
            auto status = reap(kChildReapGrace);
            if (!status.exited) {
                stop_child();
                status = reap(std::chrono::milliseconds{0});
            }
            const auto reason = classify_exit(status);
            settle(JobState::failed, reason,
                   foundation::Error{error_code_for(reason), describe_exit(status)}, std::nullopt,
                   WorkerStop::cooperative);
            return;
        }

        const auto decoded = decode_worker_frame(incoming.line);
        if (!decoded.ok()) {
            const bool bad_schema =
                decoded.error->code == foundation::ErrorCode::unsupported_schema_version;
            const auto reason = bad_schema ? WorkerFailureReason::unsupported_schema
                                           : WorkerFailureReason::protocol_violation;
            stop_child();
            settle(JobState::failed, reason, decoded.error, std::nullopt, WorkerStop::cooperative);
            return;
        }
        const auto& frame = decoded.envelope.value();
        if (frame.job_id != request.job_id) {
            reject("worker frame carries task identity " + frame.job_id + " instead of " +
                   request.job_id);
            return;
        }

        switch (frame.kind) {
        case WorkerMessageKind::accepted:
            break;
        case WorkerMessageKind::progress: {
            if (!record_progress(record, frame)) {
                const auto reported = frame.progress->completed_units;
                const auto total = frame.progress->total_units.value_or(request.work_units);
                reject("worker progress reports " + std::to_string(reported) + " of " +
                       std::to_string(total) + " units");
                return;
            }
            break;
        }
        case WorkerMessageKind::failure: {
            const bool reported = frame.failure_reason.has_value() &&
                                  *frame.failure_reason != WorkerFailureReason::none;
            const auto reason =
                reported ? *frame.failure_reason : WorkerFailureReason::protocol_violation;
            std::string message =
                reported
                    ? std::string{"worker reported "} + to_string(reason) +
                          (frame.detail.empty() ? std::string{} : ": " + frame.detail)
                    : std::string{"worker failure frame carries no reason"};
            // A worker that stops because it was asked to is a canceled job, not a
            // failed one; only a fault it hit on its own is a failure.
            const auto state = (reason == WorkerFailureReason::canceled ||
                                reason == WorkerFailureReason::superseded)
                                   ? JobState::canceled
                                   : JobState::failed;
            (void)reap(kChildReapGrace);
            settle(state, reason,
                   foundation::Error{error_code_for(reason), std::move(message)}, std::nullopt,
                   WorkerStop::cooperative);
            return;
        }
        case WorkerMessageKind::result: {
            if (frame.payload_key != std::string_view{kWorkerResultPayloadKey} ||
                !frame.payload_number.has_value()) {
                reject("result frame has no numeric value payload");
                return;
            }
            (void)child.write_line(
                make_control_frame(request, WorkerMessageKind::shutdown).encode());
            (void)reap(kChildReapGrace);
            // Whether the candidate is committable is decided by finalize, which
            // holds the lock that the stop request was recorded under.
            settle(JobState::succeeded, WorkerFailureReason::none, std::nullopt,
                   frame.payload_number, WorkerStop::cooperative);
            return;
        }
        case WorkerMessageKind::submit:
        case WorkerMessageKind::cancel:
        case WorkerMessageKind::superseded:
        case WorkerMessageKind::shutdown:
            reject(std::string{"worker sent a host-side frame: "} + to_string(frame.kind));
            return;
        }
    }
}

std::uint64_t JobScheduler::Impl::committed_bytes() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return ledger_.committed_bytes();
}

JobScheduler::JobScheduler(SchedulerConfig config)
    : impl_(std::make_unique<Impl>(std::move(config))) {}

JobScheduler::~JobScheduler() = default;

SubmitResult JobScheduler::submit(const JobRequest& request) {
    return impl_->submit(request);
}

bool JobScheduler::request_cancellation(const std::string& job_id) {
    return impl_->request_cancellation(job_id);
}

bool JobScheduler::await_terminal(const std::string& job_id, std::chrono::milliseconds timeout) {
    return impl_->await_terminal(job_id, timeout);
}

std::optional<JobOutcome> JobScheduler::outcome(const std::string& job_id) const {
    return impl_->outcome(job_id);
}

std::optional<JobOutcome> JobScheduler::superseded_outcome(const std::string& job_id) const {
    return impl_->superseded_outcome(job_id);
}

ProgressState JobScheduler::progress(const std::string& job_id) const {
    return impl_->progress(job_id);
}

std::size_t JobScheduler::queue_depth() const {
    return impl_->queue_depth();
}

std::size_t JobScheduler::known_job_count() const {
    return impl_->known_job_count();
}

std::uint64_t JobScheduler::available_bytes() const {
    return impl_->available_bytes();
}

std::uint64_t JobScheduler::committed_bytes() const {
    return impl_->committed_bytes();
}

std::size_t JobScheduler::reservation_count() const {
    return impl_->reservation_count();
}

const SchedulerConfig& JobScheduler::config() const noexcept {
    return impl_->config();
}

CommandOutcome commit_worker_result(CommandGateway& gateway, const JobOutcome& outcome,
                                    const WorkerCommitContext& context,
                                    std::stop_token stop_token) {
    // Cancellation and supersession stop work on purpose, so they are not command
    // failures; a fault the worker hit on its own is.
    if (outcome.state == JobState::canceled || outcome.state == JobState::superseded) {
        CommandOutcome canceled;
        canceled.status = CommandStatus::canceled;
        canceled.error = outcome.error.value_or(
            foundation::Error{foundation::ErrorCode::canceled, "worker job canceled"});
        return canceled;
    }
    if (!outcome.succeeded()) {
        CommandOutcome failed;
        failed.status = CommandStatus::failed;
        failed.error = outcome.error.value_or(
            foundation::Error{foundation::ErrorCode::internal_error,
                              "worker produced no committable candidate"});
        return failed;
    }
    if (!outcome.result_value.has_value()) {
        CommandOutcome failed;
        failed.status = CommandStatus::failed;
        failed.error = foundation::Error{foundation::ErrorCode::internal_error,
                                         "a succeeded worker job carries no candidate value"};
        return failed;
    }

    CommandRequest request;
    request.command_id = outcome.command_id;
    request.schema_version = outcome.command_schema_version;
    request.request_id = context.request_id;
    request.principal = context.principal;
    request.project_id = context.project_id;
    request.expected_revisions.reads[outcome.target_object] = outcome.base_revision;
    request.expected_revisions.writes[outcome.target_object] = outcome.base_revision;
    request.arguments[std::string{kWorkerResultPayloadKey}] = *outcome.result_value;
    return gateway.execute(request, stop_token);
}

}  // namespace everygraph::application
