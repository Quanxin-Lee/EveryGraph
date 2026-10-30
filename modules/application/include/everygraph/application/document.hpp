#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "everygraph/application/value.hpp"
#include "everygraph/foundation/error.hpp"
#include "everygraph/foundation/ids.hpp"

namespace everygraph::application {

class CommandGateway;

struct ObjectState {
    foundation::ObjectId id;
    foundation::Revision revision{foundation::kInitialRevision};
    std::string type;
    ParameterMap properties;

    friend bool operator==(const ObjectState& left, const ObjectState& right) {
        return left.id == right.id && left.revision == right.revision &&
               left.type == right.type && parameters_equal(left.properties, right.properties);
    }
};

struct DocumentSnapshot {
    std::map<foundation::ObjectId, ObjectState> objects;

    [[nodiscard]] const ObjectState* find(foundation::ObjectId id) const noexcept;
};

class CommandTransaction {
public:
    CommandTransaction(const CommandTransaction&) = delete;
    CommandTransaction& operator=(const CommandTransaction&) = delete;
    CommandTransaction(CommandTransaction&&) noexcept = default;
    CommandTransaction& operator=(CommandTransaction&&) noexcept = default;

    [[nodiscard]] const ObjectState* find(foundation::ObjectId id) const noexcept;
    [[nodiscard]] foundation::Revision revision_of(foundation::ObjectId id) const noexcept;
    [[nodiscard]] std::vector<ObjectState> objects() const;

    [[nodiscard]] std::optional<foundation::Error> create_object(
        foundation::ObjectId id, std::string type, ParameterMap properties = {});
    [[nodiscard]] std::optional<foundation::Error> set_property(
        foundation::ObjectId id, std::string key, Value value);
    [[nodiscard]] std::optional<foundation::Error> erase_object(foundation::ObjectId id);

private:
    friend class CommandGateway;

    explicit CommandTransaction(std::map<foundation::ObjectId, ObjectState> baseline);

    std::map<foundation::ObjectId, ObjectState> candidate_;
};

}  // namespace everygraph::application
