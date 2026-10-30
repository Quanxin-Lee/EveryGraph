#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "everygraph/application/event.hpp"
#include "everygraph/application/value.hpp"
#include "everygraph/foundation/error.hpp"
#include "everygraph/foundation/ids.hpp"

namespace everygraph::application {

struct ExpectedRevisions {
    std::map<foundation::ObjectId, foundation::Revision> reads;
    std::map<foundation::ObjectId, foundation::Revision> writes;
};

struct CommandRequest {
    std::string command_id;
    std::uint32_t schema_version{0};
    std::string request_id;
    std::string principal;
    std::string project_id;
    std::optional<std::string> parent_workflow_id;
    ExpectedRevisions expected_revisions;
    ParameterMap arguments;
};

enum class CommandStatus {
    succeeded,
    failed,
    canceled,
};

struct CommandOutcome {
    CommandStatus status{CommandStatus::failed};
    std::optional<foundation::Error> error;
    std::optional<foundation::TransactionId> transaction_id;
    std::vector<DocumentEvent> events;
    std::map<foundation::ObjectId, foundation::Revision> revisions;
    bool changed{false};

    [[nodiscard]] bool succeeded() const noexcept { return status == CommandStatus::succeeded; }
};

}  // namespace everygraph::application
