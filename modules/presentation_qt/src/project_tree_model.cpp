#include "everygraph/presentation/project_tree_model.hpp"

#include <QVariant>

#include <algorithm>
#include <cstddef>
#include <set>
#include <string>
#include <utility>

namespace everygraph::presentation {
namespace {

QString display_name(const application::ObjectState& state) {
    const auto name = state.properties.find("name");
    if (name != state.properties.end()) {
        if (const auto* text = std::get_if<std::string>(&name->second); text != nullptr) {
            if (!text->empty()) {
                return QString::fromStdString(*text);
            }
        }
    }
    return QString::fromStdString(foundation::to_string(state.id));
}

foundation::ObjectId parent_property(const application::ObjectState& state) {
    const auto parent = state.properties.find("parent");
    if (parent == state.properties.end()) {
        return {};
    }
    const auto* value = std::get_if<std::int64_t>(&parent->second);
    if (value == nullptr || *value <= 0) {
        return {};
    }
    return foundation::ObjectId{static_cast<std::uint64_t>(*value)};
}

}  // namespace

ProjectTreeModel::ProjectTreeModel(CommandQuery& query, QObject* parent)
    : QAbstractItemModel(parent), query_(query) {
    rebuild();
    subscription_ = query_.subscribe(
        [this](const application::DocumentEvent&) { rebuild(); });
}

ProjectTreeModel::~ProjectTreeModel() {
    query_.unsubscribe(subscription_);
}

QModelIndex ProjectTreeModel::index(int row, int column, const QModelIndex& parent) const {
    if (row < 0 || column < 0 || column >= column_count) {
        return {};
    }
    const Node* parent_node = parent.isValid() ? node_from_index(parent) : &root_;
    if (parent_node == nullptr || row >= static_cast<int>(parent_node->children.size())) {
        return {};
    }
    return createIndex(row, column,
                       parent_node->children[static_cast<std::size_t>(row)]);
}

QModelIndex ProjectTreeModel::parent(const QModelIndex& child) const {
    if (!child.isValid()) {
        return {};
    }
    const Node* node = node_from_index(child);
    if (node == nullptr || node->parent == nullptr || node->parent == &root_) {
        return {};
    }
    return createIndex(node->parent->row, 0, node->parent);
}

int ProjectTreeModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid() && parent.column() != 0) {
        return 0;
    }
    const Node* node = parent.isValid() ? node_from_index(parent) : &root_;
    if (node == nullptr) {
        return 0;
    }
    return static_cast<int>(node->children.size());
}

int ProjectTreeModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid() && parent.column() != 0) {
        return 0;
    }
    return column_count;
}

QVariant ProjectTreeModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return {};
    }
    const Node* node = node_from_index(index);
    if (node == nullptr) {
        return {};
    }
    const application::ObjectState& state = node->state;
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case name_column:
                return display_name(state);
            case type_column:
                return QString::fromStdString(state.type);
            case id_column:
                return QString::number(static_cast<qulonglong>(state.id.value));
            case revision_column:
                return QString::number(static_cast<qulonglong>(state.revision));
            default:
                return {};
        }
    }
    if (role == Qt::ToolTipRole || role == Qt::AccessibleTextRole) {
        return QStringLiteral("%1 (%2), object %3, revision %4")
            .arg(display_name(state), QString::fromStdString(state.type),
                 QString::number(static_cast<qulonglong>(state.id.value)),
                 QString::number(static_cast<qulonglong>(state.revision)));
    }
    if (role == Qt::UserRole) {
        return QVariant::fromValue<qulonglong>(static_cast<qulonglong>(state.id.value));
    }
    if (role == Qt::TextAlignmentRole && index.column() >= id_column) {
        return QVariant::fromValue(static_cast<int>(Qt::AlignRight | Qt::AlignVCenter));
    }
    return {};
}

QVariant ProjectTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return {};
    }
    switch (section) {
        case name_column:
            return QStringLiteral("Name");
        case type_column:
            return QStringLiteral("Type");
        case id_column:
            return QStringLiteral("Object ID");
        case revision_column:
            return QStringLiteral("Revision");
        default:
            return {};
    }
}

Qt::ItemFlags ProjectTreeModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return QAbstractItemModel::flags(index) | Qt::ItemIsSelectable;
}

foundation::ObjectId ProjectTreeModel::object_id(const QModelIndex& index) const {
    const Node* node = node_from_index(index);
    return node == nullptr ? foundation::ObjectId{} : node->state.id;
}

QModelIndex ProjectTreeModel::index_for(foundation::ObjectId id) const {
    const auto iterator = index_.find(id);
    if (iterator == index_.end()) {
        return {};
    }
    return createIndex(iterator->second->row, 0, iterator->second);
}

int ProjectTreeModel::object_count() const noexcept {
    return static_cast<int>(nodes_.size());
}

QString ProjectTreeModel::object_name(foundation::ObjectId id) const {
    const auto iterator = index_.find(id);
    return iterator == index_.end() ? QString() : display_name(iterator->second->state);
}

void ProjectTreeModel::rebuild() {
    beginResetModel();
    nodes_.clear();
    index_.clear();
    root_.children.clear();
    root_.parent = nullptr;
    root_.row = 0;

    const application::DocumentSnapshot snapshot = query_.snapshot();
    nodes_.reserve(snapshot.objects.size());
    for (const auto& entry : snapshot.objects) {
        auto node = std::make_unique<Node>();
        node->state = entry.second;
        index_.emplace(entry.first, node.get());
        nodes_.push_back(std::move(node));
    }

    std::map<foundation::ObjectId, foundation::ObjectId> parents;
    for (const auto& entry : snapshot.objects) {
        const foundation::ObjectId candidate = parent_property(entry.second);
        parents.emplace(entry.first, index_.contains(candidate) ? candidate
                                                                : foundation::ObjectId{});
    }
    for (const auto& entry : parents) {
        std::set<foundation::ObjectId> chain;
        foundation::ObjectId current = entry.first;
        while (current.is_valid()) {
            if (!chain.insert(current).second) {
                parents[entry.first] = foundation::ObjectId{};
                break;
            }
            current = parents.at(current);
        }
    }

    for (const auto& entry : snapshot.objects) {
        Node* node = index_.at(entry.first);
        const foundation::ObjectId parent_id = parents.at(entry.first);
        Node* parent_node =
            parent_id.is_valid() ? index_.at(parent_id) : static_cast<Node*>(&root_);
        node->parent = parent_node;
        node->row = static_cast<int>(parent_node->children.size());
        parent_node->children.push_back(node);
    }
    endResetModel();
}

ProjectTreeModel::Node* ProjectTreeModel::node_from_index(const QModelIndex& index) const {
    if (!index.isValid() || index.model() != this) {
        return nullptr;
    }
    return static_cast<Node*>(index.internalPointer());
}

}  // namespace everygraph::presentation
