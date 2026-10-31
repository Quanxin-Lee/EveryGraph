#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "everygraph/foundation/error.hpp"

namespace everygraph::application {

// macOS-only process helper for the P1-005 worker prototype. It carries no
// platform handle into the caller's public surface: the process identity is a
// plain integer so no platform-specific type reaches a module interface.
//
// Writing to a pipe whose read end is closed raises SIGPIPE. The thread driving a
// ChildProcess blocks that signal itself, which is what JobScheduler's own worker
// threads do, so a worker that dies mid-write surfaces as an EPIPE write_line
// reports instead of a signal that terminates the host. start() then resets the
// child's signal mask and dispositions, so the worker still gets the default
// behaviour a standalone process expects.
//
// The worker's stderr is discarded: the framing pipe is the only channel this
// transport reads, so no worker diagnostic is readable yet.
class ChildProcess {
public:
    enum class ReadStatus {
        line,
        timeout,
        closed,
        error,
    };

    struct ReadResult {
        ReadStatus status{ReadStatus::error};
        std::string line;
    };

    struct ExitStatus {
        bool exited{false};
        bool signalled{false};
        int exit_code{0};
        int signal{0};
    };

    ChildProcess() = default;
    ChildProcess(const ChildProcess&) = delete;
    ChildProcess& operator=(const ChildProcess&) = delete;
    ~ChildProcess();

    [[nodiscard]] std::optional<foundation::Error> start(const std::string& program,
                                                         const std::vector<std::string>& arguments);
    // Returns false instead of blocking forever when the worker stops draining its
    // stdin, so the host can still reach its escalation path.
    [[nodiscard]] bool write_line(const std::string& line,
                                  std::chrono::milliseconds timeout =
                                      std::chrono::milliseconds{200});
    [[nodiscard]] ReadResult read_line(std::chrono::milliseconds timeout);
    [[nodiscard]] ExitStatus wait(std::chrono::milliseconds timeout);
    // Escalation is a single unconditional kill: there is no polite-request stage
    // before it, because the cooperative stop is the protocol's own cancel frame.
    void terminate_and_reap();

    [[nodiscard]] std::uint64_t process_id() const noexcept { return pid_; }
    [[nodiscard]] bool running() const noexcept { return pid_ != 0 && !reaped_; }

private:
    void close_pipes();

    int write_fd_{-1};
    int read_fd_{-1};
    std::uint64_t pid_{0};
    bool reaped_{false};
    ExitStatus last_status_{};
    std::string read_buffer_;
};

}  // namespace everygraph::application
