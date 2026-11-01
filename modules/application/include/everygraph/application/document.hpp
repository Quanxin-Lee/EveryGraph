#pragma once

#include <map>
#include <optional>
#include <string>
#include <variant>
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

// A document snapshot checked to hold one identity per object: every map key
// equals the id the stored object carries, and no key is the invalid id zero.
// CommandGateway is built only from this form, so a loader, import or
// undo-from-disk path cannot hand it a document whose lookup keys disagree with
// the identities they hold; `from` reports a rejection instead of re-keying the
// caller's objects, and the checked map has no accessor outside the gateway.
class ValidatedDocument {
public:
    ValidatedDocument() = default;

    [[nodiscard]] static std::variant<ValidatedDocument, foundation::Error> from(
        DocumentSnapshot snapshot);

private:
    friend class CommandGateway;

    explicit ValidatedDocument(DocumentSnapshot snapshot);

    DocumentSnapshot snapshot_;
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
