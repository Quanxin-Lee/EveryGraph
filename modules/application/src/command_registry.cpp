#include "everygraph/application/command_registry.hpp"

#include <algorithm>
#include <cstddef>
#include <exception>
#include <set>
#include <utility>

namespace everygraph::application {
namespace {

bool is_valid_command_id(std::string_view command_id) {
    if (command_id.empty()) {
        return false;
    }
    bool at_segment_start = true;
    std::size_t separator_count = 0;
    for (const char character : command_id) {
        if (character == '.') {
            if (at_segment_start) {
                return false;
            }
            at_segment_start = true;
            ++separator_count;
            continue;
        }
        const bool lowercase = character >= 'a' && character <= 'z';
        const bool digit = character >= '0' && character <= '9';
        const bool underscore = character == '_';
        if (at_segment_start) {
            if (!lowercase) {
                return false;
            }
            at_segment_start = false;
        } else if (!(lowercase || digit || underscore)) {
            return false;
        }
    }
    return !at_segment_start && separator_count >= 1;
}

bool supports_schema_version(const CommandDefinition& definition, std::uint32_t schema_version) {
    return std::find(definition.supported_schema_versions.begin(),
                     definition.supported_schema_versions.end(),
                     schema_version) != definition.supported_schema_versions.end();
}

}  // namespace

bool ExecutionContext::stop_requested() const noexcept { return stop_token.stop_requested(); }

std::optional<foundation::Error> ExecutionContext::check_canceled() const {
    if (!stop_requested()) {
        return std::nullopt;
    }
    return foundation::Error{foundation::ErrorCode::canceled, "command canceled"};
}

std::optional<foundation::Error> CommandRegistry::register_command(CommandDefinition definition) {
    if (!is_valid_command_id(definition.command_id)) {
        return foundation::Error{foundation::ErrorCode::invalid_argument,
                                 "command id must be a dotted lowercase namespace.verb name"};
    }
    if (definition.supported_schema_versions.empty()) {
        return foundation::Error{foundation::ErrorCode::invalid_argument,
                                 "command must declare at least one schema version"};
    }
    std::set<std::uint32_t> versions;
    for (const std::uint32_t version : definition.supported_schema_versions) {
        if (version == 0) {
            return foundation::Error{foundation::ErrorCode::invalid_argument,
                                     "schema version 0 is invalid"};
        }
        if (!versions.insert(version).second) {
            return foundation::Error{foundation::ErrorCode::invalid_argument,
                                     "schema version " + std::to_string(version) +
                                         " is declared more than once"};
        }
    }
    if (!definition.validate_arguments) {
        return foundation::Error{foundation::ErrorCode::invalid_argument,
                                 "command must provide an argument validator"};
    }
    if (!definition.execute) {
        return foundation::Error{foundation::ErrorCode::invalid_argument,
                                 "command must provide an execution handler"};
    }
    if (definition.effect == EffectClass::external_write ||
        definition.effect == EffectClass::external_execution ||
        definition.effect == EffectClass::network_transfer) {
        return foundation::Error{foundation::ErrorCode::unsupported_feature,
                                 "external effects are not implemented by this prototype"};
    }
    if (commands_.contains(definition.command_id)) {
        return foundation::Error{foundation::ErrorCode::duplicate_command,
                                 "command " + definition.command_id + " is already registered"};
    }
    std::string key = definition.command_id;
    commands_.emplace(std::move(key), std::move(definition));
    return std::nullopt;
}

const CommandDefinition* CommandRegistry::find(std::string_view command_id) const noexcept {
    const auto iterator = commands_.find(command_id);
    return iterator == commands_.end() ? nullptr : &iterator->second;
}

std::optional<foundation::Error> CommandRegistry::validate(const CommandRequest& request) const {
    if (!is_valid_command_id(request.command_id)) {
        return foundation::Error{foundation::ErrorCode::invalid_request,
                                 "command id must be a dotted lowercase namespace.verb name"};
    }
    const CommandDefinition* definition = find(request.command_id);
    if (definition == nullptr) {
        return foundation::Error{foundation::ErrorCode::unknown_command,
                                 "unknown command " + request.command_id};
    }
    if (!supports_schema_version(*definition, request.schema_version)) {
        return foundation::Error{foundation::ErrorCode::unsupported_schema_version,
                                 "command " + request.command_id +
                                     " does not support schema version " +
                                     std::to_string(request.schema_version)};
    }
    try {
        return definition->validate_arguments(request.arguments);
    } catch (const std::exception& exception) {
        return foundation::Error{foundation::ErrorCode::internal_error,
                                 "argument validator failed: " + std::string(exception.what())};
    } catch (...) {
        return foundation::Error{foundation::ErrorCode::internal_error,
                                 "argument validator failed with an unknown exception"};
    }
}

}  // namespace everygraph::application
