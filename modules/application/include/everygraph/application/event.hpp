#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include "everygraph/foundation/ids.hpp"

namespace everygraph::application {

enum class ChangeKind {
    created,
    updated,
    erased,
};

struct ObjectChange {
    foundation::ObjectId object_id;
    foundation::Revision new_revision{foundation::kInitialRevision};
    ChangeKind kind{ChangeKind::updated};
};

struct DocumentEvent {
    foundation::TransactionId transaction_id{foundation::kInvalidTransactionId};
    foundation::EventSequence sequence{foundation::kNoEventSequence};
    std::vector<ObjectChange> changes;
};

using EventListener = std::function<void(const DocumentEvent&)>;
using SubscriptionId = std::uint64_t;

}  // namespace everygraph::application
