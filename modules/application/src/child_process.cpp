#include "child_process.hpp"

#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <spawn.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

#include <cerrno>
#include <cstddef>

#include "everygraph/application/worker_protocol.hpp"

extern char** environ;

namespace everygraph::application {
namespace {

constexpr std::chrono::milliseconds kReapSlice{5};

[[nodiscard]] foundation::Error spawn_error(const char* operation, int error_number) {
    return foundation::Error{
        foundation::ErrorCode::io_error,
        std::string{operation} + " failed: " + std::strerror(error_number)};
}

}  // namespace

ChildProcess::~ChildProcess() {
    if (running()) {
        terminate_and_reap();
    }
    close_pipes();
}

std::optional<foundation::Error> ChildProcess::start(const std::string& program,
                                                    const std::vector<std::string>& arguments) {
    int to_child[2] = {-1, -1};
    int from_child[2] = {-1, -1};
    if (::pipe(to_child) != 0) {
        return spawn_error("pipe", errno);
    }
    if (::pipe(from_child) != 0) {
        const int pipe_error = errno;
        ::close(to_child[0]);
        ::close(to_child[1]);
        return spawn_error("pipe", pipe_error);
    }

    const auto close_parents = [to_child, from_child] {
        ::close(to_child[0]);
        ::close(to_child[1]);
        ::close(from_child[0]);
        ::close(from_child[1]);
    };

    posix_spawn_file_actions_t file_actions;
    // These setup calls return an errno value themselves and do not set the global
    // errno, so the returned code is what a diagnostic can quote.
    const int actions_init = posix_spawn_file_actions_init(&file_actions);
    if (actions_init != 0) {
        close_parents();
        return spawn_error("posix_spawn_file_actions_init", actions_init);
    }
    const int stdin_action =
        posix_spawn_file_actions_adddup2(&file_actions, to_child[0], STDIN_FILENO);
    const int stdout_action =
        posix_spawn_file_actions_adddup2(&file_actions, from_child[1], STDOUT_FILENO);
    // The framing pipe is the only channel the transport reads, so a worker's own
    // diagnostics must not share it. This prototype discards stderr instead; a
    // real transport needs a separate diagnostic channel before a worker crash
    // log can be read at all.
    const int stderr_action =
        posix_spawn_file_actions_addopen(&file_actions, STDERR_FILENO, "/dev/null", O_WRONLY, 0);
    if (stdin_action != 0 || stdout_action != 0 || stderr_action != 0) {
        const int action_error = stdin_action != 0   ? stdin_action
                                 : stdout_action != 0 ? stdout_action
                                                      : stderr_action;
        posix_spawn_file_actions_destroy(&file_actions);
        close_parents();
        return spawn_error("posix_spawn_file_actions", action_error);
    }

    posix_spawnattr_t attributes;
    const int attributes_init = posix_spawnattr_init(&attributes);
    if (attributes_init != 0) {
        posix_spawn_file_actions_destroy(&file_actions);
        close_parents();
        return spawn_error("posix_spawnattr_init", attributes_init);
    }
    // The transport blocks SIGPIPE on the thread that owns these pipes, so the
    // child's mask and dispositions are reset here: a worker has to see the
    // signal behaviour its own code expects, not the one the host chose.
    const short spawn_flags =
        POSIX_SPAWN_CLOEXEC_DEFAULT | POSIX_SPAWN_SETSIGMASK | POSIX_SPAWN_SETSIGDEF;
    sigset_t child_mask;
    sigemptyset(&child_mask);
    sigset_t every_signal;
    sigfillset(&every_signal);
    const int flags_result = posix_spawnattr_setflags(&attributes, spawn_flags);
    const int mask_result = posix_spawnattr_setsigmask(&attributes, &child_mask);
    const int disposition_result = posix_spawnattr_setsigdefault(&attributes, &every_signal);
    if (flags_result != 0 || mask_result != 0 || disposition_result != 0) {
        const int attribute_error = flags_result != 0        ? flags_result
                                    : mask_result != 0       ? mask_result
                                                             : disposition_result;
        posix_spawnattr_destroy(&attributes);
        posix_spawn_file_actions_destroy(&file_actions);
        close_parents();
        return spawn_error("posix_spawnattr", attribute_error);
    }

    std::vector<char*> argv;
    argv.push_back(const_cast<char*>(program.c_str()));
    for (const auto& argument : arguments) {
        argv.push_back(const_cast<char*>(argument.c_str()));
    }
    argv.push_back(nullptr);

    pid_t spawned = 0;
    const int spawn_result = posix_spawn(&spawned, program.c_str(), &file_actions, &attributes,
                                         argv.data(), environ);

    posix_spawnattr_destroy(&attributes);
    posix_spawn_file_actions_destroy(&file_actions);
    ::close(to_child[0]);
    ::close(from_child[1]);

    if (spawn_result != 0) {
        ::close(to_child[1]);
        ::close(from_child[0]);
        return spawn_error("posix_spawn", spawn_result);
    }

    write_fd_ = to_child[1];
    read_fd_ = from_child[0];
    // The child's read end is a different open file description, so this does not
    // change how the worker itself reads its stdin.
    const auto flags = ::fcntl(write_fd_, F_GETFL, 0);
    if (flags != -1) {
        ::fcntl(write_fd_, F_SETFL, flags | O_NONBLOCK);
    }
    pid_ = static_cast<std::uint64_t>(spawned);
    reaped_ = false;
    return std::nullopt;
}

bool ChildProcess::write_line(const std::string& line, std::chrono::milliseconds timeout) {
    if (write_fd_ < 0) {
        return false;
    }
    std::string frame = line;
    frame.push_back('\n');

    const auto started = std::chrono::steady_clock::now();
    std::size_t written = 0;
    while (written < frame.size()) {
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - started);
        const auto remaining = timeout > elapsed ? timeout - elapsed : std::chrono::milliseconds{0};

        pollfd descriptor{};
        descriptor.fd = write_fd_;
        descriptor.events = POLLOUT;
        const int ready = ::poll(&descriptor, 1, static_cast<int>(remaining.count()));
        if (ready < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        if (ready == 0) {
            return false;
        }

        const auto result = ::write(write_fd_, frame.data() + written, frame.size() - written);
        if (result < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            return false;
        }
        written += static_cast<std::size_t>(result);
    }
    return true;
}

ChildProcess::ReadResult ChildProcess::read_line(std::chrono::milliseconds timeout) {
    ReadResult result;
    if (read_fd_ < 0) {
        result.status = ReadStatus::closed;
        return result;
    }

    // A single read can carry several frames, so undelivered lines stay in the
    // channel buffer instead of being dropped when one line is returned.
    const auto take_line = [this, &result] {
        const auto newline = read_buffer_.find('\n');
        if (newline == std::string::npos) {
            return false;
        }
        result.line = read_buffer_.substr(0, newline);
        read_buffer_.erase(0, newline + 1);
        return true;
    };
    if (take_line()) {
        result.status = ReadStatus::line;
        return result;
    }

    const auto started = std::chrono::steady_clock::now();
    while (true) {
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - started);
        const auto remaining = timeout > elapsed ? timeout - elapsed : std::chrono::milliseconds{0};

        pollfd descriptor{};
        descriptor.fd = read_fd_;
        descriptor.events = POLLIN;
        const int ready = ::poll(&descriptor, 1, static_cast<int>(remaining.count()));
        if (ready == 0) {
            result.status = ReadStatus::timeout;
            return result;
        }
        if (ready < 0) {
            if (errno == EINTR) {
                continue;
            }
            result.status = ReadStatus::error;
            return result;
        }

        char buffer[1024];
        const ssize_t read = ::read(read_fd_, buffer, sizeof(buffer));
        if (read < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                continue;
            }
            result.status = ReadStatus::error;
            return result;
        }
        if (read == 0) {
            result.status = ReadStatus::closed;
            return result;
        }
        read_buffer_.append(buffer, static_cast<std::size_t>(read));
        if (read_buffer_.size() > kWorkerMaxFrameBytes) {
            read_buffer_.clear();
            result.status = ReadStatus::error;
            return result;
        }
        if (take_line()) {
            result.status = ReadStatus::line;
            return result;
        }
    }
}

ChildProcess::ExitStatus ChildProcess::wait(std::chrono::milliseconds timeout) {
    ExitStatus status;
    if (pid_ == 0) {
        status.exited = true;
        return status;
    }
    if (reaped_) {
        status = last_status_;
        status.exited = true;
        return status;
    }

    const auto started = std::chrono::steady_clock::now();
    while (true) {
        int raw_status = 0;
        const pid_t result = ::waitpid(static_cast<pid_t>(pid_), &raw_status, WNOHANG);
        if (result == static_cast<pid_t>(pid_)) {
            reaped_ = true;
            status.exited = true;
            if (WIFEXITED(raw_status) != 0) {
                status.exit_code = WEXITSTATUS(raw_status);
            } else if (WIFSIGNALED(raw_status) != 0) {
                status.signalled = true;
                status.signal = WTERMSIG(raw_status);
            }
            last_status_ = status;
            return status;
        }
        if (result < 0 && errno != EINTR) {
            reaped_ = true;
            status.exited = true;
            last_status_ = status;
            return status;
        }
        if (std::chrono::steady_clock::now() - started >= timeout) {
            return status;
        }
        std::this_thread::sleep_for(kReapSlice);
    }
}

void ChildProcess::terminate_and_reap() {
    if (pid_ != 0 && !reaped_) {
        ::kill(static_cast<pid_t>(pid_), SIGKILL);
        (void)wait(std::chrono::milliseconds{500});
    }
    close_pipes();
}

void ChildProcess::close_pipes() {
    if (write_fd_ >= 0) {
        ::close(write_fd_);
        write_fd_ = -1;
    }
    if (read_fd_ >= 0) {
        ::close(read_fd_);
        read_fd_ = -1;
    }
}

}  // namespace everygraph::application
