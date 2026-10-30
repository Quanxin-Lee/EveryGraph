#pragma once

#include <cstddef>
#include <cstdint>
#include <list>
#include <map>
#include <optional>
#include <stop_token>
#include <string>
#include <string_view>
#include <vector>

#include "everygraph/application/command.hpp"
#include "everygraph/application/command_registry.hpp"
#include "everygraph/application/document.hpp"
#include "everygraph/application/event.hpp"

namespace everygraph::application {

struct LastActionRecord {
    std::string command_id;
    std::uint32_t schema_version{0};
    std::string principal;
    std::string project_id;
    std::optional<std::string> parent_workflow_id;
    ParameterSnapshot parameters;
    ExpectedRevisions original_expected_revisions;
    foundation::TransactionId transaction_id{foundation::kInvalidTransactionId};
    std::map<foundation::ObjectId, foundation::Revision> result_revisions;
    Repeatability repeatability{Repeatability::not_repeatable};
};

struct GatewayOptions {
    std::size_t max_undo_entries{128};
};

class CommandGateway {
public:
    explicit CommandGateway(CommandRegistry registry, DocumentSnapshot initial_state = {},
                            GatewayOptions options = {});
    CommandGateway(const CommandGateway&) = delete;
    CommandGateway& operator=(const CommandGateway&) = delete;

    [[nodiscard]] CommandOutcome execute(const CommandRequest& request,
                                         std::stop_token stop_token = {});
    [[nodiscard]] CommandOutcome undo(std::stop_token stop_token = {});
    [[nodiscard]] CommandOutcome redo(std::stop_token stop_token = {});
    [[nodiscard]] std::optional<CommandRequest> recorded_command(std::string new_request_id) const;
    [[nodiscard]] CommandOutcome repeat_last(std::string new_request_id,
                                             std::stop_token stop_token = {});
    [[nodiscard]] DocumentSnapshot snapshot() const;
    [[nodiscard]] std::optional<LastActionRecord> last_action() const;
    [[nodiscard]] foundation::EventSequence last_sequence() const noexcept;
    [[nodiscard]] std::size_t undo_depth() const noexcept;
    [[nodiscard]] std::size_t redo_depth() const noexcept;
    [[nodiscard]] const CommandRegistry& registry() const noexcept;

    SubscriptionId subscribe(EventListener listener);
    bool unsubscribe(SubscriptionId subscription_id);

private:
    struct UndoEntry {
        foundation::TransactionId transaction_id{foundation::kInvalidTransactionId};
        std::string command_id;
        std::map<foundation::ObjectId, std::optional<ObjectState>> before;
        std::map<foundation::ObjectId, std::optional<ObjectState>> after;
    };

    [[nodiscard]] std::optional<foundation::Error> validate_envelope(
        const CommandRequest& request) const;
    [[nodiscard]] std::optional<foundation::Error> validate_expected_revisions(
        const ExpectedRevisions& expected) const;
    [[nodiscard]] foundation::Revision current_revision(foundation::ObjectId id) const noexcept;
    [[nodiscard]] CommandOutcome commit_command(
        const CommandRequest& request, const CommandDefinition& definition,
        foundation::TransactionId transaction_id,
        std::map<foundation::ObjectId, ObjectState> candidate);
    [[nodiscard]] CommandOutcome apply_history_entry(
        const std::map<foundation::ObjectId, std::optional<ObjectState>>& target);
    [[nodiscard]] std::vector<EventListener> listener_snapshot() const;
    void publish(const DocumentEvent& event, const std::vector<EventListener>& listeners);

    CommandRegistry registry_;
    std::map<foundation::ObjectId, ObjectState> document_;
    std::map<foundation::ObjectId, foundation::Revision> retired_revisions_;
    std::list<UndoEntry> undo_stack_;
    std::list<UndoEntry> redo_stack_;
    std::optional<LastActionRecord> last_action_;
    std::map<SubscriptionId, EventListener> listeners_;
    foundation::EventSequence sequence_{foundation::kNoEventSequence};
    foundation::TransactionId next_transaction_id_{1};
    SubscriptionId next_subscription_id_{1};
    std::size_t max_undo_entries_{128};
    bool publishing_{false};
    bool executing_{false};
};

}  // namespace everygraph::application
