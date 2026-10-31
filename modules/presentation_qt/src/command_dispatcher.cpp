#include "everygraph/presentation/command_dispatcher.hpp"

#include <utility>

#include "everygraph/application/command_gateway.hpp"

namespace everygraph::presentation {

ShellCommandDispatcher::ShellCommandDispatcher(application::CommandGateway& gateway)
    : gateway_(gateway) {}

std::string ShellCommandDispatcher::make_request_id(std::string_view command_id) {
    return "shell-" + std::to_string(next_request_++) + "-" + std::string(command_id);
}

application::CommandOutcome ShellCommandDispatcher::execute(
    std::string_view command_id, application::ParameterMap arguments,
    application::ExpectedRevisions expected) {
    application::CommandRequest request;
    request.command_id = std::string(command_id);
    request.schema_version = 1;
    request.request_id = make_request_id(command_id);
    request.principal = "ui.shell";
    request.project_id = "prototype";
    request.expected_revisions = std::move(expected);
    request.arguments = std::move(arguments);
    last_command_id_ = request.command_id;
    last_request_id_ = request.request_id;
    return gateway_.execute(request);
}

application::CommandOutcome ShellCommandDispatcher::undo() {
    last_command_id_ = "history.undo";
    last_request_id_ = make_request_id(last_command_id_);
    return gateway_.undo();
}

application::CommandOutcome ShellCommandDispatcher::redo() {
    last_command_id_ = "history.redo";
    last_request_id_ = make_request_id(last_command_id_);
    return gateway_.redo();
}

const std::string& ShellCommandDispatcher::last_request_id() const noexcept {
    return last_request_id_;
}

const std::string& ShellCommandDispatcher::last_command_id() const noexcept {
    return last_command_id_;
}

CommandQuery::CommandQuery(application::CommandGateway& gateway) : gateway_(gateway) {}

application::DocumentSnapshot CommandQuery::snapshot() const {
    return gateway_.snapshot();
}

std::size_t CommandQuery::undo_depth() const noexcept {
    return gateway_.undo_depth();
}

std::size_t CommandQuery::redo_depth() const noexcept {
    return gateway_.redo_depth();
}

application::SubscriptionId CommandQuery::subscribe(application::EventListener listener) {
    return gateway_.subscribe(std::move(listener));
}

bool CommandQuery::unsubscribe(application::SubscriptionId subscription_id) {
    return gateway_.unsubscribe(subscription_id);
}

}  // namespace everygraph::presentation
