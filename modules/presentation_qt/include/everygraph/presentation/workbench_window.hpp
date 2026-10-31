#pragma once

#include <QMainWindow>
#include <QString>

#include <functional>
#include <string>

#include "everygraph/application/command.hpp"
#include "everygraph/foundation/ids.hpp"
#include "everygraph/presentation/preferences.hpp"
#include "everygraph/presentation/workbench_icons.hpp"

class QAction;
class QDockWidget;
class QLabel;
class QToolBar;
class QTreeView;

namespace everygraph::application {
struct DocumentSnapshot;
struct ObjectState;
}  // namespace everygraph::application

namespace everygraph::presentation {

class CommandQuery;
class DocumentHost;
class FunctionLibraryPanel;
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

using PreferencesSaver =
    std::function<bool(const Preferences& preferences, QString* error_message)>;

// The workbench composition: menus, toolbar, status bar, project-tree dock,
// properties dock, function-library dock and tabbed document host. Every
// document write travels through a binding; widgets only display kernel
// snapshots. Preferences affect shell chrome and panel layout only.
class WorkbenchWindow : public QMainWindow {
    Q_OBJECT

public:
    WorkbenchWindow(CommandQuery& query, WorkbenchBindings bindings,
                    Preferences initial_preferences = {},
                    PreferencesSaver save_preferences = {}, QWidget* parent = nullptr);
    ~WorkbenchWindow() override;

    [[nodiscard]] ProjectTreeModel* project_model() const noexcept;
    [[nodiscard]] QTreeView* project_tree() const noexcept;
    [[nodiscard]] PropertiesPanel* properties_panel() const noexcept;
    [[nodiscard]] DocumentHost* document_host() const noexcept;
    [[nodiscard]] QToolBar* workbench_toolbar() const noexcept;
    [[nodiscard]] QDockWidget* project_dock() const noexcept;
    [[nodiscard]] QDockWidget* properties_dock() const noexcept;
    [[nodiscard]] QDockWidget* function_library_dock() const noexcept;
    [[nodiscard]] FunctionLibraryPanel* function_library_panel() const noexcept;
    [[nodiscard]] QAction* find_action(const QString& object_name) const;
    [[nodiscard]] foundation::ObjectId selected_object() const noexcept;
    [[nodiscard]] const Preferences& preferences() const noexcept;
    [[nodiscard]] FocusMode focus_mode() const noexcept;
    [[nodiscard]] QString default_format_text() const;

    void apply_preferences(const Preferences& preferences);
    [[nodiscard]] bool persist_preferences(QString* error_message = nullptr);
    void set_focus_mode(FocusMode mode);
    void restore_layout();

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
    void open_preferences();
    void select_normal_focus_mode();
    void select_data_focus_mode();
    void select_commands_focus_mode();

private:
    void build_ui();
    void build_actions();
    QAction* create_action(const QString& object_name, const QString& text, WorkbenchIcon icon,
                           const QString& shortcut, const QString& tooltip);
    void apply_appearance();
    void apply_focus_mode_visibility();
    void update_focus_actions();
    void update_default_format_label();
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
    Preferences preferences_;
    PreferencesSaver save_preferences_;
    ProjectTreeModel* model_{nullptr};
    QTreeView* tree_{nullptr};
    PropertiesPanel* properties_{nullptr};
    FunctionLibraryPanel* function_library_{nullptr};
    DocumentHost* documents_{nullptr};
    QToolBar* toolbar_{nullptr};
    QDockWidget* project_dock_{nullptr};
    QDockWidget* properties_dock_{nullptr};
    QDockWidget* function_library_dock_{nullptr};
    QLabel* default_format_label_{nullptr};
    QAction* normal_focus_action_{nullptr};
    QAction* data_focus_action_{nullptr};
    QAction* commands_focus_action_{nullptr};
    foundation::ObjectId selected_;
    QString last_detached_document_;
    application::SubscriptionId subscription_{0};
    bool applying_preferences_{false};
};

}  // namespace everygraph::presentation
