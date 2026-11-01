#include "fake_project.hpp"

#include <cctype>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace everygraph::desktop {
namespace {

using application::CommandDefinition;
using application::CommandRegistry;
using application::CommandRequest;
using application::CommandTransaction;
using application::EffectClass;
using application::ExecutionContext;
using application::ObjectState;
using application::ParameterMap;
using application::Repeatability;
using application::Value;
using foundation::Error;
using foundation::ErrorCode;
using foundation::ObjectId;

constexpr std::uint32_t kSchemaVersion = 1;

std::optional<Error> invalid_argument(std::string message) {
    return Error{ErrorCode::invalid_argument, std::move(message)};
}

std::optional<std::int64_t> integer_argument(const ParameterMap& arguments,
                                             std::string_view key) {
    const auto iterator = arguments.find(key);
    if (iterator == arguments.end()) {
        return std::nullopt;
    }
    const auto* value = std::get_if<std::int64_t>(&iterator->second);
    return value == nullptr ? std::nullopt
                            : std::optional<std::int64_t>{*value};
}

std::optional<std::string> string_argument(const ParameterMap& arguments,
                                           std::string_view key) {
    const auto iterator = arguments.find(key);
    if (iterator == arguments.end()) {
        return std::nullopt;
    }
    const auto* value = std::get_if<std::string>(&iterator->second);
    return value == nullptr ? std::nullopt : std::optional<std::string>{*value};
}

bool is_known_type(std::string_view type) {
    return type == "folder" || type == "dataset" || type == "graph" || type == "note";
}

ObjectId parent_of(const ObjectState& object) {
    const auto parent = object.properties.find("parent");
    if (parent == object.properties.end()) {
        return {};
    }
    const auto* value = std::get_if<std::int64_t>(&parent->second);
    if (value == nullptr || *value <= 0) {
        return {};
    }
    return ObjectId{static_cast<std::uint64_t>(*value)};
}

std::optional<Error> validate_create_arguments(const ParameterMap& arguments) {
    const auto parent = integer_argument(arguments, "parent_id");
    if (!parent.has_value() || *parent <= 0) {
        return invalid_argument("parent_id must be a positive integer");
    }
    const auto object = integer_argument(arguments, "object_id");
    if (!object.has_value() || *object <= 0) {
        return invalid_argument("object_id must be a positive integer");
    }
    const auto type = string_argument(arguments, "type");
    if (!type.has_value() || !is_known_type(*type)) {
        return invalid_argument("type must be one of folder, dataset, graph or note");
    }
    const auto name = string_argument(arguments, "name");
    if (!name.has_value() || name->empty()) {
        return invalid_argument("name must be a non-empty string");
    }
    return std::nullopt;
}

std::optional<Error> validate_move_arguments(const ParameterMap& arguments) {
    const auto object = integer_argument(arguments, "object_id");
    if (!object.has_value() || *object <= 0) {
        return invalid_argument("object_id must be a positive integer");
    }
    const auto parent = integer_argument(arguments, "new_parent_id");
    if (!parent.has_value() || *parent <= 0) {
        return invalid_argument("new_parent_id must be a positive integer");
    }
    return std::nullopt;
}

std::optional<Error> validate_find_arguments(const ParameterMap& arguments) {
    const auto object = integer_argument(arguments, "object_id");
    if (!object.has_value() || *object <= 0) {
        return invalid_argument("object_id must be a positive integer");
    }
    return std::nullopt;
}

CommandDefinition make_create_object_command() {
    CommandDefinition definition;
    definition.command_id = std::string(fake_commands::kCreateObject);
    definition.supported_schema_versions = {kSchemaVersion};
    definition.effect = EffectClass::reversible_document_write;
    definition.repeatability = Repeatability::not_repeatable;
    definition.validate_arguments = validate_create_arguments;
    definition.execute = [](CommandTransaction& transaction, const CommandRequest& request,
                            const ExecutionContext&) -> std::optional<Error> {
        const ObjectId parent_id{static_cast<std::uint64_t>(
            *integer_argument(request.arguments, "parent_id"))};
        const ObjectId object_id{static_cast<std::uint64_t>(
            *integer_argument(request.arguments, "object_id"))};
        const std::string type = *string_argument(request.arguments, "type");
        const std::string name = *string_argument(request.arguments, "name");
        const ObjectState* parent = transaction.find(parent_id);
        if (parent == nullptr) {
            return Error{ErrorCode::not_found,
                         "parent object " + foundation::to_string(parent_id) + " does not exist"};
        }
        if (parent->type != "folder") {
            return invalid_argument("parent object " + foundation::to_string(parent_id) +
                                    " is not a folder");
        }
        if (transaction.find(object_id) != nullptr) {
            return Error{ErrorCode::already_exists,
                         "object " + foundation::to_string(object_id) + " already exists"};
        }
        ParameterMap properties;
        properties.emplace("name", Value{name});
        properties.emplace("parent", Value{static_cast<std::int64_t>(parent_id.value)});
        return transaction.create_object(object_id, type, std::move(properties));
    };
    return definition;
}

CommandDefinition make_move_object_command() {
    CommandDefinition definition;
    definition.command_id = std::string(fake_commands::kMoveObject);
    definition.supported_schema_versions = {kSchemaVersion};
    definition.effect = EffectClass::reversible_document_write;
    definition.repeatability = Repeatability::not_repeatable;
    definition.validate_arguments = validate_move_arguments;
    definition.execute = [](CommandTransaction& transaction, const CommandRequest& request,
                            const ExecutionContext&) -> std::optional<Error> {
        const ObjectId object_id{static_cast<std::uint64_t>(
            *integer_argument(request.arguments, "object_id"))};
        const ObjectId new_parent_id{static_cast<std::uint64_t>(
            *integer_argument(request.arguments, "new_parent_id"))};
        if (transaction.find(object_id) == nullptr) {
            return Error{ErrorCode::not_found,
                         "object " + foundation::to_string(object_id) + " does not exist"};
        }
        const ObjectState* parent = transaction.find(new_parent_id);
        if (parent == nullptr) {
            return Error{ErrorCode::not_found,
                         "object " + foundation::to_string(new_parent_id) + " does not exist"};
        }
        if (parent->type != "folder") {
            return invalid_argument("object " + foundation::to_string(new_parent_id) +
                                    " is not a folder");
        }
        const std::size_t guard_limit = transaction.objects().size() + 1;
        ObjectId cursor = new_parent_id;
        for (std::size_t guard = 0; guard < guard_limit && cursor.is_valid(); ++guard) {
            if (cursor == object_id) {
                return invalid_argument("an object cannot move under itself or its descendants");
            }
            const ObjectState* current = transaction.find(cursor);
            if (current == nullptr) {
                break;
            }
            cursor = parent_of(*current);
        }
        return transaction.set_property(
            object_id, "parent", Value{static_cast<std::int64_t>(new_parent_id.value)});
    };
    return definition;
}

CommandDefinition make_find_object_command() {
    CommandDefinition definition;
    definition.command_id = std::string(fake_commands::kFindObject);
    definition.supported_schema_versions = {kSchemaVersion};
    definition.effect = EffectClass::read_only;
    definition.repeatability = Repeatability::repeatable;
    definition.validate_arguments = validate_find_arguments;
    definition.execute = [](CommandTransaction& transaction, const CommandRequest& request,
                            const ExecutionContext&) -> std::optional<Error> {
        const ObjectId object_id{static_cast<std::uint64_t>(
            *integer_argument(request.arguments, "object_id"))};
        if (transaction.find(object_id) == nullptr) {
            return Error{ErrorCode::not_found,
                         "object " + foundation::to_string(object_id) + " does not exist"};
        }
        return std::nullopt;
    };
    return definition;
}

void add_object(application::DocumentSnapshot& document, ObjectId id, std::string type,
                std::string name, ObjectId parent) {
    ObjectState state;
    state.id = id;
    state.revision = 1;
    state.type = std::move(type);
    state.properties.emplace("name", Value{std::move(name)});
    if (parent.is_valid()) {
        state.properties.emplace("parent", Value{static_cast<std::int64_t>(parent.value)});
    }
    document.objects.emplace(id, std::move(state));
}

}  // namespace

application::DocumentSnapshot make_fake_document() {
    application::DocumentSnapshot document;
    add_object(document, fake_ids::kRootFolder, "folder", "Demo Project", {});
    add_object(document, fake_ids::kExperimentsFolder, "folder", "Experiments",
               fake_ids::kRootFolder);
    add_object(document, fake_ids::kScratchFolder, "folder", "Scratch",
               fake_ids::kRootFolder);
    add_object(document, fake_ids::kSpectrumDataset, "dataset", "Spectrum A",
               fake_ids::kExperimentsFolder);
    add_object(document, fake_ids::kSpectrumGraph, "graph", "Spectrum A Plot",
               fake_ids::kExperimentsFolder);
    add_object(document, fake_ids::kReadmeNote, "note", "README", fake_ids::kRootFolder);
    return document;
}

application::CommandRegistry make_fake_registry() {
    CommandRegistry registry;
    std::vector<CommandDefinition> definitions;
    definitions.push_back(make_create_object_command());
    definitions.push_back(make_move_object_command());
    definitions.push_back(make_find_object_command());
    for (CommandDefinition& definition : definitions) {
        if (const auto error = registry.register_command(std::move(definition))) {
            throw std::runtime_error("fake command registration failed: " + error->message);
        }
    }
    return registry;
}

namespace {

[[nodiscard]] application::ValidatedDocument require_valid_document(
    application::DocumentSnapshot document) {
    auto validated = application::ValidatedDocument::from(std::move(document));
    if (const auto* error = std::get_if<Error>(&validated)) {
        throw std::runtime_error("fake document rejected: " + error->message);
    }
    return std::get<application::ValidatedDocument>(std::move(validated));
}

}  // namespace

FakeProject::FakeProject()
    : gateway_(make_fake_registry(), require_valid_document(make_fake_document())) {}

application::CommandGateway& FakeProject::gateway() noexcept {
    return gateway_;
}

const application::CommandGateway& FakeProject::gateway() const noexcept {
    return gateway_;
}

FakeProjectController::FakeProjectController(application::CommandGateway& gateway,
                                             FakeCommandDispatch dispatch)
    : gateway_(gateway), dispatch_(std::move(dispatch)) {
    for (const auto& entry : gateway_.snapshot().objects) {
        if (entry.first.value >= next_object_id_) {
            next_object_id_ = entry.first.value + 1;
        }
    }
}

application::CommandOutcome FakeProjectController::create_folder(ObjectId parent) {
    return create_object(parent, "folder");
}

application::CommandOutcome FakeProjectController::create_dataset(ObjectId parent) {
    return create_object(parent, "dataset");
}

application::CommandOutcome FakeProjectController::create_object(ObjectId parent,
                                                                 std::string_view type) {
    const application::DocumentSnapshot snapshot = gateway_.snapshot();
    const ObjectId object_id{next_object_id_++};
    ParameterMap arguments;
    arguments.emplace("parent_id", Value{static_cast<std::int64_t>(parent.value)});
    arguments.emplace("object_id", Value{static_cast<std::int64_t>(object_id.value)});
    arguments.emplace("type", Value{std::string(type)});
    arguments.emplace("name", Value{next_name(type, object_id)});
    application::ExpectedRevisions expected;
    if (const ObjectState* parent_state = snapshot.find(parent); parent_state != nullptr) {
        expected.reads.emplace(parent, parent_state->revision);
    }
    expected.writes.emplace(object_id, foundation::kInitialRevision);
    return dispatch_(fake_commands::kCreateObject, std::move(arguments), std::move(expected));
}

application::CommandOutcome FakeProjectController::move_object(ObjectId object) {
    const application::DocumentSnapshot snapshot = gateway_.snapshot();
    const ObjectId destination = object == fake_ids::kScratchFolder ? fake_ids::kRootFolder
                                                                   : fake_ids::kScratchFolder;
    ParameterMap arguments;
    arguments.emplace("object_id", Value{static_cast<std::int64_t>(object.value)});
    arguments.emplace("new_parent_id", Value{static_cast<std::int64_t>(destination.value)});
    application::ExpectedRevisions expected;
    if (const ObjectState* state = snapshot.find(object); state != nullptr) {
        expected.reads.emplace(object, state->revision);
        expected.writes.emplace(object, state->revision);
    }
    if (const ObjectState* destination_state = snapshot.find(destination);
        destination_state != nullptr) {
        expected.reads.emplace(destination, destination_state->revision);
    }
    return dispatch_(fake_commands::kMoveObject, std::move(arguments), std::move(expected));
}

application::CommandOutcome FakeProjectController::find_object(ObjectId object) {
    const application::DocumentSnapshot snapshot = gateway_.snapshot();
    ParameterMap arguments;
    arguments.emplace("object_id", Value{static_cast<std::int64_t>(object.value)});
    application::ExpectedRevisions expected;
    if (const ObjectState* state = snapshot.find(object); state != nullptr) {
        expected.reads.emplace(object, state->revision);
    }
    return dispatch_(fake_commands::kFindObject, std::move(arguments), std::move(expected));
}

std::string FakeProjectController::next_name(std::string_view type, ObjectId id) const {
    std::string label(type);
    if (!label.empty()) {
        label.front() = static_cast<char>(
            std::toupper(static_cast<unsigned char>(label.front())));
    }
    return label + " " + std::to_string(id.value);
}

}  // namespace everygraph::desktop
