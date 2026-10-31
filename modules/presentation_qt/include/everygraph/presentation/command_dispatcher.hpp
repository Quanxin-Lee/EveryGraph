#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "everygraph/application/command.hpp"
#include "everygraph/application/document.hpp"

namespace everygraph::application {
class CommandGateway;
}  // namespace everygraph::application

namespace everygraph::presentation {

// The only presentation-layer boundary that submits commands. Widgets receive
// command bindings backed by this class and never hold the command gateway.
class ShellCommandDispatcher {
public:
    explicit ShellCommandDispatcher(application::CommandGateway& gateway);

    [[nodiscard]] application::CommandOutcome execute(
        std::string_view command_id, application::ParameterMap arguments,
        application::ExpectedRevisions expected = {});
    [[nodiscard]] application::CommandOutcome undo();
    [[nodiscard]] application::CommandOutcome redo();

    [[nodiscard]] const std::string& last_request_id() const noexcept;
    [[nodiscard]] const std::string& last_command_id() const noexcept;

private:
    [[nodiscard]] std::string make_request_id(std::string_view command_id);

    application::CommandGateway& gateway_;
    std::uint64_t next_request_{1};
    std::string last_request_id_;
    std::string last_command_id_;
};

// Read-only query boundary for widgets. The shell exposes only snapshots,
// subscriptions and undo/redo depth here, so presentation code cannot reach a
// kernel write method through its query reference.
class CommandQuery {
public:
    explicit CommandQuery(application::CommandGateway& gateway);

    [[nodiscard]] application::DocumentSnapshot snapshot() const;
    [[nodiscard]] std::size_t undo_depth() const noexcept;
    [[nodiscard]] std::size_t redo_depth() const noexcept;
    [[nodiscard]] application::SubscriptionId subscribe(
        application::EventListener listener);
    bool unsubscribe(application::SubscriptionId subscription_id);

private:
    application::CommandGateway& gateway_;
};

}  // namespace everygraph::presentation
