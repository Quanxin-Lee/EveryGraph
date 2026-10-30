#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <functional>
#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "everygraph/application/command_gateway.hpp"
#include "everygraph/application/command_registry.hpp"
#include "everygraph/application/document.hpp"
#include "everygraph/application/event.hpp"
#include "everygraph/application/value.hpp"
#include "everygraph/foundation/error.hpp"
#include "everygraph/foundation/ids.hpp"

namespace {

using everygraph::application::CommandDefinition;
using everygraph::application::CommandGateway;
using everygraph::application::CommandOutcome;
using everygraph::application::CommandRegistry;
using everygraph::application::CommandRequest;
using everygraph::application::CommandStatus;
using everygraph::application::CommandTransaction;
using everygraph::application::DocumentEvent;
using everygraph::application::DocumentSnapshot;
using everygraph::application::EffectClass;
using everygraph::application::ExecutionContext;
using everygraph::application::ObjectState;
using everygraph::application::ParameterMap;
using everygraph::application::Repeatability;
using everygraph::application::Value;
using everygraph::foundation::Error;
using everygraph::foundation::ErrorCode;
using everygraph::foundation::EventSequence;
using everygraph::foundation::ObjectId;
using everygraph::foundation::Revision;

constexpr std::uint32_t kSchemaVersion = 1;

struct TestFailure : std::runtime_error {
    using std::runtime_error::runtime_error;
};

using TestFunction = std::function<void()>;

struct TestCase {
    std::string name;
    TestFunction function;
};

std::vector<TestCase>& test_cases() {
    static std::vector<TestCase> cases;
    return cases;
}

struct TestRegistrar {
    TestRegistrar(std::string name, TestFunction function) {
        test_cases().push_back(TestCase{std::move(name), std::move(function)});
    }
};

#define EG_TEST(name)                                                    \
    static void name();                                                  \
    [[maybe_unused]] static const TestRegistrar name##_registrar(#name, name); \
    static void name()

template <typename Actual, typename Expected>
void require_equal(const Actual& actual, const Expected& expected, const char* actual_expression,
                   const char* expected_expression, const char* file, int line) {
    if (!(actual == expected)) {
        std::ostringstream stream;
        stream << file << ':' << line << ": expected " << actual_expression << " == "
               << expected_expression;
        throw TestFailure(stream.str());
    }
}

#define EG_REQUIRE(expression)                                                    \
    do {                                                                          \
        if (!(expression)) {                                                      \
            std::ostringstream stream;                                            \
            stream << __FILE__ << ':' << __LINE__ << ": requirement failed: " #expression; \
            throw TestFailure(stream.str());                                      \
        }                                                                         \
    } while (false)

#define EG_REQUIRE_EQ(actual, expected) \
    require_equal((actual), (expected), #actual, #expected, __FILE__, __LINE__)

void register_command(CommandRegistry& registry, CommandDefinition definition) {
    const auto error = registry.register_command(std::move(definition));
    if (error.has_value()) {
        throw TestFailure("registration failed: " + error->message);
    }
}

std::optional<Error> validate_positive_int64(const ParameterMap& arguments,
                                             std::string_view key) {
    const auto iterator = arguments.find(key);
    if (iterator == arguments.end()) {
        return Error{ErrorCode::invalid_argument, "missing argument " + std::string(key)};
    }
    const auto* value = std::get_if<std::int64_t>(&iterator->second);
    if (value == nullptr || *value <= 0) {
        return Error{ErrorCode::invalid_argument,
                     "argument " + std::string(key) + " must be a positive integer"};
    }
    return std::nullopt;
}

std::optional<Error> validate_int64(const ParameterMap& arguments, std::string_view key) {
    const auto iterator = arguments.find(key);
    if (iterator == arguments.end()) {
        return Error{ErrorCode::invalid_argument, "missing argument " + std::string(key)};
    }
    if (!std::holds_alternative<std::int64_t>(iterator->second)) {
        return Error{ErrorCode::invalid_argument,
                     "argument " + std::string(key) + " must be an integer"};
    }
    return std::nullopt;
}

std::optional<Error> validate_object_and_value(const ParameterMap& arguments) {
    if (const auto error = validate_positive_int64(arguments, "object")) {
        return error;
    }
    return validate_int64(arguments, "value");
}

std::optional<Error> validate_double(const ParameterMap& arguments, std::string_view key) {
    const auto iterator = arguments.find(key);
    if (iterator == arguments.end()) {
        return Error{ErrorCode::invalid_argument, "missing argument " + std::string(key)};
    }
    if (!std::holds_alternative<double>(iterator->second)) {
        return Error{ErrorCode::invalid_argument,
                     "argument " + std::string(key) + " must be a double"};
    }
    return std::nullopt;
}

std::optional<Error> validate_object_and_double(const ParameterMap& arguments) {
    if (const auto error = validate_positive_int64(arguments, "object")) {
        return error;
    }
    return validate_double(arguments, "value");
}

std::optional<Error> validate_pair(const ParameterMap& arguments) {
    if (const auto error = validate_positive_int64(arguments, "first")) {
        return error;
    }
    if (const auto error = validate_positive_int64(arguments, "second")) {
        return error;
    }
    if (const auto error = validate_int64(arguments, "first_value")) {
        return error;
    }
    return validate_int64(arguments, "second_value");
}

std::optional<Error> validate_triple(const ParameterMap& arguments) {
    if (const auto error = validate_positive_int64(arguments, "first")) {
        return error;
    }
    if (const auto error = validate_positive_int64(arguments, "second")) {
        return error;
    }
    if (const auto error = validate_positive_int64(arguments, "created")) {
        return error;
    }
    return validate_int64(arguments, "value");
}

std::int64_t int64_argument(const ParameterMap& arguments, std::string_view key) {
    const auto iterator = arguments.find(key);
    if (iterator == arguments.end()) {
        throw TestFailure("missing argument " + std::string(key));
    }
    const auto* value = std::get_if<std::int64_t>(&iterator->second);
    if (value == nullptr) {
        throw TestFailure("argument " + std::string(key) + " is not an integer");
    }
    return *value;
}

double double_argument(const ParameterMap& arguments, std::string_view key) {
    const auto iterator = arguments.find(key);
    if (iterator == arguments.end()) {
        throw TestFailure("missing argument " + std::string(key));
    }
    const auto* value = std::get_if<double>(&iterator->second);
    if (value == nullptr) {
        throw TestFailure("argument " + std::string(key) + " is not a double");
    }
    return *value;
}

ObjectId object_argument(const ParameterMap& arguments, std::string_view key) {
    return ObjectId{static_cast<std::uint64_t>(int64_argument(arguments, key))};
}

CommandDefinition make_set_value_definition() {
    CommandDefinition definition;
    definition.command_id = "demo.set_value";
    definition.supported_schema_versions = {kSchemaVersion};
    definition.effect = EffectClass::reversible_document_write;
    definition.repeatability = Repeatability::repeatable;
    definition.validate_arguments = [](const ParameterMap& arguments) {
        return validate_object_and_value(arguments);
    };
    definition.execute = [](CommandTransaction& transaction, const CommandRequest& request,
                            const ExecutionContext&) -> std::optional<Error> {
        return transaction.set_property(object_argument(request.arguments, "object"), "value",
                                        Value{int64_argument(request.arguments, "value")});
    };
    return definition;
}

CommandDefinition make_set_double_definition() {
    CommandDefinition definition;
    definition.command_id = "demo.set_double";
    definition.supported_schema_versions = {kSchemaVersion};
    definition.effect = EffectClass::reversible_document_write;
    definition.repeatability = Repeatability::repeatable;
    definition.validate_arguments = [](const ParameterMap& arguments) {
        return validate_object_and_double(arguments);
    };
    definition.execute = [](CommandTransaction& transaction, const CommandRequest& request,
                            const ExecutionContext&) -> std::optional<Error> {
        return transaction.set_property(object_argument(request.arguments, "object"), "value",
                                        Value{double_argument(request.arguments, "value")});
    };
    return definition;
}

CommandDefinition make_set_pair_definition() {
    CommandDefinition definition;
    definition.command_id = "demo.set_pair";
    definition.supported_schema_versions = {kSchemaVersion};
    definition.effect = EffectClass::reversible_document_write;
    definition.repeatability = Repeatability::repeatable;
    definition.validate_arguments = [](const ParameterMap& arguments) {
        return validate_pair(arguments);
    };
    definition.execute = [](CommandTransaction& transaction, const CommandRequest& request,
                            const ExecutionContext&) -> std::optional<Error> {
        if (const auto error =
                transaction.set_property(object_argument(request.arguments, "first"), "value",
                                         Value{int64_argument(request.arguments, "first_value")})) {
            return error;
        }
        return transaction.set_property(object_argument(request.arguments, "second"), "value",
                                        Value{int64_argument(request.arguments, "second_value")});
    };
    return definition;
}

CommandDefinition make_edit_pair_definition() {
    CommandDefinition definition;
    definition.command_id = "demo.edit_pair";
    definition.supported_schema_versions = {kSchemaVersion};
    definition.effect = EffectClass::reversible_document_write;
    definition.repeatability = Repeatability::not_repeatable;
    definition.validate_arguments = [](const ParameterMap& arguments) {
        return validate_triple(arguments);
    };
    definition.execute = [](CommandTransaction& transaction, const CommandRequest& request,
                            const ExecutionContext&) -> std::optional<Error> {
        if (const auto error =
                transaction.set_property(object_argument(request.arguments, "first"), "value",
                                         Value{int64_argument(request.arguments, "value")})) {
            return error;
        }
        if (const auto error =
                transaction.set_property(object_argument(request.arguments, "second"), "value",
                                         Value{int64_argument(request.arguments, "value")})) {
            return error;
        }
        ParameterMap properties;
        properties.emplace("value", Value{int64_argument(request.arguments, "value")});
        return transaction.create_object(object_argument(request.arguments, "created"),
                                         "fake.result", std::move(properties));
    };
    return definition;
}

CommandDefinition make_fail_after_stage_definition() {
    CommandDefinition definition;
    definition.command_id = "demo.fail_after_stage";
    definition.supported_schema_versions = {kSchemaVersion};
    definition.effect = EffectClass::reversible_document_write;
    definition.repeatability = Repeatability::not_repeatable;
    definition.validate_arguments = [](const ParameterMap& arguments) {
        return validate_object_and_value(arguments);
    };
    definition.execute = [](CommandTransaction& transaction, const CommandRequest& request,
                            const ExecutionContext&) -> std::optional<Error> {
        if (const auto error =
                transaction.set_property(object_argument(request.arguments, "object"), "value",
                                         Value{int64_argument(request.arguments, "value")})) {
            return error;
        }
        return Error{ErrorCode::numerical_failure, "candidate rejected"};
    };
    return definition;
}

CommandRegistry make_registry() {
    CommandRegistry registry;
    register_command(registry, make_set_value_definition());
    register_command(registry, make_set_pair_definition());
    register_command(registry, make_edit_pair_definition());
    register_command(registry, make_fail_after_stage_definition());
    return registry;
}

ObjectState make_object(std::uint64_t id, std::uint64_t revision, std::int64_t value) {
    ObjectState object;
    object.id = ObjectId{id};
    object.revision = revision;
    object.type = "fake.item";
    object.properties.emplace("value", Value{value});
    return object;
}

ObjectState make_double_object(std::uint64_t id, std::uint64_t revision, double value) {
    ObjectState object;
    object.id = ObjectId{id};
    object.revision = revision;
    object.type = "fake.item";
    object.properties.emplace("value", Value{value});
    return object;
}

DocumentSnapshot make_document() {
    DocumentSnapshot document;
    const ObjectState first = make_object(1, 1, 10);
    const ObjectState second = make_object(2, 1, 20);
    document.objects.emplace(first.id, first);
    document.objects.emplace(second.id, second);
    return document;
}

CommandRequest make_request(std::string command_id, std::string request_id) {
    CommandRequest request;
    request.command_id = std::move(command_id);
    request.schema_version = kSchemaVersion;
    request.request_id = std::move(request_id);
    request.principal = "test-principal";
    request.project_id = "test-project";
    return request;
}

void expect_write(CommandRequest& request, std::uint64_t id, Revision revision) {
    request.expected_revisions.writes.emplace(ObjectId{id}, revision);
}

void expect_read(CommandRequest& request, std::uint64_t id, Revision revision) {
    request.expected_revisions.reads.emplace(ObjectId{id}, revision);
}

void set_object_argument(CommandRequest& request, std::uint64_t id) {
    request.arguments.emplace("object", Value{static_cast<std::int64_t>(id)});
}

void set_value_argument(CommandRequest& request, std::int64_t value) {
    request.arguments.emplace("value", Value{value});
}

void set_pair_arguments(CommandRequest& request, std::uint64_t first, std::uint64_t second,
                        std::int64_t first_value, std::int64_t second_value) {
    request.arguments.emplace("first", Value{static_cast<std::int64_t>(first)});
    request.arguments.emplace("second", Value{static_cast<std::int64_t>(second)});
    request.arguments.emplace("first_value", Value{first_value});
    request.arguments.emplace("second_value", Value{second_value});
}

void set_triple_arguments(CommandRequest& request, std::uint64_t first, std::uint64_t second,
                          std::uint64_t created, std::int64_t value) {
    request.arguments.emplace("first", Value{static_cast<std::int64_t>(first)});
    request.arguments.emplace("second", Value{static_cast<std::int64_t>(second)});
    request.arguments.emplace("created", Value{static_cast<std::int64_t>(created)});
    request.arguments.emplace("value", Value{value});
}

const ObjectState& require_object(const DocumentSnapshot& snapshot, std::uint64_t id) {
    const ObjectState* object = snapshot.find(ObjectId{id});
    if (object == nullptr) {
        throw TestFailure("object " + std::to_string(id) + " is missing");
    }
    return *object;
}

std::int64_t object_value(const DocumentSnapshot& snapshot, std::uint64_t id) {
    const ObjectState& object = require_object(snapshot, id);
    const auto iterator = object.properties.find("value");
    if (iterator == object.properties.end()) {
        throw TestFailure("object " + std::to_string(id) + " has no value property");
    }
    const auto* value = std::get_if<std::int64_t>(&iterator->second);
    if (value == nullptr) {
        throw TestFailure("object " + std::to_string(id) + " has a non-integer value");
    }
    return *value;
}

double double_value(const DocumentSnapshot& snapshot, std::uint64_t id) {
    const ObjectState& object = require_object(snapshot, id);
    const auto iterator = object.properties.find("value");
    if (iterator == object.properties.end()) {
        throw TestFailure("object " + std::to_string(id) + " has no value property");
    }
    const auto* value = std::get_if<double>(&iterator->second);
    if (value == nullptr) {
        throw TestFailure("object " + std::to_string(id) + " has a non-double value");
    }
    return *value;
}

Revision object_revision(const DocumentSnapshot& snapshot, std::uint64_t id) {
    return require_object(snapshot, id).revision;
}

void require_error(const CommandOutcome& outcome, ErrorCode code) {
    EG_REQUIRE(!outcome.succeeded());
    EG_REQUIRE(outcome.error.has_value());
    EG_REQUIRE_EQ(outcome.error->code, code);
}

EG_TEST(registry_rejects_unknown_command) {
    CommandGateway gateway(make_registry(), make_document());
    const auto outcome = gateway.execute(make_request("demo.unknown", "req-unknown"));
    require_error(outcome, ErrorCode::unknown_command);
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{0});
    EG_REQUIRE_EQ(gateway.last_sequence(), EventSequence{0});
}

EG_TEST(registry_rejects_duplicate_registration) {
    CommandRegistry registry;
    register_command(registry, make_set_value_definition());
    const auto error = registry.register_command(make_set_value_definition());
    EG_REQUIRE(error.has_value());
    EG_REQUIRE_EQ(error->code, ErrorCode::duplicate_command);
}

EG_TEST(registry_rejects_invalid_command_id) {
    CommandRegistry registry;
    auto definition = make_set_value_definition();
    definition.command_id = "Invalid";
    const auto error = registry.register_command(std::move(definition));
    EG_REQUIRE(error.has_value());
    EG_REQUIRE_EQ(error->code, ErrorCode::invalid_argument);
}

EG_TEST(registry_rejects_external_effects) {
    CommandRegistry registry;
    auto definition = make_set_value_definition();
    definition.command_id = "demo.external";
    definition.effect = EffectClass::external_write;
    const auto error = registry.register_command(std::move(definition));
    EG_REQUIRE(error.has_value());
    EG_REQUIRE_EQ(error->code, ErrorCode::unsupported_feature);
}

EG_TEST(registry_rejects_unsupported_schema_version) {
    CommandGateway gateway(make_registry(), make_document());
    auto request = make_request("demo.set_value", "req-version");
    request.schema_version = 2;
    const auto outcome = gateway.execute(request);
    require_error(outcome, ErrorCode::unsupported_schema_version);
}

EG_TEST(registry_rejects_malformed_arguments) {
    CommandGateway gateway(make_registry(), make_document());
    auto request = make_request("demo.set_value", "req-arguments");
    set_object_argument(request, 1);
    expect_write(request, 1, 1);
    const auto outcome = gateway.execute(request);
    require_error(outcome, ErrorCode::invalid_argument);
    EG_REQUIRE_EQ(gateway.last_sequence(), EventSequence{0});
}

EG_TEST(gateway_rejects_stale_write_revision) {
    CommandGateway gateway(make_registry(), make_document());
    auto request = make_request("demo.set_value", "req-stale-write");
    set_object_argument(request, 1);
    set_value_argument(request, 99);
    expect_write(request, 1, 0);
    const auto outcome = gateway.execute(request);
    require_error(outcome, ErrorCode::revision_conflict);
    EG_REQUIRE_EQ(object_value(gateway.snapshot(), 1), std::int64_t{10});
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{0});
    EG_REQUIRE_EQ(gateway.last_sequence(), EventSequence{0});
    EG_REQUIRE(!gateway.last_action().has_value());
}

EG_TEST(gateway_rejects_stale_read_revision) {
    CommandGateway gateway(make_registry(), make_document());
    auto request = make_request("demo.set_value", "req-stale-read");
    set_object_argument(request, 1);
    set_value_argument(request, 99);
    expect_write(request, 1, 1);
    expect_read(request, 2, 0);
    const auto outcome = gateway.execute(request);
    require_error(outcome, ErrorCode::revision_conflict);
    EG_REQUIRE_EQ(object_value(gateway.snapshot(), 1), std::int64_t{10});
    EG_REQUIRE_EQ(gateway.last_sequence(), EventSequence{0});
    EG_REQUIRE(!gateway.last_action().has_value());
}

EG_TEST(transaction_commits_multi_object_edit_once) {
    CommandGateway gateway(make_registry(), make_document());
    std::vector<EventSequence> sequences;
    const auto subscription = gateway.subscribe([&sequences](const DocumentEvent& event) {
        sequences.push_back(event.sequence);
    });
    EG_REQUIRE(subscription != 0);

    auto request = make_request("demo.edit_pair", "req-edit-pair");
    set_triple_arguments(request, 1, 2, 3, 99);
    expect_write(request, 1, 1);
    expect_write(request, 2, 1);
    expect_write(request, 3, 0);
    const auto outcome = gateway.execute(request);

    EG_REQUIRE(outcome.succeeded());
    EG_REQUIRE(outcome.changed);
    EG_REQUIRE(outcome.transaction_id.has_value());
    EG_REQUIRE_EQ(outcome.events.size(), std::size_t{1});
    EG_REQUIRE_EQ(outcome.events.front().changes.size(), std::size_t{3});
    EG_REQUIRE_EQ(outcome.events.front().sequence, EventSequence{1});
    EG_REQUIRE_EQ(outcome.events.front().transaction_id, *outcome.transaction_id);
    EG_REQUIRE_EQ(gateway.last_sequence(), EventSequence{1});
    EG_REQUIRE_EQ(sequences.size(), std::size_t{1});
    EG_REQUIRE_EQ(sequences.front(), EventSequence{1});

    const auto snapshot = gateway.snapshot();
    EG_REQUIRE_EQ(object_value(snapshot, 1), std::int64_t{99});
    EG_REQUIRE_EQ(object_value(snapshot, 2), std::int64_t{99});
    EG_REQUIRE_EQ(object_value(snapshot, 3), std::int64_t{99});
    EG_REQUIRE_EQ(object_revision(snapshot, 1), Revision{2});
    EG_REQUIRE_EQ(object_revision(snapshot, 2), Revision{2});
    EG_REQUIRE_EQ(object_revision(snapshot, 3), Revision{1});
    EG_REQUIRE_EQ(require_object(snapshot, 3).type, std::string("fake.result"));
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{1});
    EG_REQUIRE_EQ(gateway.redo_depth(), std::size_t{0});

    const auto last = gateway.last_action();
    EG_REQUIRE(last.has_value());
    EG_REQUIRE_EQ(last->command_id, std::string("demo.edit_pair"));
    EG_REQUIRE_EQ(last->repeatability, Repeatability::not_repeatable);
}

EG_TEST(undo_and_redo_restore_state_with_new_revisions) {
    CommandGateway gateway(make_registry(), make_document());
    auto request = make_request("demo.edit_pair", "req-history-edit");
    set_triple_arguments(request, 1, 2, 3, 99);
    expect_write(request, 1, 1);
    expect_write(request, 2, 1);
    expect_write(request, 3, 0);
    EG_REQUIRE(gateway.execute(request).succeeded());

    const auto undo = gateway.undo();
    EG_REQUIRE(undo.succeeded());
    EG_REQUIRE(undo.changed);
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{0});
    EG_REQUIRE_EQ(gateway.redo_depth(), std::size_t{1});
    EG_REQUIRE_EQ(gateway.last_sequence(), EventSequence{2});

    auto snapshot = gateway.snapshot();
    EG_REQUIRE_EQ(object_value(snapshot, 1), std::int64_t{10});
    EG_REQUIRE_EQ(object_value(snapshot, 2), std::int64_t{20});
    EG_REQUIRE(snapshot.find(ObjectId{3}) == nullptr);
    EG_REQUIRE_EQ(object_revision(snapshot, 1), Revision{3});
    EG_REQUIRE_EQ(object_revision(snapshot, 2), Revision{3});

    const auto redo = gateway.redo();
    EG_REQUIRE(redo.succeeded());
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{1});
    EG_REQUIRE_EQ(gateway.redo_depth(), std::size_t{0});
    EG_REQUIRE_EQ(gateway.last_sequence(), EventSequence{3});

    snapshot = gateway.snapshot();
    EG_REQUIRE_EQ(object_value(snapshot, 1), std::int64_t{99});
    EG_REQUIRE_EQ(object_value(snapshot, 2), std::int64_t{99});
    EG_REQUIRE_EQ(object_value(snapshot, 3), std::int64_t{99});
    EG_REQUIRE_EQ(object_revision(snapshot, 1), Revision{4});
    EG_REQUIRE_EQ(object_revision(snapshot, 2), Revision{4});
    EG_REQUIRE_EQ(object_revision(snapshot, 3), Revision{2});
}

EG_TEST(history_reports_empty_stacks_and_new_edit_clears_redo) {
    CommandGateway gateway(make_registry(), make_document());
    require_error(gateway.undo(), ErrorCode::nothing_to_undo);
    require_error(gateway.redo(), ErrorCode::nothing_to_redo);

    auto first = make_request("demo.set_value", "req-history-first");
    set_object_argument(first, 1);
    set_value_argument(first, 11);
    expect_write(first, 1, 1);
    EG_REQUIRE(gateway.execute(first).succeeded());
    EG_REQUIRE(gateway.undo().succeeded());
    EG_REQUIRE_EQ(gateway.redo_depth(), std::size_t{1});

    auto second = make_request("demo.set_value", "req-history-second");
    set_object_argument(second, 1);
    set_value_argument(second, 12);
    expect_write(second, 1, 3);
    EG_REQUIRE(gateway.execute(second).succeeded());
    EG_REQUIRE_EQ(gateway.redo_depth(), std::size_t{0});
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{1});
    EG_REQUIRE_EQ(object_value(gateway.snapshot(), 1), std::int64_t{12});
}

EG_TEST(canceled_command_publishes_nothing) {
    int handler_calls = 0;
    CommandRegistry registry;
    auto definition = make_set_value_definition();
    definition.execute = [&handler_calls](CommandTransaction&, const CommandRequest&,
                                          const ExecutionContext&) -> std::optional<Error> {
        ++handler_calls;
        return std::nullopt;
    };
    register_command(registry, std::move(definition));
    CommandGateway gateway(std::move(registry), make_document());
    int notifications = 0;
    gateway.subscribe([&notifications](const DocumentEvent&) { ++notifications; });

    std::stop_source source;
    (void)source.request_stop();
    auto request = make_request("demo.set_value", "req-canceled");
    set_object_argument(request, 1);
    set_value_argument(request, 99);
    expect_write(request, 1, 1);
    const auto outcome = gateway.execute(request, source.get_token());

    EG_REQUIRE_EQ(outcome.status, CommandStatus::canceled);
    require_error(outcome, ErrorCode::canceled);
    EG_REQUIRE_EQ(handler_calls, 0);
    EG_REQUIRE_EQ(notifications, 0);
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{0});
    EG_REQUIRE_EQ(gateway.last_sequence(), EventSequence{0});
    EG_REQUIRE_EQ(object_value(gateway.snapshot(), 1), std::int64_t{10});
}

EG_TEST(canceled_handler_discards_staged_candidate) {
    std::stop_source source;
    CommandRegistry registry;
    auto definition = make_set_value_definition();
    definition.execute = [&source](CommandTransaction& transaction, const CommandRequest& request,
                                   const ExecutionContext& context) -> std::optional<Error> {
        if (const auto error =
                transaction.set_property(object_argument(request.arguments, "object"), "value",
                                         Value{int64_argument(request.arguments, "value")})) {
            return error;
        }
        (void)source.request_stop();
        return context.check_canceled();
    };
    register_command(registry, std::move(definition));
    CommandGateway gateway(std::move(registry), make_document());
    int notifications = 0;
    gateway.subscribe([&notifications](const DocumentEvent&) { ++notifications; });

    auto request = make_request("demo.set_value", "req-cancel-mid");
    set_object_argument(request, 1);
    set_value_argument(request, 99);
    expect_write(request, 1, 1);
    const auto outcome = gateway.execute(request, source.get_token());

    EG_REQUIRE_EQ(outcome.status, CommandStatus::canceled);
    require_error(outcome, ErrorCode::canceled);
    EG_REQUIRE_EQ(notifications, 0);
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{0});
    EG_REQUIRE_EQ(gateway.last_sequence(), EventSequence{0});
    EG_REQUIRE_EQ(object_value(gateway.snapshot(), 1), std::int64_t{10});
}

EG_TEST(rejected_candidate_publishes_nothing) {
    CommandGateway gateway(make_registry(), make_document());
    int notifications = 0;
    gateway.subscribe([&notifications](const DocumentEvent&) { ++notifications; });

    auto request = make_request("demo.fail_after_stage", "req-rejected");
    set_object_argument(request, 1);
    set_value_argument(request, 77);
    expect_write(request, 1, 1);
    const auto outcome = gateway.execute(request);

    require_error(outcome, ErrorCode::numerical_failure);
    EG_REQUIRE_EQ(notifications, 0);
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{0});
    EG_REQUIRE_EQ(gateway.last_sequence(), EventSequence{0});
    EG_REQUIRE_EQ(object_value(gateway.snapshot(), 1), std::int64_t{10});
}

EG_TEST(handler_exception_is_reported_without_publication) {
    CommandRegistry registry;
    auto definition = make_set_value_definition();
    definition.command_id = "demo.throw";
    definition.execute = [](CommandTransaction&, const CommandRequest&,
                            const ExecutionContext&) -> std::optional<Error> {
        throw std::runtime_error("handler failure");
    };
    register_command(registry, std::move(definition));
    CommandGateway gateway(std::move(registry), make_document());

    auto request = make_request("demo.throw", "req-handler-throw");
    set_object_argument(request, 1);
    set_value_argument(request, 99);
    expect_write(request, 1, 1);
    const auto outcome = gateway.execute(request);

    require_error(outcome, ErrorCode::internal_error);
    EG_REQUIRE_EQ(object_value(gateway.snapshot(), 1), std::int64_t{10});
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{0});
    EG_REQUIRE_EQ(gateway.last_sequence(), EventSequence{0});
}

EG_TEST(listener_exception_is_contained) {
    CommandGateway gateway(make_registry(), make_document());
    int healthy_calls = 0;
    gateway.subscribe([](const DocumentEvent&) { throw std::runtime_error("listener failure"); });
    gateway.subscribe([&healthy_calls](const DocumentEvent&) { ++healthy_calls; });

    auto request = make_request("demo.set_value", "req-listener-throw");
    set_object_argument(request, 1);
    set_value_argument(request, 11);
    expect_write(request, 1, 1);
    const auto outcome = gateway.execute(request);

    EG_REQUIRE(outcome.succeeded());
    EG_REQUIRE_EQ(healthy_calls, 1);
    EG_REQUIRE_EQ(object_value(gateway.snapshot(), 1), std::int64_t{11});
    EG_REQUIRE_EQ(gateway.last_sequence(), EventSequence{1});
}

EG_TEST(stop_after_handler_prevents_commit) {
    std::stop_source source;
    CommandRegistry registry;
    auto definition = make_set_value_definition();
    definition.execute = [&source](CommandTransaction& transaction, const CommandRequest& request,
                                   const ExecutionContext&) -> std::optional<Error> {
        if (const auto error =
                transaction.set_property(object_argument(request.arguments, "object"), "value",
                                         Value{int64_argument(request.arguments, "value")})) {
            return error;
        }
        (void)source.request_stop();
        return std::nullopt;
    };
    register_command(registry, std::move(definition));
    CommandGateway gateway(std::move(registry), make_document());

    auto request = make_request("demo.set_value", "req-stop-after-handler");
    set_object_argument(request, 1);
    set_value_argument(request, 99);
    expect_write(request, 1, 1);
    const auto outcome = gateway.execute(request, source.get_token());

    EG_REQUIRE_EQ(outcome.status, CommandStatus::canceled);
    require_error(outcome, ErrorCode::canceled);
    EG_REQUIRE_EQ(object_value(gateway.snapshot(), 1), std::int64_t{10});
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{0});
    EG_REQUIRE_EQ(gateway.last_sequence(), EventSequence{0});
}

EG_TEST(undeclared_writes_are_rejected) {
    CommandGateway gateway(make_registry(), make_document());
    auto request = make_request("demo.set_value", "req-undeclared");
    set_object_argument(request, 1);
    set_value_argument(request, 99);
    expect_write(request, 2, 1);
    const auto outcome = gateway.execute(request);

    require_error(outcome, ErrorCode::invalid_argument);
    EG_REQUIRE_EQ(object_value(gateway.snapshot(), 1), std::int64_t{10});
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{0});
    EG_REQUIRE_EQ(gateway.last_sequence(), EventSequence{0});
}

EG_TEST(handler_reentrancy_cannot_commit_stale_candidate) {
    std::optional<CommandOutcome> inner_outcome;
    CommandGateway* gateway_pointer = nullptr;
    CommandRegistry registry;
    auto definition = make_set_value_definition();
    definition.command_id = "demo.outer";
    definition.validate_arguments = [](const ParameterMap&) -> std::optional<Error> {
        return std::nullopt;
    };
    definition.execute = [&gateway_pointer, &inner_outcome](CommandTransaction& transaction,
                                                            const CommandRequest&,
                                                            const ExecutionContext&)
        -> std::optional<Error> {
        auto inner = make_request("demo.set_value", "req-inner");
        set_object_argument(inner, 1);
        set_value_argument(inner, 99);
        expect_write(inner, 1, 1);
        inner_outcome = gateway_pointer->execute(inner);
        return transaction.set_property(ObjectId{1}, "value", Value{std::int64_t{50}});
    };
    register_command(registry, std::move(definition));
    register_command(registry, make_set_value_definition());
    CommandGateway gateway(std::move(registry), make_document());
    gateway_pointer = &gateway;

    auto outer = make_request("demo.outer", "req-outer");
    expect_write(outer, 1, 1);
    const auto outcome = gateway.execute(outer);

    EG_REQUIRE(outcome.succeeded());
    EG_REQUIRE(inner_outcome.has_value());
    require_error(*inner_outcome, ErrorCode::reentrant_mutation);
    EG_REQUIRE_EQ(object_value(gateway.snapshot(), 1), std::int64_t{50});
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{1});
    EG_REQUIRE_EQ(gateway.last_sequence(), EventSequence{1});
}

EG_TEST(double_no_op_detection_handles_nan_and_signed_zero) {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    DocumentSnapshot document;
    const ObjectState first = make_double_object(1, 1, nan);
    const ObjectState second = make_double_object(2, 1, 0.0);
    document.objects.emplace(first.id, first);
    document.objects.emplace(second.id, second);

    CommandRegistry registry;
    register_command(registry, make_set_double_definition());
    CommandGateway gateway(std::move(registry), std::move(document));

    auto nan_request = make_request("demo.set_double", "req-nan");
    set_object_argument(nan_request, 1);
    nan_request.arguments.emplace("value", Value{nan});
    expect_write(nan_request, 1, 1);
    const auto nan_outcome = gateway.execute(nan_request);
    EG_REQUIRE(nan_outcome.succeeded());
    EG_REQUIRE(!nan_outcome.changed);
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{0});
    EG_REQUIRE_EQ(gateway.last_sequence(), EventSequence{0});

    auto zero_request = make_request("demo.set_double", "req-zero");
    set_object_argument(zero_request, 2);
    zero_request.arguments.emplace("value", Value{-0.0});
    expect_write(zero_request, 2, 1);
    const auto zero_outcome = gateway.execute(zero_request);
    EG_REQUIRE(zero_outcome.succeeded());
    EG_REQUIRE(zero_outcome.changed);
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{1});
    EG_REQUIRE_EQ(gateway.last_sequence(), EventSequence{1});
    const double zero = double_value(gateway.snapshot(), 2);
    EG_REQUIRE(zero == 0.0);
    EG_REQUIRE(std::signbit(zero));
}

EG_TEST(read_only_effect_cannot_mutate_document) {
    CommandRegistry registry;
    auto definition = make_set_value_definition();
    definition.command_id = "demo.read_only_write";
    definition.effect = EffectClass::read_only;
    register_command(registry, std::move(definition));
    CommandGateway gateway(std::move(registry), make_document());

    auto request = make_request("demo.read_only_write", "req-read-only");
    set_object_argument(request, 1);
    set_value_argument(request, 99);
    expect_write(request, 1, 1);
    const auto outcome = gateway.execute(request);

    require_error(outcome, ErrorCode::invalid_argument);
    EG_REQUIRE_EQ(object_value(gateway.snapshot(), 1), std::int64_t{10});
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{0});
    EG_REQUIRE_EQ(gateway.last_sequence(), EventSequence{0});
}

EG_TEST(repeat_last_reproduces_without_duplicate_result) {
    CommandGateway gateway(make_registry(), make_document());
    auto request = make_request("demo.set_pair", "req-pair");
    set_pair_arguments(request, 1, 2, 42, 43);
    expect_write(request, 1, 1);
    expect_write(request, 2, 1);
    const auto first = gateway.execute(request);
    EG_REQUIRE(first.succeeded());
    EG_REQUIRE(first.changed);

    const auto recorded = gateway.last_action();
    EG_REQUIRE(recorded.has_value());
    const std::string canonical = recorded->parameters.canonical();
    const std::size_t object_count = gateway.snapshot().objects.size();

    const auto repeated = gateway.repeat_last("req-pair-repeat");
    EG_REQUIRE(repeated.succeeded());
    EG_REQUIRE(!repeated.changed);
    EG_REQUIRE(!repeated.transaction_id.has_value());
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{1});
    EG_REQUIRE_EQ(gateway.last_sequence(), EventSequence{1});
    EG_REQUIRE_EQ(gateway.snapshot().objects.size(), object_count);

    const auto repeated_record = gateway.last_action();
    EG_REQUIRE(repeated_record.has_value());
    EG_REQUIRE_EQ(repeated_record->parameters.canonical(), canonical);
}

EG_TEST(non_repeatable_last_action_is_rejected) {
    CommandGateway gateway(make_registry(), make_document());
    auto request = make_request("demo.edit_pair", "req-edit-non-repeatable");
    set_triple_arguments(request, 1, 2, 3, 99);
    expect_write(request, 1, 1);
    expect_write(request, 2, 1);
    expect_write(request, 3, 0);
    EG_REQUIRE(gateway.execute(request).succeeded());
    const std::size_t object_count = gateway.snapshot().objects.size();

    const auto repeated = gateway.repeat_last("req-edit-repeat");
    require_error(repeated, ErrorCode::not_repeatable);
    EG_REQUIRE_EQ(gateway.snapshot().objects.size(), object_count);
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{1});
    EG_REQUIRE_EQ(gateway.last_sequence(), EventSequence{1});
}

EG_TEST(reopen_parameters_are_immutable_and_reproduce_command) {
    CommandGateway gateway(make_registry(), make_document());
    auto request = make_request("demo.set_pair", "req-reopen-seed");
    set_pair_arguments(request, 1, 2, 42, 43);
    expect_write(request, 1, 1);
    expect_write(request, 2, 1);
    EG_REQUIRE(gateway.execute(request).succeeded());

    const auto recorded = gateway.last_action();
    EG_REQUIRE(recorded.has_value());
    const std::string canonical = recorded->parameters.canonical();
    const std::string digest = recorded->parameters.digest();

    request.arguments.insert_or_assign("first_value", Value{std::int64_t{999}});
    const auto after_request_mutation = gateway.last_action();
    EG_REQUIRE(after_request_mutation.has_value());
    EG_REQUIRE_EQ(after_request_mutation->parameters.canonical(), canonical);
    EG_REQUIRE_EQ(after_request_mutation->parameters.digest(), digest);

    auto reopened = gateway.recorded_command("req-reopened");
    EG_REQUIRE(reopened.has_value());
    EG_REQUIRE_EQ(reopened->command_id, std::string("demo.set_pair"));
    EG_REQUIRE_EQ(reopened->schema_version, kSchemaVersion);
    EG_REQUIRE_EQ(int64_argument(reopened->arguments, "first_value"), std::int64_t{42});
    EG_REQUIRE_EQ(reopened->expected_revisions.writes.at(ObjectId{1}), Revision{2});
    EG_REQUIRE_EQ(reopened->expected_revisions.writes.at(ObjectId{2}), Revision{2});

    const auto reopened_outcome = gateway.execute(*reopened);
    EG_REQUIRE(reopened_outcome.succeeded());
    EG_REQUIRE(!reopened_outcome.changed);
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{1});
    EG_REQUIRE_EQ(gateway.last_sequence(), EventSequence{1});

    reopened->arguments.insert_or_assign("first_value", Value{std::int64_t{1000}});
    const auto after_reopen_mutation = gateway.last_action();
    EG_REQUIRE(after_reopen_mutation.has_value());
    EG_REQUIRE_EQ(after_reopen_mutation->parameters.canonical(), canonical);
}

EG_TEST(event_listener_cannot_mutate_history) {
    std::optional<CommandOutcome> nested;
    CommandGateway gateway(make_registry(), make_document());
    const auto subscription = gateway.subscribe([&gateway, &nested](const DocumentEvent&) {
        nested = gateway.undo();
    });
    EG_REQUIRE(subscription != 0);

    auto request = make_request("demo.set_value", "req-reentrant");
    set_object_argument(request, 1);
    set_value_argument(request, 11);
    expect_write(request, 1, 1);
    const auto outcome = gateway.execute(request);

    EG_REQUIRE(outcome.succeeded());
    EG_REQUIRE(nested.has_value());
    require_error(*nested, ErrorCode::reentrant_mutation);
    EG_REQUIRE_EQ(gateway.undo_depth(), std::size_t{1});
    EG_REQUIRE_EQ(gateway.last_sequence(), EventSequence{1});
}

EG_TEST(request_envelope_validation) {
    CommandGateway gateway(make_registry(), make_document());
    auto request = make_request("demo.set_value", "");
    const auto outcome = gateway.execute(request);
    require_error(outcome, ErrorCode::invalid_request);
}

}  // namespace

int main() {
    const auto& cases = test_cases();
    std::size_t failures = 0;
    for (const auto& test_case : cases) {
        try {
            test_case.function();
            std::cout << "[PASS] " << test_case.name << '\n';
        } catch (const std::exception& exception) {
            ++failures;
            std::cerr << "[FAIL] " << test_case.name << ": " << exception.what() << '\n';
        } catch (...) {
            ++failures;
            std::cerr << "[FAIL] " << test_case.name << ": unknown exception\n";
        }
    }
    std::cout << (cases.size() - failures) << '/' << cases.size()
              << " command-kernel tests passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
