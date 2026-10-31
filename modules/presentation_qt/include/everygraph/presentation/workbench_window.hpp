#pragma once

#include <QMainWindow>
#include <QString>

#include <functional>
#include <string>

#include "everygraph/application/command.hpp"
#include "everygraph/foundation/ids.hpp"
#include "everygraph/presentation/workbench_icons.hpp"

class QAction;
class QDockWidget;
class QToolBar;
class QTreeView;

namespace everygraph::application {
struct DocumentSnapshot;
struct ObjectState;
}  // namespace everygraph::application

namespace everygraph::presentation {

class CommandQuery;
class DocumentHost;
class ProjectTreeModel;
class PropertiesPanel;

// Command handlers supplied by the composition root. The window invokes these
// from user actions and never reaches the command kernel's write path itself.
struct WorkbenchBindings {
    std::function<application::CommandOutcome(foundation::ObjectId parent)> create_folder;
    std::function<application::CommandOutcome(foundation::ObjectId parent)> create_dataset;
    std::function<application::CommandOutcome(foundation::ObjectId object)> move_object;
    std::function<application::CommandOutcome(foundation::ObjectId object)> find_object;
    std::function<application::CommandOutcome()> undo;
    std::function<application::CommandOutcome()> redo;
};

// The workbench composition: menus, toolbar, status bar, project-tree dock,
// properties dock and tabbed document host. Every document write travels
// through a binding; widgets only display kernel snapshots.
class WorkbenchWindow : public QMainWindow {
    Q_OBJECT

public:
    WorkbenchWindow(CommandQuery& query, WorkbenchBindings bindings,
                    QWidget* parent = nullptr);
    ~WorkbenchWindow() override;

    [[nodiscard]] ProjectTreeModel* project_model() const noexcept;
    [[nodiscard]] QTreeView* project_tree() const noexcept;
    [[nodiscard]] PropertiesPanel* properties_panel() const noexcept;
    [[nodiscard]] DocumentHost* document_host() const noexcept;
    [[nodiscard]] QToolBar* workbench_toolbar() const noexcept;
    [[nodiscard]] QDockWidget* project_dock() const noexcept;
    [[nodiscard]] QDockWidget* properties_dock() const noexcept;
    [[nodiscard]] QAction* find_action(const QString& object_name) const;
    [[nodiscard]] foundation::ObjectId selected_object() const noexcept;
    [[nodiscard]] bool select_object(foundation::ObjectId id);
    void open_object_document(foundation::ObjectId id);

public slots:
    void create_folder();
    void create_dataset();
    void move_selected();
    void find_selected();
    void undo();
    void redo();
    void open_selected_document();
    void detach_current_document();
    void dock_current_document();
    void close_current_document();

private:
    void build_ui();
    void build_actions();
    QAction* create_action(const QString& object_name, const QString& text, WorkbenchIcon icon,
                           const QString& shortcut, const QString& tooltip);
    void update_selection();
    void refresh_after_event();
    void update_action_states();
    void restore_detached_windows();
    [[nodiscard]] foundation::ObjectId parent_for_new_object(
        const application::DocumentSnapshot& snapshot) const;
    [[nodiscard]] bool apply_outcome(const application::CommandOutcome& outcome,
                                     const QString& label);
    void select_created_object(const application::DocumentSnapshot& before,
                               const application::CommandOutcome& outcome);

    CommandQuery& query_;
    WorkbenchBindings bindings_;
    ProjectTreeModel* model_{nullptr};
    QTreeView* tree_{nullptr};
    PropertiesPanel* properties_{nullptr};
    DocumentHost* documents_{nullptr};
    QToolBar* toolbar_{nullptr};
    QDockWidget* project_dock_{nullptr};
    QDockWidget* properties_dock_{nullptr};
    foundation::ObjectId selected_;
    QString last_detached_document_;
    application::SubscriptionId subscription_{0};
};

}  // namespace everygraph::presentation
