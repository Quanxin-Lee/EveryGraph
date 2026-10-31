#include <QAbstractSpinBox>
#include <QAction>
#include <QApplication>
#include <QDockWidget>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QStatusBar>
#include <QTabWidget>
#include <QTextEdit>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QWidget>
#include <QtTest/QtTest>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "everygraph/application/command_gateway.hpp"
#include "everygraph/application/document.hpp"
#include "everygraph/presentation/command_dispatcher.hpp"
#include "everygraph/presentation/document_host.hpp"
#include "everygraph/presentation/project_tree_model.hpp"
#include "everygraph/presentation/properties_panel.hpp"
#include "everygraph/presentation/window_placement.hpp"
#include "everygraph/presentation/workbench_window.hpp"

#include "fake_project.hpp"

namespace {

using everygraph::application::CommandOutcome;
using everygraph::application::ExpectedRevisions;
using everygraph::application::ObjectState;
using everygraph::application::ParameterMap;
using everygraph::desktop::FakeCommandDispatch;
using everygraph::desktop::FakeProject;
using everygraph::desktop::FakeProjectController;
using everygraph::desktop::fake_commands::kCreateObject;
using everygraph::desktop::fake_commands::kFindObject;
using everygraph::desktop::fake_commands::kMoveObject;
using everygraph::desktop::fake_ids::kExperimentsFolder;
using everygraph::desktop::fake_ids::kRootFolder;
using everygraph::desktop::fake_ids::kScratchFolder;
using everygraph::desktop::fake_ids::kSpectrumDataset;
using everygraph::foundation::ObjectId;
using everygraph::presentation::CommandQuery;
using everygraph::presentation::DocumentHost;
using everygraph::presentation::ProjectTreeModel;
using everygraph::presentation::PropertiesPanel;
using everygraph::presentation::ShellCommandDispatcher;
using everygraph::presentation::WindowPlacement;
using everygraph::presentation::WorkbenchBindings;
using everygraph::presentation::WorkbenchWindow;

QString command_text(std::string_view command_id) {
    return QString::fromLatin1(command_id.data(), static_cast<int>(command_id.size()));
}

std::optional<std::int64_t> integer_property(const ObjectState& object, std::string_view key) {
    const auto iterator = object.properties.find(key);
    if (iterator == object.properties.end()) {
        return std::nullopt;
    }
    const auto* value = std::get_if<std::int64_t>(&iterator->second);
    return value == nullptr ? std::nullopt : std::optional<std::int64_t>{*value};
}

struct WorkbenchHarness {
    FakeProject project;
    ShellCommandDispatcher dispatcher;
    FakeProjectController controller;
    CommandQuery query;
    WorkbenchBindings bindings;
    std::unique_ptr<WorkbenchWindow> window;

    WorkbenchHarness()
        : project(),
          dispatcher(project.gateway()),
          controller(project.gateway(),
                     [this](std::string_view command_id, ParameterMap arguments,
                            ExpectedRevisions expected) {
                         return dispatcher.execute(command_id, std::move(arguments),
                                                   std::move(expected));
                     }),
          query(project.gateway()) {
        bindings.create_folder = [this](ObjectId parent) {
            return controller.create_folder(parent);
        };
        bindings.create_dataset = [this](ObjectId parent) {
            return controller.create_dataset(parent);
        };
        bindings.move_object = [this](ObjectId object) {
            return controller.move_object(object);
        };
        bindings.find_object = [this](ObjectId object) {
            return controller.find_object(object);
        };
        bindings.undo = [this] { return dispatcher.undo(); };
        bindings.redo = [this] { return dispatcher.redo(); };
        window = std::make_unique<WorkbenchWindow>(query, bindings);
    }
};

}  // namespace

class WorkbenchTests : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void startup_composition();
    void toolbar_actions_are_accessible_and_high_dpi();
    void project_tree_shows_nested_stable_ids();
    void create_move_find_dispatch_registered_commands();
    void fake_object_ids_stay_unique_across_undo();
    void command_dispatcher_submits_and_undoes();
    void properties_panel_is_read_only();
    void document_host_tabs_detach_and_dock();
    void window_placement_restores_after_display_change();
};

void WorkbenchTests::initTestCase() {
    QCOMPARE(QGuiApplication::platformName(), QStringLiteral("offscreen"));
}

void WorkbenchTests::startup_composition() {
    WorkbenchHarness harness;
    WorkbenchWindow& window = *harness.window;

    QVERIFY(window.menuBar() != nullptr);
    QStringList menus;
    for (QAction* action : window.menuBar()->actions()) {
        menus.append(action->text().remove(QLatin1Char('&')));
    }
    for (const QString& expected : {QStringLiteral("File"), QStringLiteral("Edit"),
                                    QStringLiteral("Object"), QStringLiteral("View"),
                                    QStringLiteral("Help")}) {
        QVERIFY2(menus.contains(expected), qPrintable(expected));
    }
    QVERIFY(window.workbench_toolbar() != nullptr);
    QVERIFY(window.project_dock() != nullptr);
    QVERIFY(window.properties_dock() != nullptr);
    QVERIFY(window.document_host() != nullptr);
    QVERIFY(window.centralWidget() == window.document_host());
    QVERIFY(window.statusBar() != nullptr);
    QVERIFY(window.document_host()->document_count() == 1);
    QCOMPARE(window.document_host()->tab_count(), 1);
    QCOMPARE(window.document_host()->current_document_id(), QStringLiteral("welcome"));
    QVERIFY(window.project_dock()->widget() != nullptr);
    QVERIFY(window.project_dock()->widget()->isAncestorOf(window.project_tree()));
    QCOMPARE(window.project_tree()->model(), window.project_model());
}

void WorkbenchTests::toolbar_actions_are_accessible_and_high_dpi() {
    WorkbenchHarness harness;
    QToolBar* toolbar = harness.window->workbench_toolbar();
    QVERIFY(toolbar != nullptr);

    int command_actions = 0;
    for (QAction* action : toolbar->actions()) {
        if (action->isSeparator()) {
            continue;
        }
        ++command_actions;
        QVERIFY2(!action->text().isEmpty(), qPrintable(action->objectName()));
        QVERIFY2(!action->shortcut().isEmpty(), qPrintable(action->objectName()));
        QVERIFY2(!action->icon().isNull(), qPrintable(action->objectName()));

        auto* button = qobject_cast<QToolButton*>(toolbar->widgetForAction(action));
        QVERIFY2(button != nullptr, qPrintable(action->objectName()));
        QCOMPARE(button->accessibleName(), action->text());

        const QIcon icon = action->icon();
        const QList<QSize> available = icon.availableSizes();
        QVERIFY2(available.contains(QSize(16, 16)), qPrintable(action->objectName()));
        QVERIFY2(available.contains(QSize(32, 32)), qPrintable(action->objectName()));
        QVERIFY2(available.contains(QSize(64, 64)), qPrintable(action->objectName()));
        const QPixmap high_dpi = icon.pixmap(QSize(16, 16), 2.0);
        QCOMPARE(high_dpi.width(), 32);
        QCOMPARE(high_dpi.height(), 32);
        QCOMPARE(high_dpi.devicePixelRatio(), 2.0);
    }
    QVERIFY(command_actions >= 9);
}

void WorkbenchTests::project_tree_shows_nested_stable_ids() {
    WorkbenchHarness harness;
    ProjectTreeModel* model = harness.window->project_model();
    QVERIFY(model != nullptr);

    QCOMPARE(model->object_count(), 6);
    QCOMPARE(model->rowCount(), 1);

    const QModelIndex root = model->index_for(kRootFolder);
    QVERIFY(root.isValid());
    QCOMPARE(model->data(root, Qt::DisplayRole).toString(), QStringLiteral("Demo Project"));
    QCOMPARE(model->data(model->index(root.row(), ProjectTreeModel::id_column, {}),
                         Qt::DisplayRole)
                 .toString(),
             QStringLiteral("1"));

    const QModelIndex dataset = model->index_for(kSpectrumDataset);
    QVERIFY(dataset.isValid());
    QCOMPARE(model->data(dataset, Qt::DisplayRole).toString(), QStringLiteral("Spectrum A"));
    const QModelIndex dataset_id =
        model->index(dataset.row(), ProjectTreeModel::id_column, model->parent(dataset));
    QCOMPARE(model->data(dataset_id, Qt::DisplayRole).toString(), QStringLiteral("4"));
    QCOMPARE(model->object_id(dataset), kSpectrumDataset);

    const QModelIndex experiments = model->parent(dataset);
    QVERIFY(experiments.isValid());
    QCOMPARE(model->object_id(experiments), kExperimentsFolder);
    const QModelIndex top = model->parent(experiments);
    QVERIFY(top.isValid());
    QCOMPARE(model->object_id(top), kRootFolder);
    QVERIFY(!model->parent(top).isValid());
    QCOMPARE(model->object_name(kExperimentsFolder), QStringLiteral("Experiments"));
}

void WorkbenchTests::create_move_find_dispatch_registered_commands() {
    WorkbenchHarness harness;
    WorkbenchWindow& window = *harness.window;
    QVERIFY(window.select_object(kExperimentsFolder));
    QCOMPARE(window.properties_panel()->object_id_text(), QStringLiteral("2"));
    QCOMPARE(window.properties_panel()->object_type_text(), QStringLiteral("folder"));

    QAction* create_folder = window.find_action(QStringLiteral("newFolderAction"));
    QVERIFY(create_folder != nullptr);
    QVERIFY(create_folder->isEnabled());
    create_folder->trigger();
    QCOMPARE(QString::fromStdString(harness.dispatcher.last_command_id()), command_text(kCreateObject));
    QCOMPARE(harness.project.gateway().snapshot().objects.size(), std::size_t{7});
    QCOMPARE(window.project_model()->object_count(), 7);

    const ObjectId created = window.selected_object();
    QVERIFY(created.is_valid());
    QVERIFY(created != kExperimentsFolder);
    const auto created_snapshot = harness.project.gateway().snapshot();
    const ObjectState* state = created_snapshot.find(created);
    QVERIFY(state != nullptr);
    QCOMPARE(state->type, std::string("folder"));
    QCOMPARE(integer_property(*state, "parent"), std::optional<std::int64_t>{2});
    QCOMPARE(window.properties_panel()->object_id_text(),
             QString::number(static_cast<qulonglong>(created.value)));
    QCOMPARE(window.properties_panel()->object_type_text(), QStringLiteral("folder"));

    QAction* move = window.find_action(QStringLiteral("moveObjectAction"));
    QVERIFY(move != nullptr);
    QVERIFY(move->isEnabled());
    move->trigger();
    QCOMPARE(QString::fromStdString(harness.dispatcher.last_command_id()), command_text(kMoveObject));
    const auto moved_snapshot = harness.project.gateway().snapshot();
    state = moved_snapshot.find(created);
    QVERIFY(state != nullptr);
    QCOMPARE(integer_property(*state, "parent"), std::optional<std::int64_t>{3});
    QCOMPARE(window.selected_object(), created);

    QAction* find = window.find_action(QStringLiteral("findObjectAction"));
    QVERIFY(find != nullptr);
    QVERIFY(find->isEnabled());
    find->trigger();
    QCOMPARE(QString::fromStdString(harness.dispatcher.last_command_id()), command_text(kFindObject));
    QVERIFY(window.statusBar()->currentMessage().contains(QStringLiteral("Find object succeeded")));
    QCOMPARE(harness.project.gateway().snapshot().objects.size(), std::size_t{7});
    QCOMPARE(harness.project.gateway().undo_depth(), std::size_t{2});

    QAction* undo = window.find_action(QStringLiteral("undoAction"));
    QVERIFY(undo != nullptr);
    QVERIFY(undo->isEnabled());
    undo->trigger();
    const auto undone_snapshot = harness.project.gateway().snapshot();
    state = undone_snapshot.find(created);
    QVERIFY(state != nullptr);
    QCOMPARE(integer_property(*state, "parent"), std::optional<std::int64_t>{2});
    QCOMPARE(undone_snapshot.objects.size(), std::size_t{7});

    QAction* redo = window.find_action(QStringLiteral("redoAction"));
    QVERIFY(redo != nullptr);
    QVERIFY(redo->isEnabled());
    redo->trigger();
    const auto redone_snapshot = harness.project.gateway().snapshot();
    state = redone_snapshot.find(created);
    QVERIFY(state != nullptr);
    QCOMPARE(integer_property(*state, "parent"), std::optional<std::int64_t>{3});

    undo->trigger();
    undo->trigger();
    QCOMPARE(harness.project.gateway().snapshot().objects.size(), std::size_t{6});
    QCOMPARE(window.project_model()->object_count(), 6);
    QCOMPARE(harness.project.gateway().undo_depth(), std::size_t{0});
}

void WorkbenchTests::fake_object_ids_stay_unique_across_undo() {
    WorkbenchHarness harness;
    WorkbenchWindow& window = *harness.window;
    QAction* create = window.find_action(QStringLiteral("newFolderAction"));
    QAction* undo = window.find_action(QStringLiteral("undoAction"));
    QVERIFY(create != nullptr);
    QVERIFY(undo != nullptr);
    QVERIFY(window.select_object(kExperimentsFolder));

    create->trigger();
    const ObjectId first = window.selected_object();
    QVERIFY(first.is_valid());
    QVERIFY(harness.project.gateway().snapshot().find(first) != nullptr);

    undo->trigger();
    QVERIFY(harness.project.gateway().snapshot().find(first) == nullptr);
    QVERIFY(window.select_object(kExperimentsFolder));

    create->trigger();
    const ObjectId second = window.selected_object();
    QVERIFY(second.is_valid());
    QVERIFY2(second != first, "a restored undo slot must not reuse an object ID");
    QCOMPARE(second.value, first.value + 1);
}

void WorkbenchTests::command_dispatcher_submits_and_undoes() {
    FakeProject project;
    ShellCommandDispatcher dispatcher(project.gateway());
    FakeProjectController controller(
        project.gateway(),
        [&dispatcher](std::string_view command_id, ParameterMap arguments,
                      ExpectedRevisions expected) {
            return dispatcher.execute(command_id, std::move(arguments), std::move(expected));
        });

    const CommandOutcome created = controller.create_folder(kRootFolder);
    QVERIFY(created.succeeded());
    QVERIFY(created.changed);
    QCOMPARE(QString::fromStdString(dispatcher.last_command_id()), command_text(kCreateObject));
    QVERIFY(dispatcher.last_request_id().rfind("shell-", 0) == 0);
    QCOMPARE(project.gateway().snapshot().objects.size(), std::size_t{7});

    const CommandOutcome found = controller.find_object(kRootFolder);
    QVERIFY(found.succeeded());
    QCOMPARE(QString::fromStdString(dispatcher.last_command_id()), command_text(kFindObject));
    QVERIFY(!found.changed);

    const CommandOutcome unknown = dispatcher.execute("project.unknown", {});
    QVERIFY(!unknown.succeeded());
    QVERIFY(unknown.error.has_value());

    QVERIFY(dispatcher.undo().succeeded());
    QCOMPARE(project.gateway().snapshot().objects.size(), std::size_t{6});
    QVERIFY(dispatcher.redo().succeeded());
    QCOMPARE(project.gateway().snapshot().objects.size(), std::size_t{7});
}

void WorkbenchTests::properties_panel_is_read_only() {
    WorkbenchHarness harness;
    const auto snapshot = harness.project.gateway().snapshot();
    const ObjectState* dataset = snapshot.find(kSpectrumDataset);
    QVERIFY(dataset != nullptr);

    PropertiesPanel panel;
    panel.show_object(*dataset);
    QCOMPARE(panel.object_id_text(), QStringLiteral("4"));
    QCOMPARE(panel.object_type_text(), QStringLiteral("dataset"));
    QCOMPARE(panel.object_revision_text(), QStringLiteral("1"));
    QVERIFY(panel.properties_text().contains(QStringLiteral("name = Spectrum A")));
    QVERIFY(panel.findChildren<QLineEdit*>().isEmpty());
    QVERIFY(panel.findChildren<QTextEdit*>().isEmpty());
    QVERIFY(panel.findChildren<QAbstractSpinBox*>().isEmpty());
}

void WorkbenchTests::document_host_tabs_detach_and_dock() {
    DocumentHost host;
    auto* first = new QLabel(QStringLiteral("first document"));
    auto* second = new QLabel(QStringLiteral("second document"));
    QVERIFY(host.open_document(
        {"doc-1", QStringLiteral("First"), QStringLiteral("note")}, first));
    QVERIFY(host.open_document(
        {"doc-2", QStringLiteral("Second"), QStringLiteral("dataset")}, second));
    QCOMPARE(host.document_count(), 2);
    QCOMPARE(host.tab_count(), 2);
    QCOMPARE(host.current_document_id(), QStringLiteral("doc-2"));
    QVERIFY(host.contains("doc-1"));
    QCOMPARE(host.content("doc-1"), first);

    auto* duplicate = new QLabel(QStringLiteral("duplicate"));
    QVERIFY(!host.open_document(
        {"doc-1", QStringLiteral("Duplicate"), QStringLiteral("note")}, duplicate));
    delete duplicate;

    QVERIFY(host.detach_document("doc-1"));
    QVERIFY(host.is_detached("doc-1"));
    QCOMPARE(host.tab_count(), 1);
    auto* first_window = host.detached_window("doc-1");
    QVERIFY(first_window != nullptr);
    QTRY_VERIFY(first_window->isVisible());
    QCOMPARE(first_window->descriptor().title, QStringLiteral("First"));

    QVERIFY(host.dock_document("doc-1"));
    QVERIFY(!host.is_detached("doc-1"));
    QCOMPARE(host.tab_count(), 2);
    QCOMPARE(host.current_document_id(), QStringLiteral("doc-1"));

    QVERIFY(host.detach_document("doc-2"));
    auto* second_window = host.detached_window("doc-2");
    QVERIFY(second_window != nullptr);
    second_window->setGeometry(QRect(5000, 4000, 640, 480));
    const std::vector<QRect> screens{QRect(0, 0, 1920, 1080)};
    QVERIFY(!screens.front().intersects(second_window->geometry()));
    host.restore_detached_windows(screens);
    const QRect visible = screens.front().intersected(second_window->geometry());
    QVERIFY(visible.width() >= 64);
    QVERIFY(visible.height() >= 32);

    second_window->close();
    QTRY_VERIFY(!host.is_detached("doc-2"));
    QCOMPARE(host.tab_count(), 2);

    QVERIFY(host.close_document("doc-1"));
    QCOMPARE(host.tab_count(), 1);
    QCOMPARE(host.document_count(), 1);
    QVERIFY(host.close_document("doc-2"));
    QCOMPARE(host.document_count(), 0);
}

void WorkbenchTests::window_placement_restores_after_display_change() {
    const std::vector<QRect> primary{QRect(0, 0, 1920, 1080)};
    const std::vector<QRect> dual{QRect(0, 0, 1920, 1080), QRect(1920, 0, 1920, 1080)};
    const std::vector<QRect> left_of_primary{QRect(-1920, 0, 1920, 1080),
                                             QRect(0, 0, 1920, 1080)};

    const QRect on_secondary(2200, 300, 700, 500);
    QCOMPARE(WindowPlacement::restore_to_visible(on_secondary, dual), on_secondary);
    const QRect restored = WindowPlacement::restore_to_visible(on_secondary, primary);
    QVERIFY(primary.front().contains(restored));
    QCOMPARE(restored.size(), QSize(700, 500));

    const QRect on_left_screen(-1700, 200, 700, 500);
    QCOMPARE(WindowPlacement::restore_to_visible(on_left_screen, left_of_primary),
             on_left_screen);

    const QRect off_screen(4000, 3000, 800, 600);
    const QRect rescued = WindowPlacement::restore_to_visible(off_screen, primary);
    QVERIFY(primary.front().contains(rescued));

    const QRect sliver(1910, 100, 700, 500);
    const QRect sliver_rescued = WindowPlacement::restore_to_visible(sliver, primary);
    QVERIFY(primary.front().contains(sliver_rescued));
    QCOMPARE(sliver_rescued.size(), sliver.size());

    const QRect visible_window(200, 200, 800, 600);
    QCOMPARE(WindowPlacement::restore_to_visible(visible_window, primary), visible_window);

    const QRect oversized(100, 100, 3000, 2000);
    QCOMPARE(WindowPlacement::restore_to_visible(oversized, primary), oversized);

    const QRect oversized_off_screen(-4000, -3000, 3000, 2000);
    const QRect shrunk = WindowPlacement::restore_to_visible(oversized_off_screen, primary);
    QVERIFY(primary.front().contains(shrunk));
    QCOMPARE(shrunk.size(), primary.front().size());

    const QRect title_bar_frame(100, 100, 400, 332);
    const QRect title_bar_client(100, 132, 400, 300);
    const QRect restored_title_bar(32, 32, 400, 332);
    QCOMPARE(WindowPlacement::client_geometry_for_frame(title_bar_frame, title_bar_client,
                                                        restored_title_bar),
             QRect(32, 64, 400, 300));

    const QRect framed(100, 100, 420, 340);
    const QRect framed_client(110, 130, 400, 300);
    QCOMPARE(WindowPlacement::client_geometry_for_frame(framed, framed_client,
                                                        QRect(50, 60, 420, 340)),
             QRect(60, 90, 400, 300));

    QWidget floating;
    floating.setGeometry(QRect(5000, 4000, 320, 240));
    WindowPlacement::ensure_visible(&floating, primary);
    QVERIFY(primary.front().intersects(floating.geometry()));
    QVERIFY(floating.frameGeometry().width() > 0);
}

int main(int argc, char* argv[]) {
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }
    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("EveryGraph"));
    QCoreApplication::setApplicationName(QStringLiteral("EveryGraph Workbench Tests"));
    WorkbenchTests tests;
    return QTest::qExec(&tests, argc, argv);
}

#include "workbench_tests.moc"
