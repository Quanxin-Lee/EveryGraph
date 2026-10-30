#include "everygraph/application/command_gateway.hpp"

#include <algorithm>
#include <exception>
#include <iterator>
#include <type_traits>
#include <utility>

namespace everygraph::application {
namespace {

CommandOutcome make_failure(foundation::ErrorCode code, std::string message) {
    CommandOutcome outcome;
    outcome.status = CommandStatus::failed;
    outcome.error = foundation::Error{code, std::move(message)};
    return outcome;
}

CommandOutcome make_canceled(std::string message) {
    CommandOutcome outcome;
    outcome.status = CommandStatus::canceled;
    outcome.error = foundation::Error{foundation::ErrorCode::canceled, std::move(message)};
    return outcome;
}

bool same_content(const ObjectState& left, const ObjectState& right) {
    return left.id == right.id && left.type == right.type &&
           parameters_equal(left.properties, right.properties);
}

struct OperationGuard {
    explicit OperationGuard(bool& flag) : flag_(flag) { flag_ = true; }
    ~OperationGuard() { flag_ = false; }

    OperationGuard(const OperationGuard&) = delete;
    OperationGuard& operator=(const OperationGuard&) = delete;

private:
    bool& flag_;
};

static_assert(std::is_nothrow_move_assignable_v<LastActionRecord>,
              "last-action assignment must not throw after the undo entry is staged");

}  // namespace

CommandGateway::CommandGateway(CommandRegistry registry, DocumentSnapshot initial_state,
                               GatewayOptions options)
    : registry_(std::move(registry)),
      document_(std::move(initial_state.objects)),
      max_undo_entries_(options.max_undo_entries) {}

std::optional<foundation::Error> CommandGateway::validate_envelope(
    const CommandRequest& request) const {
    if (request.command_id.empty()) {
        return foundation::Error{foundation::ErrorCode::invalid_request,
                                 "command_id must not be empty"};
    }
    if (request.request_id.empty()) {
        return foundation::Error{foundation::ErrorCode::invalid_request,
                                 "request_id must not be empty"};
    }
    if (request.principal.empty()) {
        return foundation::Error{foundation::ErrorCode::invalid_request,
                                 "principal must not be empty"};
    }
    if (request.project_id.empty()) {
        return foundation::Error{foundation::ErrorCode::invalid_request,
                                 "project_id must not be empty"};
    }
    if (request.parent_workflow_id.has_value() && request.parent_workflow_id->empty()) {
        return foundation::Error{foundation::ErrorCode::invalid_request,
                                 "parent_workflow_id must not be empty when present"};
    }
    for (const auto& entry : request.expected_revisions.reads) {
        if (!entry.first.is_valid()) {
            return foundation::Error{foundation::ErrorCode::invalid_request,
                                     "read revision references object id zero"};
        }
    }
    for (const auto& entry : request.expected_revisions.writes) {
        if (!entry.first.is_valid()) {
            return foundation::Error{foundation::ErrorCode::invalid_request,
                                     "write revision references object id zero"};
        }
    }
    for (const auto& entry : request.expected_revisions.reads) {
        const auto write = request.expected_revisions.writes.find(entry.first);
        if (write != request.expected_revisions.writes.end() && write->second != entry.second) {
            return foundation::Error{
                foundation::ErrorCode::invalid_request,
                "conflicting read and write revisions for object " +
                    foundation::to_string(entry.first)};
        }
    }
    return std::nullopt;
}

std::optional<foundation::Error> CommandGateway::validate_expected_revisions(
    const ExpectedRevisions& expected) const {
    for (const auto& entry : expected.reads) {
        const foundation::Revision current = current_revision(entry.first);
        if (current != entry.second) {
            return foundation::Error{
                foundation::ErrorCode::revision_conflict,
                "object " + foundation::to_string(entry.first) + " is at revision " +
                    std::to_string(current) + ", expected read revision " +
                    std::to_string(entry.second)};
        }
    }
    for (const auto& entry : expected.writes) {
        const foundation::Revision current = current_revision(entry.first);
        if (current != entry.second) {
            return foundation::Error{
                foundation::ErrorCode::revision_conflict,
                "object " + foundation::to_string(entry.first) + " is at revision " +
                    std::to_string(current) + ", expected write revision " +
                    std::to_string(entry.second)};
        }
    }
    return std::nullopt;
}

foundation::Revision CommandGateway::current_revision(foundation::ObjectId id) const noexcept {
    const auto iterator = document_.find(id);
    if (iterator != document_.end()) {
        return iterator->second.revision;
    }
    const auto retired = retired_revisions_.find(id);
    return retired == retired_revisions_.end() ? foundation::kInitialRevision : retired->second;
}

CommandOutcome CommandGateway::execute(const CommandRequest& request, std::stop_token stop_token) {
    if (publishing_ || executing_) {
        return make_failure(foundation::ErrorCode::reentrant_mutation,
                            "commands cannot be issued from an event listener or handler");
    }
    if (stop_token.stop_requested()) {
        return make_canceled("command canceled before dispatch");
    }
    const auto envelope_error = validate_envelope(request);
    if (envelope_error.has_value()) {
        return make_failure(envelope_error->code, envelope_error->message);
    }
    const auto validation_error = registry_.validate(request);
    if (validation_error.has_value()) {
        return make_failure(validation_error->code, validation_error->message);
    }
    const CommandDefinition* definition = registry_.find(request.command_id);
    if (definition == nullptr) {
        return make_failure(foundation::ErrorCode::unknown_command,
                            "unknown command " + request.command_id);
    }
    if (stop_token.stop_requested()) {
        return make_canceled("command canceled before execution");
    }
    const auto revision_error = validate_expected_revisions(request.expected_revisions);
    if (revision_error.has_value()) {
        return make_failure(revision_error->code, revision_error->message);
    }

    CommandTransaction transaction(document_);
    ExecutionContext context{next_transaction_id_++, stop_token};
    OperationGuard operation_guard{executing_};
    std::optional<foundation::Error> handler_error;
    try {
        handler_error = definition->execute(transaction, request, context);
    } catch (const std::exception& exception) {
        return make_failure(foundation::ErrorCode::internal_error,
                            "command handler failed: " + std::string(exception.what()));
    } catch (...) {
        return make_failure(foundation::ErrorCode::internal_error,
                            "command handler failed with an unknown exception");
    }
    if (handler_error.has_value()) {
        if (handler_error->code == foundation::ErrorCode::canceled ||
            stop_token.stop_requested()) {
            return make_canceled(handler_error->code == foundation::ErrorCode::canceled
                                     ? handler_error->message
                                     : "command canceled during execution");
        }
        return make_failure(handler_error->code, handler_error->message);
    }
    if (stop_token.stop_requested()) {
        return make_canceled("command canceled before commit");
    }
    const auto commit_revision_error = validate_expected_revisions(request.expected_revisions);
    if (commit_revision_error.has_value()) {
        return make_failure(commit_revision_error->code, commit_revision_error->message);
    }
    try {
        return commit_command(request, *definition, context.transaction_id,
                              std::move(transaction.candidate_));
    } catch (const std::exception& exception) {
        return make_failure(foundation::ErrorCode::internal_error,
                            "command commit failed: " + std::string(exception.what()));
    } catch (...) {
        return make_failure(foundation::ErrorCode::internal_error,
                            "command commit failed with an unknown exception");
    }
}

CommandOutcome CommandGateway::commit_command(
    const CommandRequest& request, const CommandDefinition& definition,
    foundation::TransactionId transaction_id,
    std::map<foundation::ObjectId, ObjectState> candidate) {
    std::vector<foundation::ObjectId> changed_ids;
    for (const auto& entry : document_) {
        const auto candidate_iterator = candidate.find(entry.first);
        if (candidate_iterator == candidate.end() ||
            !same_content(candidate_iterator->second, entry.second)) {
            changed_ids.push_back(entry.first);
        }
    }
    for (const auto& entry : candidate) {
        if (!document_.contains(entry.first)) {
            changed_ids.push_back(entry.first);
        }
    }
    if (changed_ids.empty()) {
        CommandOutcome outcome;
        outcome.status = CommandStatus::succeeded;
        return outcome;
    }
    if (definition.effect == EffectClass::read_only) {
        return make_failure(
            foundation::ErrorCode::invalid_argument,
            "read-only command " + request.command_id + " attempted a document write");
    }
    for (const auto id : changed_ids) {
        if (!request.expected_revisions.writes.contains(id)) {
            return make_failure(
                foundation::ErrorCode::invalid_argument,
                "command modified object " + foundation::to_string(id) +
                    " outside its declared write set");
        }
    }
    std::sort(changed_ids.begin(), changed_ids.end());

    UndoEntry undo_entry;
    undo_entry.transaction_id = transaction_id;
    undo_entry.command_id = request.command_id;
    std::vector<ObjectChange> changes;
    changes.reserve(changed_ids.size());
    std::map<foundation::ObjectId, foundation::Revision> revisions;
    std::map<foundation::ObjectId, foundation::Revision> next_retired = retired_revisions_;
    for (const auto id : changed_ids) {
        const auto live_iterator = document_.find(id);
        if (live_iterator == document_.end()) {
            undo_entry.before.emplace(id, std::nullopt);
        } else {
            undo_entry.before.emplace(id, live_iterator->second);
        }
        const auto candidate_iterator = candidate.find(id);
        if (candidate_iterator != candidate.end()) {
            const foundation::Revision new_revision = current_revision(id) + 1;
            candidate_iterator->second.revision = new_revision;
            undo_entry.after.emplace(id, candidate_iterator->second);
            changes.push_back(ObjectChange{
                id, new_revision,
                live_iterator == document_.end() ? ChangeKind::created : ChangeKind::updated});
            revisions.emplace(id, new_revision);
            next_retired.erase(id);
        } else {
            undo_entry.after.emplace(id, std::nullopt);
            changes.push_back(ObjectChange{id, foundation::kInitialRevision, ChangeKind::erased});
            revisions.emplace(id, foundation::kInitialRevision);
            if (live_iterator != document_.end()) {
                next_retired[id] = live_iterator->second.revision;
            }
        }
    }

    LastActionRecord record;
    record.command_id = request.command_id;
    record.schema_version = request.schema_version;
    record.principal = request.principal;
    record.project_id = request.project_id;
    record.parent_workflow_id = request.parent_workflow_id;
    record.parameters = ParameterSnapshot{request.arguments};
    record.original_expected_revisions = request.expected_revisions;
    record.transaction_id = transaction_id;
    record.result_revisions = revisions;
    record.repeatability = definition.repeatability;

    const foundation::EventSequence next_sequence = sequence_ + 1;
    DocumentEvent event{transaction_id, next_sequence, std::move(changes)};
    CommandOutcome outcome;
    outcome.status = CommandStatus::succeeded;
    outcome.transaction_id = transaction_id;
    outcome.events.push_back(event);
    outcome.revisions = std::move(revisions);
    outcome.changed = true;
    const std::vector<EventListener> listeners = listener_snapshot();

    if (max_undo_entries_ > 0) {
        undo_stack_.push_back(std::move(undo_entry));
        while (undo_stack_.size() > max_undo_entries_) {
            undo_stack_.pop_front();
        }
    }
    redo_stack_.clear();
    last_action_ = std::move(record);
    document_.swap(candidate);
    retired_revisions_.swap(next_retired);
    sequence_ = next_sequence;
    publish(event, listeners);
    return outcome;
}

CommandOutcome CommandGateway::apply_history_entry(
    const std::map<foundation::ObjectId, std::optional<ObjectState>>& target) {
    std::map<foundation::ObjectId, ObjectState> next = document_;
    std::map<foundation::ObjectId, foundation::Revision> next_retired = retired_revisions_;
    std::vector<ObjectChange> changes;
    std::map<foundation::ObjectId, foundation::Revision> revisions;
    for (const auto& entry : target) {
        const auto current = next.find(entry.first);
        if (entry.second.has_value()) {
            if (current != next.end() && same_content(current->second, *entry.second)) {
                continue;
            }
            ObjectState state = *entry.second;
            const foundation::Revision new_revision = current_revision(entry.first) + 1;
            state.revision = new_revision;
            const ChangeKind kind =
                current == next.end() ? ChangeKind::created : ChangeKind::updated;
            next.insert_or_assign(entry.first, std::move(state));
            changes.push_back(ObjectChange{entry.first, new_revision, kind});
            revisions.emplace(entry.first, new_revision);
            next_retired.erase(entry.first);
        } else {
            if (current == next.end()) {
                continue;
            }
            const foundation::Revision erased_revision = current->second.revision;
            next.erase(current);
            changes.push_back(ObjectChange{entry.first, foundation::kInitialRevision,
                                           ChangeKind::erased});
            revisions.emplace(entry.first, foundation::kInitialRevision);
            next_retired[entry.first] = erased_revision;
        }
    }
    if (changes.empty()) {
        CommandOutcome outcome;
        outcome.status = CommandStatus::succeeded;
        return outcome;
    }
    const foundation::TransactionId transaction_id = next_transaction_id_++;
    const foundation::EventSequence next_sequence = sequence_ + 1;
    DocumentEvent event{transaction_id, next_sequence, std::move(changes)};
    CommandOutcome outcome;
    outcome.status = CommandStatus::succeeded;
    outcome.transaction_id = transaction_id;
    outcome.events.push_back(event);
    outcome.revisions = std::move(revisions);
    outcome.changed = true;
    const std::vector<EventListener> listeners = listener_snapshot();

    document_.swap(next);
    retired_revisions_.swap(next_retired);
    sequence_ = next_sequence;
    publish(event, listeners);
    return outcome;
}

CommandOutcome CommandGateway::undo(std::stop_token stop_token) {
    if (publishing_ || executing_) {
        return make_failure(foundation::ErrorCode::reentrant_mutation,
                            "history cannot change from an event listener or handler");
    }
    if (stop_token.stop_requested()) {
        return make_canceled("undo canceled");
    }
    if (undo_stack_.empty()) {
        return make_failure(foundation::ErrorCode::nothing_to_undo, "there is no undo entry");
    }
    const UndoEntry& entry = undo_stack_.back();
    CommandOutcome outcome;
    try {
        outcome = apply_history_entry(entry.before);
    } catch (const std::exception& exception) {
        return make_failure(foundation::ErrorCode::internal_error,
                            "undo failed: " + std::string(exception.what()));
    } catch (...) {
        return make_failure(foundation::ErrorCode::internal_error,
                            "undo failed with an unknown exception");
    }
    if (outcome.succeeded()) {
        redo_stack_.splice(redo_stack_.end(), undo_stack_, std::prev(undo_stack_.end()));
    }
    return outcome;
}

CommandOutcome CommandGateway::redo(std::stop_token stop_token) {
    if (publishing_ || executing_) {
        return make_failure(foundation::ErrorCode::reentrant_mutation,
                            "history cannot change from an event listener or handler");
    }
    if (stop_token.stop_requested()) {
        return make_canceled("redo canceled");
    }
    if (redo_stack_.empty()) {
        return make_failure(foundation::ErrorCode::nothing_to_redo, "there is no redo entry");
    }
    const UndoEntry& entry = redo_stack_.back();
    CommandOutcome outcome;
    try {
        outcome = apply_history_entry(entry.after);
    } catch (const std::exception& exception) {
        return make_failure(foundation::ErrorCode::internal_error,
                            "redo failed: " + std::string(exception.what()));
    } catch (...) {
        return make_failure(foundation::ErrorCode::internal_error,
                            "redo failed with an unknown exception");
    }
    if (outcome.succeeded()) {
        undo_stack_.splice(undo_stack_.end(), redo_stack_, std::prev(redo_stack_.end()));
    }
    return outcome;
}

std::optional<CommandRequest> CommandGateway::recorded_command(std::string new_request_id) const {
    if (!last_action_.has_value()) {
        return std::nullopt;
    }
    const LastActionRecord& record = *last_action_;
    CommandRequest request;
    request.command_id = record.command_id;
    request.schema_version = record.schema_version;
    request.request_id = std::move(new_request_id);
    request.principal = record.principal;
    request.project_id = record.project_id;
    request.parent_workflow_id = record.parent_workflow_id;
    request.arguments = record.parameters.values();
    for (const auto& entry : record.original_expected_revisions.reads) {
        request.expected_revisions.reads.emplace(entry.first, current_revision(entry.first));
    }
    for (const auto& entry : record.original_expected_revisions.writes) {
        request.expected_revisions.writes.emplace(entry.first, current_revision(entry.first));
    }
    return request;
}

CommandOutcome CommandGateway::repeat_last(std::string new_request_id, std::stop_token stop_token) {
    if (publishing_ || executing_) {
        return make_failure(foundation::ErrorCode::reentrant_mutation,
                            "commands cannot be issued from an event listener or handler");
    }
    if (stop_token.stop_requested()) {
        return make_canceled("repeat canceled");
    }
    if (!last_action_.has_value()) {
        return make_failure(foundation::ErrorCode::not_repeatable,
                            "there is no last action to repeat");
    }
    if (last_action_->repeatability != Repeatability::repeatable) {
        return make_failure(foundation::ErrorCode::not_repeatable,
                            "command " + last_action_->command_id + " is not repeatable");
    }
    auto request = recorded_command(std::move(new_request_id));
    if (!request.has_value()) {
        return make_failure(foundation::ErrorCode::not_repeatable,
                            "there is no last action to repeat");
    }
    return execute(*request, stop_token);
}

DocumentSnapshot CommandGateway::snapshot() const { return DocumentSnapshot{document_}; }

std::optional<LastActionRecord> CommandGateway::last_action() const { return last_action_; }

foundation::EventSequence CommandGateway::last_sequence() const noexcept { return sequence_; }

std::size_t CommandGateway::undo_depth() const noexcept { return undo_stack_.size(); }

std::size_t CommandGateway::redo_depth() const noexcept { return redo_stack_.size(); }

const CommandRegistry& CommandGateway::registry() const noexcept { return registry_; }

SubscriptionId CommandGateway::subscribe(EventListener listener) {
    if (!listener) {
        return 0;
    }
    const SubscriptionId subscription_id = next_subscription_id_++;
    listeners_.emplace(subscription_id, std::move(listener));
    return subscription_id;
}

bool CommandGateway::unsubscribe(SubscriptionId subscription_id) {
    return listeners_.erase(subscription_id) > 0;
}

std::vector<EventListener> CommandGateway::listener_snapshot() const {
    std::vector<EventListener> listeners;
    listeners.reserve(listeners_.size());
    for (const auto& entry : listeners_) {
        listeners.push_back(entry.second);
    }
    return listeners;
}

void CommandGateway::publish(const DocumentEvent& event,
                             const std::vector<EventListener>& listeners) {
    struct PublishingGuard {
        explicit PublishingGuard(bool& flag) : flag_(flag) { flag_ = true; }
        ~PublishingGuard() { flag_ = false; }

        bool& flag_;
    };

    PublishingGuard guard{publishing_};
    for (const auto& listener : listeners) {
        try {
            listener(event);
        } catch (...) {
        }
    }
}

}  // namespace everygraph::application
