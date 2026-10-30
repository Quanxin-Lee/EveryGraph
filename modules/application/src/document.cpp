#include "everygraph/application/document.hpp"

#include <utility>

namespace everygraph::application {

const ObjectState* DocumentSnapshot::find(foundation::ObjectId id) const noexcept {
    const auto iterator = objects.find(id);
    return iterator == objects.end() ? nullptr : &iterator->second;
}

CommandTransaction::CommandTransaction(std::map<foundation::ObjectId, ObjectState> baseline)
    : candidate_(std::move(baseline)) {}

const ObjectState* CommandTransaction::find(foundation::ObjectId id) const noexcept {
    const auto iterator = candidate_.find(id);
    return iterator == candidate_.end() ? nullptr : &iterator->second;
}

foundation::Revision CommandTransaction::revision_of(foundation::ObjectId id) const noexcept {
    const auto iterator = candidate_.find(id);
    return iterator == candidate_.end() ? foundation::kInitialRevision : iterator->second.revision;
}

std::vector<ObjectState> CommandTransaction::objects() const {
    std::vector<ObjectState> result;
    result.reserve(candidate_.size());
    for (const auto& entry : candidate_) {
        result.push_back(entry.second);
    }
    return result;
}

std::optional<foundation::Error> CommandTransaction::create_object(
    foundation::ObjectId id, std::string type, ParameterMap properties) {
    if (!id.is_valid()) {
        return foundation::Error{foundation::ErrorCode::invalid_argument,
                                 "object id must not be zero"};
    }
    if (type.empty()) {
        return foundation::Error{foundation::ErrorCode::invalid_argument,
                                 "object type must not be empty"};
    }
    if (candidate_.contains(id)) {
        return foundation::Error{foundation::ErrorCode::already_exists,
                                 "object " + foundation::to_string(id) + " already exists"};
    }
    ObjectState state;
    state.id = id;
    state.revision = foundation::kInitialRevision;
    state.type = std::move(type);
    state.properties = std::move(properties);
    candidate_.emplace(id, std::move(state));
    return std::nullopt;
}

std::optional<foundation::Error> CommandTransaction::set_property(
    foundation::ObjectId id, std::string key, Value value) {
    if (!id.is_valid()) {
        return foundation::Error{foundation::ErrorCode::invalid_argument,
                                 "object id must not be zero"};
    }
    if (key.empty()) {
        return foundation::Error{foundation::ErrorCode::invalid_argument,
                                 "property key must not be empty"};
    }
    if (std::holds_alternative<std::monostate>(value)) {
        return foundation::Error{foundation::ErrorCode::invalid_argument,
                                 "property value must not be empty"};
    }
    const auto iterator = candidate_.find(id);
    if (iterator == candidate_.end()) {
        return foundation::Error{foundation::ErrorCode::not_found,
                                 "object " + foundation::to_string(id) + " was not found"};
    }
    iterator->second.properties.insert_or_assign(std::move(key), std::move(value));
    return std::nullopt;
}

std::optional<foundation::Error> CommandTransaction::erase_object(foundation::ObjectId id) {
    if (!id.is_valid()) {
        return foundation::Error{foundation::ErrorCode::invalid_argument,
                                 "object id must not be zero"};
    }
    const auto iterator = candidate_.find(id);
    if (iterator == candidate_.end()) {
        return foundation::Error{foundation::ErrorCode::not_found,
                                 "object " + foundation::to_string(id) + " was not found"};
    }
    candidate_.erase(iterator);
    return std::nullopt;
}

}  // namespace everygraph::application
