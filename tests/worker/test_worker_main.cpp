// Child side of the P1-005 worker prototype: a fake isolated worker that speaks
// the line protocol and can be told to misbehave. It runs no Python, no plugin,
// no provider and no numerical kernel.
#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <errno.h>
#include <poll.h>
#include <string>
#include <thread>
#include <unistd.h>

#include "everygraph/application/job_scheduler.hpp"
#include "everygraph/application/worker_protocol.hpp"

namespace {

using everygraph::application::decode_worker_frame;
using everygraph::application::to_string;
using everygraph::application::worker_behavior_from_string;
using everygraph::application::WorkerBehavior;
using everygraph::application::WorkerEnvelope;
using everygraph::application::WorkerFailureReason;
using everygraph::application::WorkerMessageKind;
using everygraph::application::WorkerProgress;
using everygraph::application::kWorkerMaxFrameBytes;
using everygraph::application::kWorkerProtocolSchemaVersion;
using everygraph::application::kWorkerResultPayloadKey;

class StdinChannel {
public:
    struct Input {
        bool closed{false};
        bool has_line{false};
        std::string line;
    };

    // timeout < 0 waits indefinitely; timeout == 0 only reports what is pending.
    [[nodiscard]] Input read(std::chrono::milliseconds timeout) {
        Input input;
        const auto buffered = buffer_.find('\n');
        if (buffered != std::string::npos) {
            input.has_line = true;
            input.line = buffer_.substr(0, buffered);
            buffer_.erase(0, buffered + 1);
            return input;
        }

        const auto started = std::chrono::steady_clock::now();
        while (true) {
            auto wait_milliseconds = -1;
            if (timeout.count() >= 0) {
                const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - started);
                // A zero timeout still has to poll once, otherwise the control
                // channel is never read and cooperative stop cannot happen.
                if (elapsed > timeout) {
                    return input;
                }
                wait_milliseconds = static_cast<int>((timeout - elapsed).count());
            }

            pollfd descriptor{};
            descriptor.fd = STDIN_FILENO;
            descriptor.events = POLLIN;
            const int ready = ::poll(&descriptor, 1, wait_milliseconds);
            if (ready == 0) {
                return input;
            }
            if (ready < 0) {
                if (errno == EINTR) {
                    continue;
                }
                input.closed = true;
                return input;
            }

            char chunk[512];
            const ssize_t read = ::read(STDIN_FILENO, chunk, sizeof(chunk));
            if (read == 0) {
                input.closed = true;
                return input;
            }
            if (read < 0) {
                if (errno == EINTR || errno == EAGAIN) {
                    continue;
                }
                input.closed = true;
                return input;
            }
            buffer_.append(chunk, static_cast<std::size_t>(read));
            if (buffer_.size() > kWorkerMaxFrameBytes) {
                buffer_.clear();
                input.closed = true;
                return input;
            }
            const auto newline = buffer_.find('\n');
            if (newline != std::string::npos) {
                input.has_line = true;
                input.line = buffer_.substr(0, newline);
                buffer_.erase(0, newline + 1);
                return input;
            }
        }
    }

private:
    std::string buffer_;
};

void emit_raw(const std::string& text) {
    std::size_t sent = 0;
    while (sent < text.size()) {
        const auto written = ::write(STDOUT_FILENO, text.data() + sent, text.size() - sent);
        if (written <= 0) {
            // The host closed the pipe; there is nobody left to report to.
            ::_exit(70);
        }
        sent += static_cast<std::size_t>(written);
    }
}

void emit(const WorkerEnvelope& frame) {
    emit_raw(frame.encode() + "\n");
}

[[nodiscard]] WorkerEnvelope response(WorkerMessageKind kind, const std::string& job_id) {
    WorkerEnvelope frame;
    frame.kind = kind;
    frame.job_id = job_id;
    return frame;
}

// Deliberately different from the in-process fake kernel so a test can tell
// which transport produced a candidate.
[[nodiscard]] std::int64_t fake_kernel_value(std::int64_t work_units) {
    return work_units * 3 + 1000;
}

[[nodiscard]] WorkerEnvelope progress_frame(const WorkerEnvelope& submit, std::int64_t completed) {
    auto frame = response(WorkerMessageKind::progress, submit.job_id);
    WorkerProgress progress;
    progress.stage = "child-compute";
    progress.completed_units = completed;
    if (!submit.total_is_unknown) {
        progress.total_units = submit.work_units;
    }
    frame.progress = progress;
    return frame;
}

struct ControlSignal {
    bool canceled{false};
    bool superseded{false};
    bool host_gone{false};
};

[[nodiscard]] ControlSignal poll_control(StdinChannel& channel) {
    ControlSignal signal;
    const auto incoming = channel.read(std::chrono::milliseconds{0});
    if (incoming.closed) {
        signal.host_gone = true;
        return signal;
    }
    if (!incoming.has_line) {
        return signal;
    }
    const auto decoded = decode_worker_frame(incoming.line);
    if (!decoded.ok() || !decoded.envelope.has_value()) {
        return signal;
    }
    const auto kind = decoded.envelope->kind;
    if (kind == WorkerMessageKind::cancel) {
        signal.canceled = true;
    } else if (kind == WorkerMessageKind::superseded) {
        signal.superseded = true;
    } else if (kind == WorkerMessageKind::shutdown) {
        signal.host_gone = true;
    }
    return signal;
}

[[nodiscard]] WorkerEnvelope failure_frame(const std::string& job_id, WorkerFailureReason reason,
                                           std::string detail) {
    auto frame = response(WorkerMessageKind::failure, job_id);
    frame.failure_reason = reason;
    frame.detail = std::move(detail);
    return frame;
}

int run_job(const WorkerEnvelope& submit, StdinChannel& channel) {
    const auto requested = worker_behavior_from_string(submit.job_mode);
    if (!requested.has_value()) {
        // An unknown mode is not a licence to run the default job: the worker
        // would report a candidate for behaviour nobody asked for.
        emit(failure_frame(submit.job_id, WorkerFailureReason::protocol_violation,
                           "unsupported job mode " + submit.job_mode));
        return 79;
    }
    const WorkerBehavior mode = *requested;

    if (mode == WorkerBehavior::malformed_frame) {
        emit_raw("this line is not a worker frame\n");
        return 0;
    }
    if (mode == WorkerBehavior::wrong_schema) {
        auto frame = progress_frame(submit, 1);
        frame.schema_version = kWorkerProtocolSchemaVersion + 1;
        emit(frame);
        return 0;
    }
    if (mode == WorkerBehavior::process_crash) {
        emit(response(WorkerMessageKind::accepted, submit.job_id));
        std::abort();
    }
    if (mode == WorkerBehavior::process_exit) {
        emit(response(WorkerMessageKind::accepted, submit.job_id));
        ::_exit(77);
    }
    if (mode == WorkerBehavior::wrong_job_identity) {
        auto frame = progress_frame(submit, 1);
        frame.job_id = submit.job_id + "-elsewhere";
        emit(frame);
        return 0;
    }
    if (mode == WorkerBehavior::echoes_host_frame) {
        // A host-side frame back on the worker channel is a protocol violation.
        emit(submit);
        return 0;
    }
    if (mode == WorkerBehavior::progress_overrun) {
        // More units done than the total it just reported: the host cannot trust
        // any number from a worker that breaks this rule.
        emit(response(WorkerMessageKind::accepted, submit.job_id));
        emit(progress_frame(submit, submit.work_units + 5));
        return 0;
    }
    if (mode == WorkerBehavior::late_result) {
        emit(response(WorkerMessageKind::accepted, submit.job_id));
        emit(progress_frame(submit, std::min<std::int64_t>(1, submit.work_units)));
        // Wait until the host has stopped the job, then answer with a candidate
        // anyway, so the not-committable rule is proven without any timing margin.
        const auto patient_deadline =
            std::chrono::steady_clock::now() + std::chrono::milliseconds{8000};
        while (std::chrono::steady_clock::now() < patient_deadline) {
            const auto control = poll_control(channel);
            if (control.canceled || control.superseded || control.host_gone) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds{5});
        }
        auto frame = response(WorkerMessageKind::result, submit.job_id);
        frame.payload_key = std::string{kWorkerResultPayloadKey};
        frame.payload_number = fake_kernel_value(submit.work_units);
        emit(frame);
        return 0;
    }

    emit(response(WorkerMessageKind::accepted, submit.job_id));

    const auto step = submit.checkpoint_every_units > 0 ? submit.checkpoint_every_units : 1;
    for (std::int64_t unit = 0; unit < submit.work_units; unit += step) {
        if (mode == WorkerBehavior::unresponsive) {
            // Never reads its control channel and never reports: the host has to
            // escalate a deadline overrun to termination of this process.
            std::this_thread::sleep_for(std::chrono::milliseconds{60});
            continue;
        }
        const auto completed = std::min(unit + step, submit.work_units);
        emit(progress_frame(submit, completed));
        const auto control = poll_control(channel);
        if (control.host_gone) {
            return 0;
        }
        if ((control.canceled || control.superseded) && mode != WorkerBehavior::ignores_cancel) {
            emit(control.superseded
                     ? failure_frame(submit.job_id, WorkerFailureReason::superseded,
                                     "superseded by a newer submission")
                     : failure_frame(submit.job_id, WorkerFailureReason::canceled,
                                     "stopped at a checkpoint"));
            return 0;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{5});
    }

    if (mode == WorkerBehavior::numerical_failure) {
        emit(failure_frame(submit.job_id, WorkerFailureReason::numerical, "fake kernel diverged"));
        return 0;
    }

    auto frame = response(WorkerMessageKind::result, submit.job_id);
    frame.payload_key = std::string{kWorkerResultPayloadKey};
    frame.payload_number = fake_kernel_value(submit.work_units);
    emit(frame);
    (void)channel.read(std::chrono::milliseconds{2000});
    return 0;
}

}  // namespace

int main() {
    // The host gives a worker the default signal dispositions, so ignoring
    // SIGPIPE is this worker's own choice: it keeps a closed pipe an error return
    // the code can turn into its own exit status instead of a signal death that
    // tells the host nothing.
    std::signal(SIGPIPE, SIG_IGN);
    StdinChannel channel;
    while (true) {
        const auto incoming = channel.read(std::chrono::milliseconds{-1});
        if (incoming.closed) {
            return 0;
        }
        if (!incoming.has_line) {
            continue;
        }
        const auto decoded = decode_worker_frame(incoming.line);
        if (!decoded.ok()) {
            return 78;
        }
        const WorkerEnvelope submit = decoded.envelope.value();
        if (submit.kind == WorkerMessageKind::shutdown) {
            return 0;
        }
        if (submit.kind != WorkerMessageKind::submit) {
            continue;
        }
        return run_job(submit, channel);
    }
}
