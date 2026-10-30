#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <stop_token>
#include <string>
#include <string_view>
#include <vector>

#include "everygraph/application/command.hpp"
#include "everygraph/application/document.hpp"
#include "everygraph/foundation/error.hpp"

namespace everygraph::application {

enum class EffectClass {
    read_only,
    reversible_document_write,
    external_write,
    external_execution,
    network_transfer,
};

enum class Repeatability {
    repeatable,
    not_repeatable,
};

struct ExecutionContext {
    foundation::TransactionId transaction_id{foundation::kInvalidTransactionId};
    std::stop_token stop_token;

    [[nodiscard]] bool stop_requested() const noexcept;
    [[nodiscard]] std::optional<foundation::Error> check_canceled() const;
};

using ArgumentValidator =
    std::function<std::optional<foundation::Error>(const ParameterMap& arguments)>;
using CommandHandler = std::function<std::optional<foundation::Error>(
    CommandTransaction& transaction, const CommandRequest& request,
    const ExecutionContext& context)>;

struct CommandDefinition {
    std::string command_id;
    std::vector<std::uint32_t> supported_schema_versions;
    EffectClass effect{EffectClass::reversible_document_write};
    Repeatability repeatability{Repeatability::not_repeatable};
    ArgumentValidator validate_arguments;
    CommandHandler execute;
};

class CommandRegistry {
public:
    [[nodiscard]] std::optional<foundation::Error> register_command(CommandDefinition definition);
    [[nodiscard]] const CommandDefinition* find(std::string_view command_id) const noexcept;
    [[nodiscard]] std::optional<foundation::Error> validate(const CommandRequest& request) const;

private:
    std::map<std::string, CommandDefinition, std::less<>> commands_;
};

}  // namespace everygraph::application
