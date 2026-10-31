#include "everygraph/presentation/workbench_window.hpp"

#include "everygraph/application/document.hpp"
#include "everygraph/application/event.hpp"
#include "everygraph/presentation/command_dispatcher.hpp"
#include "everygraph/presentation/document_host.hpp"
#include "everygraph/presentation/project_tree_model.hpp"
#include "everygraph/presentation/properties_panel.hpp"

#include <QAction>
#include <QApplication>
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
                                 QWidget* parent)
    : QMainWindow(parent), query_(query), bindings_(std::move(bindings)) {
    setObjectName(QStringLiteral("workbenchWindow"));
    setWindowTitle(QStringLiteral("EveryGraph Workbench (prototype)"));
    resize(1200, 760);
    build_ui();
    build_actions();
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

    documents_ = new DocumentHost(this);
    setCentralWidget(documents_);

    toolbar_ = addToolBar(QStringLiteral("Workbench"));
    toolbar_->setObjectName(QStringLiteral("workbenchToolbar"));
    toolbar_->setMovable(true);
    toolbar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolbar_->setIconSize(QSize(16, 16));

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
