#pragma once

#include <QAbstractItemModel>
#include <QString>

#include <cstdint>
#include <map>
#include <memory>
#include <vector>

#include "everygraph/application/document.hpp"
#include "everygraph/foundation/ids.hpp"
#include "everygraph/presentation/command_dispatcher.hpp"

namespace everygraph::presentation {

// Read-only view of the kernel snapshot. The model never mutates an object: it
// rebuilds from CommandQuery events, so the kernel stays the only writer.
class ProjectTreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    enum Column {
        name_column = 0,
        type_column = 1,
        id_column = 2,
        revision_column = 3,
        column_count = 4,
    };

    explicit ProjectTreeModel(CommandQuery& query, QObject* parent = nullptr);
    ~ProjectTreeModel() override;

    ProjectTreeModel(const ProjectTreeModel&) = delete;
    ProjectTreeModel& operator=(const ProjectTreeModel&) = delete;

    [[nodiscard]] QModelIndex index(int row, int column,
                                    const QModelIndex& parent = {}) const override;
    [[nodiscard]] QModelIndex parent(const QModelIndex& child) const override;
    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index,
                                int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                      int role = Qt::DisplayRole) const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;

    [[nodiscard]] foundation::ObjectId object_id(const QModelIndex& index) const;
    [[nodiscard]] QModelIndex index_for(foundation::ObjectId id) const;
    [[nodiscard]] int object_count() const noexcept;
    [[nodiscard]] QString object_name(foundation::ObjectId id) const;

private:
    struct Node {
        application::ObjectState state;
        Node* parent{nullptr};
        std::vector<Node*> children;
        int row{0};
    };

    void rebuild();
    [[nodiscard]] Node* node_from_index(const QModelIndex& index) const;

    CommandQuery& query_;
    application::SubscriptionId subscription_{0};
    std::vector<std::unique_ptr<Node>> nodes_;
    Node root_;
    std::map<foundation::ObjectId, Node*> index_;
};

}  // namespace everygraph::presentation
