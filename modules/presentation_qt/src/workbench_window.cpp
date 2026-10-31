#include "everygraph/presentation/workbench_window.hpp"

#include "everygraph/application/document.hpp"
#include "everygraph/application/event.hpp"
#include "everygraph/presentation/command_dispatcher.hpp"
#include "everygraph/presentation/document_host.hpp"
#include "everygraph/presentation/function_library_panel.hpp"
#include "everygraph/presentation/preferences_dialog.hpp"
#include "everygraph/presentation/preferences_store.hpp"
#include "everygraph/presentation/project_tree_model.hpp"
#include "everygraph/presentation/properties_panel.hpp"
#include "everygraph/presentation/shell_appearance.hpp"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDebug>
#include <QDockWidget>
#include <QGuiApplication>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QRect>
#include <QScreen>
#include <QSize>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace everygraph::presentation {
namespace {

bool is_folder(const application::ObjectState& state) {
    return state.type == "folder";
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

foundation::ObjectId root_folder_id(const application::DocumentSnapshot& snapshot) {
    for (const auto& entry : snapshot.objects) {
        if (is_folder(entry.second) && !parent_property(entry.second).is_valid()) {
            return entry.first;
        }
    }
    for (const auto& entry : snapshot.objects) {
        if (is_folder(entry.second)) {
            return entry.first;
        }
    }
    return {};
}

QString object_label(foundation::ObjectId id) {
    return QString::number(static_cast<qulonglong>(id.value));
}

}  // namespace

WorkbenchWindow::WorkbenchWindow(CommandQuery& query, WorkbenchBindings bindings,
                                 Preferences initial_preferences,
                                 PreferencesSaver save_preferences, QWidget* parent)
    : QMainWindow(parent),
      query_(query),
      bindings_(std::move(bindings)),
      preferences_(std::move(initial_preferences)),
      save_preferences_(std::move(save_preferences)) {
    setObjectName(QStringLiteral("workbenchWindow"));
    setWindowTitle(QStringLiteral("EveryGraph Workbench (prototype)"));
    resize(1200, 760);
    build_ui();
    build_actions();
    restore_layout();
    apply_preferences(preferences_);
    connect(project_dock_, &QDockWidget::visibilityChanged, this, [this](bool visible) {
        if (applying_preferences_ || preferences_.focus_mode != FocusMode::normal ||
            preferences_.interface.show_project_panel == visible) {
            return;
        }
        preferences_.interface.show_project_panel = visible;
        (void)persist_preferences();
    });
    connect(properties_dock_, &QDockWidget::visibilityChanged, this, [this](bool visible) {
        if (applying_preferences_ || preferences_.focus_mode != FocusMode::normal ||
            preferences_.interface.show_properties_panel == visible) {
            return;
        }
        preferences_.interface.show_properties_panel = visible;
        (void)persist_preferences();
    });
    connect(function_library_dock_, &QDockWidget::visibilityChanged, this,
            [this](bool visible) {
                if (applying_preferences_ || preferences_.focus_mode != FocusMode::normal ||
                    preferences_.function_library.visible == visible) {
                    return;
                }
                preferences_.function_library.visible = visible;
                (void)persist_preferences();
            });
    subscription_ = query_.subscribe(
        [this](const application::DocumentEvent&) { refresh_after_event(); });
    connect(qGuiApp, &QGuiApplication::screenAdded, this,
            [this](QScreen*) { restore_detached_windows(); });
    connect(qGuiApp, &QGuiApplication::screenRemoved, this,
            [this](QScreen*) { restore_detached_windows(); });
    update_action_states();
    statusBar()->showMessage(
        QStringLiteral("Fake in-memory project; all document writes go through the command kernel."));
}

WorkbenchWindow::~WorkbenchWindow() {
    // Hiding docks during QMainWindow teardown emits visibilityChanged after
    // the layout and this window's preference members are gone; disconnect the
    // dock signals before teardown and drop the saver so nothing persists.
    for (QDockWidget* dock : {project_dock_, properties_dock_, function_library_dock_}) {
        if (dock != nullptr) {
            disconnect(dock, nullptr, this, nullptr);
        }
    }
    save_preferences_ = {};
    query_.unsubscribe(subscription_);
}

void WorkbenchWindow::build_ui() {
    model_ = new ProjectTreeModel(query_, this);

    tree_ = new QTreeView(this);
    tree_->setObjectName(QStringLiteral("projectTree"));
    tree_->setAccessibleName(QStringLiteral("Project tree"));
    tree_->setModel(model_);
    tree_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tree_->setSelectionMode(QAbstractItemView::SingleSelection);
    tree_->setUniformRowHeights(true);
    tree_->setAlternatingRowColors(true);
    tree_->header()->setSectionResizeMode(ProjectTreeModel::name_column,
                                          QHeaderView::Stretch);
    tree_->header()->setSectionResizeMode(ProjectTreeModel::type_column,
                                          QHeaderView::ResizeToContents);
    tree_->header()->setSectionResizeMode(ProjectTreeModel::id_column,
                                          QHeaderView::ResizeToContents);
    tree_->header()->setSectionResizeMode(ProjectTreeModel::revision_column,
                                          QHeaderView::ResizeToContents);
    connect(tree_->selectionModel(), &QItemSelectionModel::currentChanged, this,
            [this](const QModelIndex&, const QModelIndex&) { update_selection(); });

    project_dock_ = new QDockWidget(QStringLiteral("Project"), this);
    project_dock_->setObjectName(QStringLiteral("projectDock"));
    project_dock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    auto* project_panel = new QWidget(project_dock_);
    auto* project_layout = new QVBoxLayout(project_panel);
    project_layout->setContentsMargins(4, 4, 4, 4);
    project_layout->addWidget(tree_, 1);
    auto* project_note = new QLabel(
        QStringLiteral("Read-only view of the kernel snapshot. Stable object IDs are shown."),
        project_panel);
    project_note->setWordWrap(true);
    project_layout->addWidget(project_note);
    project_dock_->setWidget(project_panel);
    addDockWidget(Qt::LeftDockWidgetArea, project_dock_);

    properties_ = new PropertiesPanel(this);
    properties_dock_ = new QDockWidget(QStringLiteral("Properties"), this);
    properties_dock_->setObjectName(QStringLiteral("propertiesDock"));
    properties_dock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    properties_dock_->setWidget(properties_);
    addDockWidget(Qt::RightDockWidgetArea, properties_dock_);

    function_library_ = new FunctionLibraryPanel(this);
    function_library_dock_ = new QDockWidget(QStringLiteral("Function Library"), this);
    function_library_dock_->setObjectName(QStringLiteral("functionLibraryDock"));
    function_library_dock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    function_library_dock_->setWidget(function_library_);
    addDockWidget(Qt::LeftDockWidgetArea, function_library_dock_);

    documents_ = new DocumentHost(this);
    setCentralWidget(documents_);

    toolbar_ = addToolBar(QStringLiteral("Workbench"));
    toolbar_->setObjectName(QStringLiteral("workbenchToolbar"));
    toolbar_->setMovable(true);
    toolbar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolbar_->setIconSize(QSize(16, 16));

    default_format_label_ = new QLabel(this);
    default_format_label_->setObjectName(QStringLiteral("defaultFormatLabel"));
    default_format_label_->setAccessibleName(QStringLiteral("Default file format"));
    statusBar()->addPermanentWidget(default_format_label_);

    auto* welcome = new QLabel(
        QStringLiteral("EveryGraph workbench prototype.\n\n"
                       "This shell hosts placeholder documents, dockable panels and "
                       "command-driven actions over a fake in-memory project.\n"
                       "No scientific data, persistence or editing is implemented."));
    welcome->setAlignment(Qt::AlignCenter);
    welcome->setWordWrap(true);
    welcome->setMinimumSize(360, 240);
    if (!documents_->open_document(
            DocumentDescriptor{"welcome", QStringLiteral("Welcome"), QStringLiteral("note")},
            welcome)) {
        delete welcome;
    }
    properties_->clear();
}

void WorkbenchWindow::build_actions() {
    QAction* new_folder =
        create_action(QStringLiteral("newFolderAction"), QStringLiteral("New Folder"),
                      WorkbenchIcon::new_folder, QStringLiteral("Ctrl+Shift+N"),
                      QStringLiteral("Create a folder through the project command"));
    QAction* new_dataset =
        create_action(QStringLiteral("newDatasetAction"), QStringLiteral("New Dataset"),
                      WorkbenchIcon::new_dataset, QStringLiteral("Ctrl+Shift+D"),
                      QStringLiteral("Create a dataset through the project command"));
    QAction* move_object =
        create_action(QStringLiteral("moveObjectAction"), QStringLiteral("Move Selected"),
                      WorkbenchIcon::move_object, QStringLiteral("Ctrl+M"),
                      QStringLiteral("Move the selected object through the project command"));
    QAction* find_object =
        create_action(QStringLiteral("findObjectAction"), QStringLiteral("Find Selected"),
                      WorkbenchIcon::find_object, QStringLiteral("Ctrl+F"),
                      QStringLiteral("Check the selected object through the read-only find command"));
    QAction* open_document =
        create_action(QStringLiteral("openDocumentAction"), QStringLiteral("Open Document"),
                      WorkbenchIcon::open_document, QStringLiteral("Ctrl+O"),
                      QStringLiteral("Open a placeholder document for the selected object"));
    QAction* undo_action =
        create_action(QStringLiteral("undoAction"), QStringLiteral("Undo"),
                      WorkbenchIcon::undo, QStringLiteral("Ctrl+Z"),
                      QStringLiteral("Undo the last kernel transaction"));
    QAction* redo_action =
        create_action(QStringLiteral("redoAction"), QStringLiteral("Redo"),
                      WorkbenchIcon::redo, QStringLiteral("Ctrl+Shift+Z"),
                      QStringLiteral("Redo the last undone transaction"));
    QAction* detach_action =
        create_action(QStringLiteral("detachAction"), QStringLiteral("Detach Document"),
                      WorkbenchIcon::detach_document, QStringLiteral("Ctrl+Shift+T"),
                      QStringLiteral("Float the current document in its own window"));
    QAction* dock_action =
        create_action(QStringLiteral("dockAction"), QStringLiteral("Dock Document"),
                      WorkbenchIcon::dock_document, QStringLiteral("Ctrl+Shift+R"),
                      QStringLiteral("Return the detached document to the tab host"));
    QAction* close_document =
        create_action(QStringLiteral("closeDocumentAction"), QStringLiteral("Close Document"),
                      WorkbenchIcon::open_document, QStringLiteral("Ctrl+W"),
                      QStringLiteral("Close the current placeholder document"));

    toolbar_->addAction(new_folder);
    toolbar_->addAction(new_dataset);
    toolbar_->addAction(move_object);
    toolbar_->addAction(find_object);
    toolbar_->addAction(open_document);
    toolbar_->addSeparator();
    toolbar_->addAction(undo_action);
    toolbar_->addAction(redo_action);
    toolbar_->addSeparator();
    toolbar_->addAction(detach_action);
    toolbar_->addAction(dock_action);

    for (QAction* action : toolbar_->actions()) {
        if (action->isSeparator()) {
            continue;
        }
        if (auto* button = qobject_cast<QToolButton*>(toolbar_->widgetForAction(action))) {
            button->setObjectName(action->objectName() + QStringLiteral("Button"));
            button->setAccessibleName(action->text());
            button->setToolTip(action->toolTip());
        }
    }

    connect(new_folder, &QAction::triggered, this, &WorkbenchWindow::create_folder);
    connect(new_dataset, &QAction::triggered, this, &WorkbenchWindow::create_dataset);
    connect(move_object, &QAction::triggered, this, &WorkbenchWindow::move_selected);
    connect(find_object, &QAction::triggered, this, &WorkbenchWindow::find_selected);
    connect(open_document, &QAction::triggered, this, &WorkbenchWindow::open_selected_document);
    connect(undo_action, &QAction::triggered, this, &WorkbenchWindow::undo);
    connect(redo_action, &QAction::triggered, this, &WorkbenchWindow::redo);
    connect(detach_action, &QAction::triggered, this, &WorkbenchWindow::detach_current_document);
    connect(dock_action, &QAction::triggered, this, &WorkbenchWindow::dock_current_document);
    connect(close_document, &QAction::triggered, this, &WorkbenchWindow::close_current_document);

    QMenu* file_menu = menuBar()->addMenu(QStringLiteral("&File"));
    file_menu->addAction(open_document);
    file_menu->addAction(close_document);
    file_menu->addSeparator();
    auto* quit = new QAction(QStringLiteral("Quit"), this);
    quit->setObjectName(QStringLiteral("quitAction"));
    quit->setShortcut(QKeySequence(QStringLiteral("Ctrl+Q")));
    connect(quit, &QAction::triggered, qApp, &QApplication::quit);
    file_menu->addAction(quit);

    QMenu* edit_menu = menuBar()->addMenu(QStringLiteral("&Edit"));
    edit_menu->addAction(undo_action);
    edit_menu->addAction(redo_action);
    edit_menu->addSeparator();
    edit_menu->addAction(find_object);
    edit_menu->addSeparator();
    auto* preferences_action = new QAction(QStringLiteral("Preferences..."), this);
    preferences_action->setObjectName(QStringLiteral("preferencesAction"));
    preferences_action->setShortcut(QKeySequence::Preferences);
    preferences_action->setMenuRole(QAction::PreferencesRole);
    preferences_action->setStatusTip(
        QStringLiteral("Edit interface preferences stored outside the project"));
    connect(preferences_action, &QAction::triggered, this,
            &WorkbenchWindow::open_preferences);
    edit_menu->addAction(preferences_action);

    QMenu* object_menu = menuBar()->addMenu(QStringLiteral("&Object"));
    object_menu->addAction(new_folder);
    object_menu->addAction(new_dataset);
    object_menu->addSeparator();
    object_menu->addAction(move_object);

    QMenu* view_menu = menuBar()->addMenu(QStringLiteral("&View"));
    QAction* project_toggle = project_dock_->toggleViewAction();
    project_toggle->setObjectName(QStringLiteral("projectDockToggleAction"));
    project_toggle->setShortcut(QKeySequence(QStringLiteral("Ctrl+1")));
    view_menu->addAction(project_toggle);
    QAction* properties_toggle = properties_dock_->toggleViewAction();
    properties_toggle->setObjectName(QStringLiteral("propertiesDockToggleAction"));
    properties_toggle->setShortcut(QKeySequence(QStringLiteral("Ctrl+2")));
    view_menu->addAction(properties_toggle);
    QAction* function_library_toggle = function_library_dock_->toggleViewAction();
    function_library_toggle->setObjectName(QStringLiteral("functionLibraryDockToggleAction"));
    function_library_toggle->setShortcut(QKeySequence(QStringLiteral("Ctrl+3")));
    view_menu->addAction(function_library_toggle);
    view_menu->addSeparator();

    normal_focus_action_ = new QAction(QStringLiteral("Normal Layout"), this);
    normal_focus_action_->setObjectName(QStringLiteral("normalFocusAction"));
    normal_focus_action_->setShortcut(QKeySequence(QStringLiteral("Ctrl+Alt+0")));
    normal_focus_action_->setCheckable(true);
    normal_focus_action_->setStatusTip(
        QStringLiteral("Show the configured panels and the command toolbar"));
    data_focus_action_ = new QAction(QStringLiteral("Data-Only Focus Mode"), this);
    data_focus_action_->setObjectName(QStringLiteral("dataFocusAction"));
    data_focus_action_->setShortcut(QKeySequence(QStringLiteral("Ctrl+Alt+D")));
    data_focus_action_->setCheckable(true);
    data_focus_action_->setStatusTip(
        QStringLiteral("Show only the project tree and document area"));
    commands_focus_action_ = new QAction(QStringLiteral("Commands-Only Focus Mode"), this);
    commands_focus_action_->setObjectName(QStringLiteral("commandsFocusAction"));
    commands_focus_action_->setShortcut(QKeySequence(QStringLiteral("Ctrl+Alt+C")));
    commands_focus_action_->setCheckable(true);
    commands_focus_action_->setStatusTip(
        QStringLiteral("Show only menus, the command toolbar and the status bar"));
    auto* focus_group = new QActionGroup(this);
    focus_group->setExclusive(true);
    for (QAction* focus_action :
         {normal_focus_action_, data_focus_action_, commands_focus_action_}) {
        focus_group->addAction(focus_action);
        view_menu->addAction(focus_action);
    }
    connect(normal_focus_action_, &QAction::triggered, this,
            &WorkbenchWindow::select_normal_focus_mode);
    connect(data_focus_action_, &QAction::triggered, this,
            &WorkbenchWindow::select_data_focus_mode);
    connect(commands_focus_action_, &QAction::triggered, this,
            &WorkbenchWindow::select_commands_focus_mode);

    view_menu->addSeparator();
    view_menu->addAction(detach_action);
    view_menu->addAction(dock_action);

    QMenu* help_menu = menuBar()->addMenu(QStringLiteral("&Help"));
    auto* about = new QAction(QStringLiteral("About EveryGraph Workbench"), this);
    about->setObjectName(QStringLiteral("aboutAction"));
    connect(about, &QAction::triggered, this, [this] {
        statusBar()->showMessage(
            QStringLiteral("EveryGraph workbench prototype: fake in-memory project, "
                           "command kernel only, no scientific implementation."),
            10000);
    });
    help_menu->addAction(about);
}

QAction* WorkbenchWindow::create_action(const QString& object_name, const QString& text,
                                        WorkbenchIcon icon, const QString& shortcut,
                                        const QString& tooltip) {
    auto* action = new QAction(make_workbench_icon(icon), text, this);
    action->setObjectName(object_name);
    action->setIconText(text);
    action->setShortcut(QKeySequence(shortcut));
    action->setShortcutContext(Qt::WindowShortcut);
    action->setToolTip(tooltip);
    action->setStatusTip(tooltip);
    return action;
}

ProjectTreeModel* WorkbenchWindow::project_model() const noexcept {
    return model_;
}

QTreeView* WorkbenchWindow::project_tree() const noexcept {
    return tree_;
}

PropertiesPanel* WorkbenchWindow::properties_panel() const noexcept {
    return properties_;
}

DocumentHost* WorkbenchWindow::document_host() const noexcept {
    return documents_;
}

QToolBar* WorkbenchWindow::workbench_toolbar() const noexcept {
    return toolbar_;
}

QDockWidget* WorkbenchWindow::project_dock() const noexcept {
    return project_dock_;
}

QDockWidget* WorkbenchWindow::properties_dock() const noexcept {
    return properties_dock_;
}

QDockWidget* WorkbenchWindow::function_library_dock() const noexcept {
    return function_library_dock_;
}

FunctionLibraryPanel* WorkbenchWindow::function_library_panel() const noexcept {
    return function_library_;
}

const Preferences& WorkbenchWindow::preferences() const noexcept {
    return preferences_;
}

FocusMode WorkbenchWindow::focus_mode() const noexcept {
    return preferences_.focus_mode;
}

QString WorkbenchWindow::default_format_text() const {
    return default_format_label_ == nullptr ? QString() : default_format_label_->text();
}

QAction* WorkbenchWindow::find_action(const QString& object_name) const {
    return findChild<QAction*>(object_name);
}

foundation::ObjectId WorkbenchWindow::selected_object() const noexcept {
    return selected_;
}

bool WorkbenchWindow::select_object(foundation::ObjectId id) {
    const QModelIndex index = model_->index_for(id);
    if (!index.isValid()) {
        return false;
    }
    tree_->setCurrentIndex(index);
    tree_->scrollTo(index);
    return true;
}

void WorkbenchWindow::open_object_document(foundation::ObjectId id) {
    const application::DocumentSnapshot snapshot = query_.snapshot();
    const application::ObjectState* state = snapshot.find(id);
    if (state == nullptr) {
        statusBar()->showMessage(QStringLiteral("Object %1 no longer exists.").arg(object_label(id)),
                                 5000);
        return;
    }
    const std::string document_id = "object-" + std::to_string(id.value);
    if (documents_->contains(document_id)) {
        statusBar()->showMessage(QStringLiteral("Document for object %1 is already open.")
                                     .arg(object_label(id)),
                                 5000);
        return;
    }
    QString name = model_->object_name(id);
    if (name.isEmpty()) {
        name = object_label(id);
    }
    auto* content = new QLabel(
        QStringLiteral("Placeholder document for %1\n\n"
                       "Object ID %2, type %3, revision %4.\n"
                       "The command kernel owns this object; this page only displays it.")
            .arg(name, object_label(id), QString::fromStdString(state->type),
                 QString::number(static_cast<qulonglong>(state->revision))));
    content->setAlignment(Qt::AlignCenter);
    content->setWordWrap(true);
    content->setMinimumSize(360, 240);
    const DocumentDescriptor descriptor{document_id, name,
                                        QString::fromStdString(state->type)};
    if (!documents_->open_document(descriptor, content)) {
        delete content;
        statusBar()->showMessage(QStringLiteral("The document host refused the document."), 5000);
        return;
    }
    statusBar()->showMessage(QStringLiteral("Opened document for object %1.").arg(name), 5000);
}

void WorkbenchWindow::create_folder() {
    if (!bindings_.create_folder) {
        return;
    }
    const application::DocumentSnapshot before = query_.snapshot();
    const application::CommandOutcome outcome =
        bindings_.create_folder(parent_for_new_object(before));
    if (apply_outcome(outcome, QStringLiteral("Create folder"))) {
        select_created_object(before, outcome);
    }
}

void WorkbenchWindow::create_dataset() {
    if (!bindings_.create_dataset) {
        return;
    }
    const application::DocumentSnapshot before = query_.snapshot();
    const application::CommandOutcome outcome =
        bindings_.create_dataset(parent_for_new_object(before));
    if (apply_outcome(outcome, QStringLiteral("Create dataset"))) {
        select_created_object(before, outcome);
    }
}

void WorkbenchWindow::move_selected() {
    if (!selected_.is_valid() || !bindings_.move_object) {
        return;
    }
    const foundation::ObjectId moved = selected_;
    if (apply_outcome(bindings_.move_object(moved), QStringLiteral("Move object"))) {
        (void)select_object(moved);
    }
}

void WorkbenchWindow::find_selected() {
    if (!selected_.is_valid() || !bindings_.find_object) {
        return;
    }
    const foundation::ObjectId found = selected_;
    if (apply_outcome(bindings_.find_object(found), QStringLiteral("Find object"))) {
        (void)select_object(found);
    }
}

void WorkbenchWindow::undo() {
    if (!bindings_.undo) {
        return;
    }
    (void)apply_outcome(bindings_.undo(), QStringLiteral("Undo"));
    update_action_states();
}

void WorkbenchWindow::redo() {
    if (!bindings_.redo) {
        return;
    }
    (void)apply_outcome(bindings_.redo(), QStringLiteral("Redo"));
    update_action_states();
}

void WorkbenchWindow::open_selected_document() {
    if (!selected_.is_valid()) {
        statusBar()->showMessage(QStringLiteral("Select a project object first."), 5000);
        return;
    }
    open_object_document(selected_);
}

void WorkbenchWindow::detach_current_document() {
    const QString current = documents_->current_document_id();
    if (current.isEmpty()) {
        statusBar()->showMessage(QStringLiteral("No tabbed document to detach."), 5000);
        return;
    }
    if (documents_->detach_document(current.toStdString())) {
        last_detached_document_ = current;
        statusBar()->showMessage(
            QStringLiteral("Detached document %1 in its own window.").arg(current), 5000);
    }
}

void WorkbenchWindow::dock_current_document() {
    if (last_detached_document_.isEmpty() ||
        !documents_->is_detached(last_detached_document_.toStdString())) {
        statusBar()->showMessage(QStringLiteral("No detached document to dock."), 5000);
        return;
    }
    if (documents_->dock_document(last_detached_document_.toStdString())) {
        statusBar()->showMessage(
            QStringLiteral("Docked document %1 back into the tabs.").arg(last_detached_document_),
            5000);
        last_detached_document_.clear();
    }
}

void WorkbenchWindow::close_current_document() {
    const QString current = documents_->current_document_id();
    if (current.isEmpty()) {
        statusBar()->showMessage(QStringLiteral("No document is open."), 5000);
        return;
    }
    if (documents_->close_document(current.toStdString())) {
        statusBar()->showMessage(QStringLiteral("Closed document %1.").arg(current), 5000);
    }
}

void WorkbenchWindow::open_preferences() {
    PreferencesDialog dialog(preferences_, this);
    connect(&dialog, &PreferencesDialog::preferences_accepted, this,
            [this](const Preferences& updated) {
                apply_preferences(updated);
                (void)persist_preferences();
            });
    dialog.exec();
}

void WorkbenchWindow::select_normal_focus_mode() {
    set_focus_mode(FocusMode::normal);
}

void WorkbenchWindow::select_data_focus_mode() {
    set_focus_mode(FocusMode::data_only);
}

void WorkbenchWindow::select_commands_focus_mode() {
    set_focus_mode(FocusMode::commands_only);
}

void WorkbenchWindow::set_focus_mode(FocusMode mode) {
    preferences_.focus_mode = mode;
    applying_preferences_ = true;
    apply_focus_mode_visibility();
    update_focus_actions();
    applying_preferences_ = false;
    (void)persist_preferences();
}

void WorkbenchWindow::apply_preferences(const Preferences& preferences) {
    applying_preferences_ = true;
    preferences_ = preferences;
    apply_appearance();
    apply_focus_mode_visibility();
    update_focus_actions();
    update_default_format_label();
    applying_preferences_ = false;
}

void WorkbenchWindow::apply_appearance() {
    ShellAppearance::apply(*this, preferences_.interface.appearance);
}

void WorkbenchWindow::apply_focus_mode_visibility() {
    const bool normal = preferences_.focus_mode == FocusMode::normal;
    const bool data_only = preferences_.focus_mode == FocusMode::data_only;
    const bool commands_only = preferences_.focus_mode == FocusMode::commands_only;

    project_dock_->setVisible(normal ? preferences_.interface.show_project_panel : data_only);
    properties_dock_->setVisible(normal && preferences_.interface.show_properties_panel);
    function_library_dock_->setVisible(normal && preferences_.function_library.visible);
    toolbar_->setVisible(normal ? preferences_.interface.show_toolbar : commands_only);
    documents_->setVisible(!commands_only);
}

void WorkbenchWindow::update_focus_actions() {
    if (normal_focus_action_ != nullptr) {
        normal_focus_action_->setChecked(preferences_.focus_mode == FocusMode::normal);
    }
    if (data_focus_action_ != nullptr) {
        data_focus_action_->setChecked(preferences_.focus_mode == FocusMode::data_only);
    }
    if (commands_focus_action_ != nullptr) {
        commands_focus_action_->setChecked(preferences_.focus_mode == FocusMode::commands_only);
    }
}

void WorkbenchWindow::update_default_format_label() {
    if (default_format_label_ == nullptr) {
        return;
    }
    default_format_label_->setText(
        QStringLiteral("Default format: %1")
            .arg(default_file_format_display_name(preferences_.defaults.file_format)));
}

bool WorkbenchWindow::persist_preferences(QString* error_message) {
    if (!save_preferences_) {
        return true;
    }
    preferences_.interface.window_state = saveState(1);
    QString error;
    if (!save_preferences_(preferences_, &error)) {
        const QString message =
            error.isEmpty() ? QStringLiteral("unknown error") : error;
        statusBar()->showMessage(
            QStringLiteral("Preferences could not be saved: %1").arg(message), 10000);
        if (error_message != nullptr) {
            *error_message = message;
        }
        return false;
    }
    if (error_message != nullptr) {
        error_message->clear();
    }
    return true;
}

void WorkbenchWindow::restore_layout() {
    if (preferences_.interface.window_state.isEmpty()) {
        return;
    }
    if (!restoreState(preferences_.interface.window_state, 1)) {
        qWarning().noquote()
            << QString::fromLatin1(PreferencesStore::kInvalidWindowStateCode)
                       + QLatin1Char(':')
            << QStringLiteral("the saved window layout could not be restored; "
                              "using the default layout");
        preferences_.interface.window_state.clear();
    }
}

void WorkbenchWindow::restore_detached_windows() {
    std::vector<QRect> available_areas;
    for (const QScreen* screen : QGuiApplication::screens()) {
        if (screen != nullptr) {
            available_areas.push_back(screen->availableGeometry());
        }
    }
    documents_->restore_detached_windows(available_areas);
}

void WorkbenchWindow::update_selection() {
    const foundation::ObjectId id = model_->object_id(tree_->currentIndex());
    const application::DocumentSnapshot snapshot = query_.snapshot();
    const application::ObjectState* state = snapshot.find(id);
    if (state == nullptr) {
        selected_ = {};
        properties_->clear();
    } else {
        selected_ = id;
        properties_->show_object(*state);
    }
    update_action_states();
}

void WorkbenchWindow::refresh_after_event() {
    const application::DocumentSnapshot snapshot = query_.snapshot();
    const application::ObjectState* state = snapshot.find(selected_);
    if (state == nullptr) {
        selected_ = {};
        properties_->clear();
    } else {
        properties_->show_object(*state);
    }
    update_action_states();
}

void WorkbenchWindow::update_action_states() {
    const bool has_selection = selected_.is_valid();
    for (const QString& name : {QStringLiteral("moveObjectAction"),
                                QStringLiteral("findObjectAction"),
                                QStringLiteral("openDocumentAction")}) {
        if (QAction* action = find_action(name)) {
            action->setEnabled(has_selection);
        }
    }
    if (QAction* undo_action = find_action(QStringLiteral("undoAction"))) {
        undo_action->setEnabled(query_.undo_depth() > 0);
    }
    if (QAction* redo_action = find_action(QStringLiteral("redoAction"))) {
        redo_action->setEnabled(query_.redo_depth() > 0);
    }
}

foundation::ObjectId WorkbenchWindow::parent_for_new_object(
    const application::DocumentSnapshot& snapshot) const {
    const application::ObjectState* state = snapshot.find(selected_);
    if (state != nullptr) {
        if (is_folder(*state)) {
            return selected_;
        }
        const foundation::ObjectId parent = parent_property(*state);
        if (parent.is_valid()) {
            return parent;
        }
    }
    return root_folder_id(snapshot);
}

bool WorkbenchWindow::apply_outcome(const application::CommandOutcome& outcome,
                                    const QString& label) {
    if (outcome.succeeded()) {
        statusBar()->showMessage(QStringLiteral("%1 succeeded.").arg(label), 5000);
        return true;
    }
    const QString detail = outcome.error.has_value()
                               ? QString::fromStdString(outcome.error->message)
                               : QStringLiteral("unknown failure");
    statusBar()->showMessage(QStringLiteral("%1 failed: %2").arg(label, detail), 10000);
    return false;
}

void WorkbenchWindow::select_created_object(const application::DocumentSnapshot& before,
                                            const application::CommandOutcome& outcome) {
    for (const auto& entry : outcome.revisions) {
        if (entry.first.is_valid() && before.find(entry.first) == nullptr) {
            (void)select_object(entry.first);
            return;
        }
    }
}

}  // namespace everygraph::presentation
