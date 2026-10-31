#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QByteArray>
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QDialog>
#include <QDir>
#include <QDockWidget>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLibraryInfo>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QPalette>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStatusBar>
#include <QStringList>
#include <QTemporaryDir>
#include <QToolBar>
#include <QTreeView>
#include <QWidget>
#include <QtTest/QtTest>

#include <cstddef>
#include <memory>
#include <string_view>
#include <utility>

#include "everygraph/application/document.hpp"
#include "everygraph/presentation/command_dispatcher.hpp"
#include "everygraph/presentation/document_host.hpp"
#include "everygraph/presentation/function_library_panel.hpp"
#include "everygraph/presentation/preferences.hpp"
#include "everygraph/presentation/preferences_dialog.hpp"
#include "everygraph/presentation/preferences_store.hpp"
#include "everygraph/presentation/shell_appearance.hpp"
#include "everygraph/presentation/workbench_window.hpp"

#include "fake_project.hpp"

namespace {

using everygraph::application::DocumentSnapshot;
using everygraph::application::ExpectedRevisions;
using everygraph::application::ParameterMap;
using everygraph::desktop::FakeProject;
using everygraph::desktop::FakeProjectController;
using everygraph::desktop::fake_ids::kRootFolder;
using everygraph::presentation::AppearanceMode;
using everygraph::presentation::CommandQuery;
using everygraph::presentation::DefaultFileFormat;
using everygraph::presentation::FocusMode;
using everygraph::presentation::FunctionLibraryPanel;
using everygraph::presentation::PreferenceDiagnostic;
using everygraph::presentation::Preferences;
using everygraph::presentation::PreferencesDialog;
using everygraph::presentation::PreferencesLoadResult;
using everygraph::presentation::PreferencesSaver;
using everygraph::presentation::PreferencesStore;
using everygraph::presentation::ShellAppearance;
using everygraph::presentation::ShellCommandDispatcher;
using everygraph::presentation::WorkbenchBindings;
using everygraph::presentation::WorkbenchWindow;

bool write_text(const QString& path, const QByteArray& text) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    return file.write(text) == static_cast<qint64>(text.size());
}

bool write_json(const QString& path, const QJsonObject& object) {
    return write_text(path, QJsonDocument(object).toJson(QJsonDocument::Indented));
}

bool has_diagnostic(const PreferencesLoadResult& result, const char* code) {
    for (const PreferenceDiagnostic& diagnostic : result.diagnostics) {
        if (diagnostic.code == QLatin1String(code)) {
            return true;
        }
    }
    return false;
}

bool snapshots_match(const DocumentSnapshot& left, const DocumentSnapshot& right) {
    return left.objects == right.objects;
}

struct PreferencesHarness {
    FakeProject project;
    CommandQuery query;
    Preferences persisted;
    int save_count{0};
    bool save_result{true};
    std::unique_ptr<WorkbenchWindow> window;

    explicit PreferencesHarness(const Preferences& initial = Preferences{},
                                bool with_saver = true)
        : query(project.gateway()) {
        PreferencesSaver saver;
        if (with_saver) {
            saver = [this](const Preferences& preferences, QString*) {
                persisted = preferences;
                ++save_count;
                return save_result;
            };
        }
        window =
            std::make_unique<WorkbenchWindow>(query, WorkbenchBindings{}, initial, saver);
    }
};

struct FakePublicationState {
    QColor scene_background;
    QColor export_background;
};

}  // namespace

class PreferencesTests : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void store_defaults_when_file_is_missing();
    void default_path_honours_environment_override();
    void store_round_trips_every_preference();
    void application_restart_persists_preferences();
    void invalid_json_keeps_defaults_and_reports();
    void unknown_and_invalid_settings_use_defaults();
    void unsupported_schema_uses_defaults();
    void non_object_root_uses_defaults();
    void unreadable_file_reports_diagnostic();
    void invalid_window_state_is_rejected();
    void unrestorable_window_state_uses_default_layout();
    void non_object_groups_use_defaults();
    void non_numeric_schema_version_uses_defaults();
    void save_failure_is_reported();
    void failed_save_preserves_previous_file();
    void preferences_do_not_change_project_state();
    void dialog_edits_and_restores_defaults();
    void appearance_changes_shell_chrome_only();
    void focus_modes_toggle_panels_and_shortcuts();
    void normal_mode_respects_visibility_preferences();
    void default_format_label_reflects_preferences();
    void layout_rearrangement_survives_restart();
    void function_library_is_read_only_placeholders();
};

void PreferencesTests::initTestCase() {
    QCOMPARE(QGuiApplication::platformName(), QStringLiteral("offscreen"));
}

void PreferencesTests::store_defaults_when_file_is_missing() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const PreferencesStore store(directory.filePath(QStringLiteral("missing.json")));

    const PreferencesLoadResult loaded = store.load();
    QVERIFY(!loaded.loaded_from_file);
    QVERIFY(loaded.diagnostics.empty());
    QCOMPARE(loaded.preferences, Preferences{});
    QCOMPARE(loaded.preferences.interface.appearance, AppearanceMode::system);
    QCOMPARE(loaded.preferences.focus_mode, FocusMode::normal);
    QCOMPARE(loaded.preferences.defaults.file_format, DefaultFileFormat::csv);
}

void PreferencesTests::default_path_honours_environment_override() {
    const QByteArray original = qgetenv("EVERYGRAPH_PREFERENCES_PATH");
    qputenv("EVERYGRAPH_PREFERENCES_PATH", "/tmp/everygraph-preferences-test.json");
    QCOMPARE(PreferencesStore::default_file_path(),
             QStringLiteral("/tmp/everygraph-preferences-test.json"));

    qunsetenv("EVERYGRAPH_PREFERENCES_PATH");
    QVERIFY(PreferencesStore::default_file_path().endsWith(
        QStringLiteral("/preferences.json")));

    if (original.isEmpty()) {
        qunsetenv("EVERYGRAPH_PREFERENCES_PATH");
    } else {
        qputenv("EVERYGRAPH_PREFERENCES_PATH", original);
    }
}

void PreferencesTests::store_round_trips_every_preference() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("preferences.json"));

    Preferences saved;
    saved.interface.appearance = AppearanceMode::dark;
    saved.interface.show_project_panel = false;
    saved.interface.show_properties_panel = false;
    saved.interface.show_toolbar = true;
    saved.interface.window_state = QByteArray("layout-state-bytes");
    saved.focus_mode = FocusMode::commands_only;
    saved.function_library.visible = false;
    saved.defaults.file_format = DefaultFileFormat::parquet;

    const PreferencesStore store(path);
    QVERIFY(store.save(saved));
    QVERIFY(QFile::exists(path));

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    QVERIFY(document.isObject());
    const QJsonObject root = document.object();
    QCOMPARE(root.value(QLatin1String("schema_version")).toInt(),
             PreferencesStore::kSchemaVersion);
    QCOMPARE(root.keys(),
             (QStringList{QStringLiteral("defaults"), QStringLiteral("focus_mode"),
                          QStringLiteral("function_library"),
                          QStringLiteral("interface"),
                          QStringLiteral("schema_version")}));
    QCOMPARE(root.value(QLatin1String("focus_mode")).toString(),
             QStringLiteral("commands_only"));
    QCOMPARE(root.value(QLatin1String("interface"))
                 .toObject()
                 .value(QLatin1String("appearance"))
                 .toString(),
             QStringLiteral("dark"));
    QCOMPARE(root.value(QLatin1String("defaults"))
                 .toObject()
                 .value(QLatin1String("file_format"))
                 .toString(),
             QStringLiteral("parquet"));

    const PreferencesLoadResult loaded = PreferencesStore(path).load();
    QVERIFY(loaded.loaded_from_file);
    QVERIFY(loaded.diagnostics.empty());
    QCOMPARE(loaded.preferences, saved);
    QCOMPARE(loaded.preferences.interface.window_state, QByteArray("layout-state-bytes"));
}

void PreferencesTests::application_restart_persists_preferences() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("preferences.json"));

    const auto run_desktop = [&path]() {
        QProcess process;
        QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
        environment.insert(QStringLiteral("EVERYGRAPH_PREFERENCES_PATH"), path);
        environment.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("offscreen"));
        environment.insert(QStringLiteral("QT_PLUGIN_PATH"),
                           QLibraryInfo::path(QLibraryInfo::PluginsPath));
        process.setProcessEnvironment(environment);
        process.setProgram(QString::fromLatin1(EVERYGRAPH_DESKTOP_BINARY));
        process.setArguments({QStringLiteral("--smoke-test")});
        process.start();
        if (!process.waitForStarted(10000)) {
            return false;
        }
        if (!process.waitForFinished(30000)) {
            process.kill();
            process.waitForFinished();
            return false;
        }
        return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
    };

    Preferences first;
    first.interface.appearance = AppearanceMode::dark;
    first.interface.show_project_panel = false;
    first.interface.show_properties_panel = false;
    first.interface.show_toolbar = false;
    first.focus_mode = FocusMode::data_only;
    first.function_library.visible = false;
    first.defaults.file_format = DefaultFileFormat::parquet;
    QVERIFY(PreferencesStore(path).save(first));
    QVERIFY(run_desktop());

    PreferencesLoadResult loaded = PreferencesStore(path).load();
    QVERIFY(loaded.loaded_from_file);
    QCOMPARE(loaded.preferences.interface.appearance, AppearanceMode::dark);
    QCOMPARE(loaded.preferences.interface.show_project_panel, false);
    QCOMPARE(loaded.preferences.interface.show_properties_panel, false);
    QCOMPARE(loaded.preferences.interface.show_toolbar, false);
    QCOMPARE(loaded.preferences.focus_mode, FocusMode::data_only);
    QCOMPARE(loaded.preferences.function_library.visible, false);
    QCOMPARE(loaded.preferences.defaults.file_format, DefaultFileFormat::parquet);
    QVERIFY(!loaded.preferences.interface.window_state.isEmpty());

    Preferences second = loaded.preferences;
    second.interface.appearance = AppearanceMode::light;
    second.interface.show_project_panel = true;
    second.interface.show_properties_panel = true;
    second.interface.show_toolbar = true;
    second.focus_mode = FocusMode::commands_only;
    second.function_library.visible = true;
    second.defaults.file_format = DefaultFileFormat::csv;
    QVERIFY(PreferencesStore(path).save(second));
    QVERIFY(run_desktop());

    loaded = PreferencesStore(path).load();
    QVERIFY(loaded.loaded_from_file);
    QCOMPARE(loaded.preferences.interface.appearance, AppearanceMode::light);
    QCOMPARE(loaded.preferences.interface.show_project_panel, true);
    QCOMPARE(loaded.preferences.interface.show_properties_panel, true);
    QCOMPARE(loaded.preferences.interface.show_toolbar, true);
    QCOMPARE(loaded.preferences.focus_mode, FocusMode::commands_only);
    QCOMPARE(loaded.preferences.function_library.visible, true);
    QCOMPARE(loaded.preferences.defaults.file_format, DefaultFileFormat::csv);
    QVERIFY(!loaded.preferences.interface.window_state.isEmpty());
}

void PreferencesTests::invalid_json_keeps_defaults_and_reports() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("preferences.json"));
    QVERIFY(write_text(path, QByteArray("{ this is not json")));

    const PreferencesLoadResult loaded = PreferencesStore(path).load();

    QVERIFY(!loaded.loaded_from_file);
    QCOMPARE(loaded.preferences, Preferences{});
    QCOMPARE(loaded.diagnostics.size(), std::size_t{1});
    QVERIFY(has_diagnostic(loaded, PreferencesStore::kInvalidJsonCode));
}

void PreferencesTests::unknown_and_invalid_settings_use_defaults() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("preferences.json"));

    QJsonObject interface;
    interface.insert(QLatin1String("appearance"), QStringLiteral("neon"));
    interface.insert(QLatin1String("show_project_panel"), QStringLiteral("sometimes"));
    interface.insert(QLatin1String("show_toolbar"), false);
    interface.insert(QLatin1String("window_state"),
                     QStringLiteral("%%%not-base64%%%"));

    QJsonObject function_library;
    function_library.insert(QLatin1String("visible"), false);
    function_library.insert(QLatin1String("auto_run"), true);

    QJsonObject defaults;
    defaults.insert(QLatin1String("file_format"), QStringLiteral("xlsx"));

    QJsonObject root;
    root.insert(QLatin1String("schema_version"), PreferencesStore::kSchemaVersion);
    root.insert(QLatin1String("interface"), interface);
    root.insert(QLatin1String("focus_mode"), QStringLiteral("zen"));
    root.insert(QLatin1String("function_library"), function_library);
    root.insert(QLatin1String("defaults"), defaults);
    root.insert(QLatin1String("cloud_sync"), true);
    QVERIFY(write_json(path, root));

    const PreferencesLoadResult loaded = PreferencesStore(path).load();

    QVERIFY(loaded.loaded_from_file);
    QCOMPARE(loaded.preferences.interface.appearance, AppearanceMode::system);
    QCOMPARE(loaded.preferences.interface.show_project_panel, true);
    QCOMPARE(loaded.preferences.interface.show_toolbar, false);
    QVERIFY(loaded.preferences.interface.window_state.isEmpty());
    QCOMPARE(loaded.preferences.focus_mode, FocusMode::normal);
    QCOMPARE(loaded.preferences.function_library.visible, false);
    QCOMPARE(loaded.preferences.defaults.file_format, DefaultFileFormat::csv);
    QCOMPARE(loaded.diagnostics.size(), std::size_t{7});
    QVERIFY(has_diagnostic(loaded, PreferencesStore::kUnknownKeyCode));
    QVERIFY(has_diagnostic(loaded, PreferencesStore::kInvalidTypeCode));
    QVERIFY(has_diagnostic(loaded, PreferencesStore::kInvalidValueCode));
    QVERIFY(has_diagnostic(loaded, PreferencesStore::kInvalidWindowStateCode));
}

void PreferencesTests::unsupported_schema_uses_defaults() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("preferences.json"));

    QJsonObject root;
    root.insert(QLatin1String("schema_version"), PreferencesStore::kSchemaVersion + 1);
    root.insert(QLatin1String("focus_mode"), QStringLiteral("data_only"));
    QVERIFY(write_json(path, root));

    const PreferencesLoadResult loaded = PreferencesStore(path).load();
    QVERIFY(!loaded.loaded_from_file);
    QCOMPARE(loaded.preferences, Preferences{});
    QCOMPARE(loaded.diagnostics.size(), std::size_t{1});
    QVERIFY(has_diagnostic(loaded, PreferencesStore::kUnsupportedSchemaCode));
}

void PreferencesTests::non_object_root_uses_defaults() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("preferences.json"));
    QVERIFY(write_text(path, QByteArray("[]")));

    const PreferencesLoadResult loaded = PreferencesStore(path).load();
    QVERIFY(!loaded.loaded_from_file);
    QCOMPARE(loaded.preferences, Preferences{});
    QVERIFY(has_diagnostic(loaded, PreferencesStore::kInvalidRootCode));
}

void PreferencesTests::unreadable_file_reports_diagnostic() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("preferences.json"));
    QVERIFY(QDir().mkdir(path));

    const PreferencesLoadResult loaded = PreferencesStore(path).load();
    QVERIFY(!loaded.loaded_from_file);
    QCOMPARE(loaded.preferences, Preferences{});
    QVERIFY(has_diagnostic(loaded, PreferencesStore::kReadFailedCode));
}

void PreferencesTests::invalid_window_state_is_rejected() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("preferences.json"));

    QJsonObject interface;
    interface.insert(QLatin1String("window_state"), QStringLiteral("%%%"));
    QJsonObject root;
    root.insert(QLatin1String("schema_version"), PreferencesStore::kSchemaVersion);
    root.insert(QLatin1String("interface"), interface);
    QVERIFY(write_json(path, root));

    const PreferencesLoadResult loaded = PreferencesStore(path).load();
    QVERIFY(loaded.loaded_from_file);
    QVERIFY(loaded.preferences.interface.window_state.isEmpty());
    QVERIFY(has_diagnostic(loaded, PreferencesStore::kInvalidWindowStateCode));
}

void PreferencesTests::unrestorable_window_state_uses_default_layout() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("preferences.json"));

    Preferences saved;
    saved.interface.window_state = QByteArray("this is not a window layout");
    QVERIFY(PreferencesStore(path).save(saved));

    const PreferencesLoadResult loaded = PreferencesStore(path).load();
    QVERIFY(loaded.loaded_from_file);
    QVERIFY(!loaded.preferences.interface.window_state.isEmpty());

    PreferencesHarness harness(loaded.preferences, false);
    QVERIFY(harness.window->preferences().interface.window_state.isEmpty());
    harness.window->show();
    QTest::qWait(20);
    QVERIFY(harness.window->project_dock()->isVisible());
    QVERIFY(harness.window->properties_dock()->isVisible());
}

void PreferencesTests::non_object_groups_use_defaults() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("preferences.json"));

    QJsonObject root;
    root.insert(QLatin1String("schema_version"), PreferencesStore::kSchemaVersion);
    root.insert(QLatin1String("interface"), QJsonArray{});
    root.insert(QLatin1String("focus_mode"), QStringLiteral("data_only"));
    root.insert(QLatin1String("function_library"), QStringLiteral("yes"));
    root.insert(QLatin1String("defaults"), 7);
    QVERIFY(write_json(path, root));

    const PreferencesLoadResult loaded = PreferencesStore(path).load();
    QVERIFY(loaded.loaded_from_file);
    QCOMPARE(loaded.preferences.interface.appearance, AppearanceMode::system);
    QCOMPARE(loaded.preferences.interface.show_project_panel, true);
    QCOMPARE(loaded.preferences.interface.show_properties_panel, true);
    QCOMPARE(loaded.preferences.interface.show_toolbar, true);
    QVERIFY(loaded.preferences.interface.window_state.isEmpty());
    QCOMPARE(loaded.preferences.focus_mode, FocusMode::data_only);
    QCOMPARE(loaded.preferences.function_library.visible, true);
    QCOMPARE(loaded.preferences.defaults.file_format, DefaultFileFormat::csv);
    QCOMPARE(loaded.diagnostics.size(), std::size_t{3});
    QVERIFY(has_diagnostic(loaded, PreferencesStore::kInvalidTypeCode));
}

void PreferencesTests::non_numeric_schema_version_uses_defaults() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("preferences.json"));

    QJsonObject root;
    root.insert(QLatin1String("schema_version"), QStringLiteral("1"));
    root.insert(QLatin1String("focus_mode"), QStringLiteral("data_only"));
    QVERIFY(write_json(path, root));

    const PreferencesLoadResult loaded = PreferencesStore(path).load();
    QVERIFY(!loaded.loaded_from_file);
    QCOMPARE(loaded.preferences, Preferences{});
    QCOMPARE(loaded.diagnostics.size(), std::size_t{1});
    QVERIFY(has_diagnostic(loaded, PreferencesStore::kUnsupportedSchemaCode));
}

void PreferencesTests::save_failure_is_reported() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString blocker = directory.filePath(QStringLiteral("blocker"));
    QVERIFY(write_text(blocker, QByteArray("not a directory")));

    const PreferencesStore store(
        directory.filePath(QStringLiteral("blocker/preferences.json")));
    QString error;
    QVERIFY(!store.save(Preferences{}, &error));
    QVERIFY(!error.isEmpty());

    PreferencesHarness harness;
    harness.save_result = false;
    QString persist_error;
    QVERIFY(!harness.window->persist_preferences(&persist_error));
    QVERIFY(!persist_error.isEmpty());
    QVERIFY(harness.window->statusBar()->currentMessage().contains(
        QStringLiteral("could not be saved")));
}

void PreferencesTests::failed_save_preserves_previous_file() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("preferences.json"));

    Preferences original;
    original.defaults.file_format = DefaultFileFormat::parquet;
    QVERIFY(PreferencesStore(path).save(original));

    const QFileDevice::Permissions writable = QFile::permissions(directory.path());
    QVERIFY(QFile::setPermissions(directory.path(),
                                  QFileDevice::ReadOwner | QFileDevice::ExeOwner));
    Preferences changed;
    changed.defaults.file_format = DefaultFileFormat::csv;
    QString error;
    const bool saved = PreferencesStore(path).save(changed, &error);
    QVERIFY(QFile::setPermissions(directory.path(), writable));

    QVERIFY(!saved);
    QVERIFY(!error.isEmpty());
    const PreferencesLoadResult loaded = PreferencesStore(path).load();
    QVERIFY(loaded.loaded_from_file);
    QCOMPARE(loaded.preferences.defaults.file_format, DefaultFileFormat::parquet);
}

void PreferencesTests::preferences_do_not_change_project_state() {
    FakeProject project;
    ShellCommandDispatcher dispatcher(project.gateway());
    FakeProjectController controller(
        project.gateway(),
        [&dispatcher](std::string_view command_id, ParameterMap arguments,
                      ExpectedRevisions expected) {
            return dispatcher.execute(command_id, std::move(arguments), std::move(expected));
        });
    QVERIFY(controller.create_dataset(kRootFolder).succeeded());

    CommandQuery query(project.gateway());
    Preferences persisted;
    PreferencesSaver saver = [&persisted](const Preferences& preferences, QString*) {
        persisted = preferences;
        return true;
    };
    WorkbenchWindow window(query, WorkbenchBindings{}, Preferences{}, saver);
    const DocumentSnapshot before = project.gateway().snapshot();
    const std::size_t undo_before = project.gateway().undo_depth();

    Preferences changed;
    changed.interface.appearance = AppearanceMode::dark;
    changed.focus_mode = FocusMode::commands_only;
    window.apply_preferences(changed);
    window.set_focus_mode(FocusMode::normal);
    QVERIFY(window.persist_preferences());

    QVERIFY(snapshots_match(project.gateway().snapshot(), before));
    QCOMPARE(project.gateway().undo_depth(), undo_before);
}

void PreferencesTests::dialog_edits_and_restores_defaults() {
    Preferences initial;
    initial.interface.window_state = QByteArray("saved-layout");
    initial.defaults.file_format = DefaultFileFormat::csv;

    PreferencesDialog dialog(initial);
    auto* appearance = dialog.findChild<QComboBox*>(QStringLiteral("appearanceCombo"));
    auto* focus_mode = dialog.findChild<QComboBox*>(QStringLiteral("focusModeCombo"));
    auto* function_library =
        dialog.findChild<QCheckBox*>(QStringLiteral("showFunctionLibraryCheck"));
    auto* project_panel =
        dialog.findChild<QCheckBox*>(QStringLiteral("showProjectPanelCheck"));
    auto* properties_panel =
        dialog.findChild<QCheckBox*>(QStringLiteral("showPropertiesPanelCheck"));
    auto* toolbar = dialog.findChild<QCheckBox*>(QStringLiteral("showToolbarCheck"));
    auto* file_format =
        dialog.findChild<QComboBox*>(QStringLiteral("defaultFileFormatCombo"));
    QVERIFY(appearance != nullptr);
    QVERIFY(focus_mode != nullptr);
    QVERIFY(function_library != nullptr);
    QVERIFY(project_panel != nullptr);
    QVERIFY(properties_panel != nullptr);
    QVERIFY(toolbar != nullptr);
    QVERIFY(file_format != nullptr);
    QVERIFY(!appearance->accessibleName().isEmpty());
    QVERIFY(!focus_mode->accessibleName().isEmpty());
    QVERIFY(!function_library->accessibleName().isEmpty());
    QVERIFY(!file_format->accessibleName().isEmpty());

    appearance->setCurrentIndex(
        appearance->findData(static_cast<int>(AppearanceMode::dark)));
    focus_mode->setCurrentIndex(
        focus_mode->findData(static_cast<int>(FocusMode::data_only)));
    function_library->setChecked(false);
    project_panel->setChecked(false);
    properties_panel->setChecked(false);
    toolbar->setChecked(false);
    file_format->setCurrentIndex(
        file_format->findData(static_cast<int>(DefaultFileFormat::everygraph_project)));

    bool emitted = false;
    Preferences emitted_preferences;
    connect(&dialog, &PreferencesDialog::preferences_accepted, this,
            [&emitted, &emitted_preferences](const Preferences& preferences) {
                emitted = true;
                emitted_preferences = preferences;
            });
    dialog.accept();

    QVERIFY(emitted);
    QCOMPARE(dialog.result(), static_cast<int>(QDialog::Accepted));
    QCOMPARE(dialog.preferences(), emitted_preferences);
    QCOMPARE(emitted_preferences.interface.appearance, AppearanceMode::dark);
    QCOMPARE(emitted_preferences.focus_mode, FocusMode::data_only);
    QCOMPARE(emitted_preferences.function_library.visible, false);
    QCOMPARE(emitted_preferences.interface.show_project_panel, false);
    QCOMPARE(emitted_preferences.interface.show_properties_panel, false);
    QCOMPARE(emitted_preferences.interface.show_toolbar, false);
    QCOMPARE(emitted_preferences.defaults.file_format,
             DefaultFileFormat::everygraph_project);
    QCOMPARE(emitted_preferences.interface.window_state, QByteArray("saved-layout"));

    dialog.restore_defaults();
    const Preferences restored = dialog.preferences();
    QCOMPARE(restored.interface.appearance, AppearanceMode::system);
    QCOMPARE(restored.focus_mode, FocusMode::normal);
    QCOMPARE(restored.function_library.visible, true);
    QCOMPARE(restored.interface.show_project_panel, true);
    QCOMPARE(restored.interface.show_properties_panel, true);
    QCOMPARE(restored.interface.show_toolbar, true);
    QCOMPARE(restored.defaults.file_format, DefaultFileFormat::csv);
    QCOMPARE(restored.interface.window_state, QByteArray("saved-layout"));
    appearance->setCurrentIndex(
        appearance->findData(static_cast<int>(AppearanceMode::dark)));
    dialog.set_preferences(initial);
    QCOMPARE(dialog.preferences().interface.appearance, AppearanceMode::system);
    QCOMPARE(dialog.preferences().interface.window_state, QByteArray("saved-layout"));
}

void PreferencesTests::appearance_changes_shell_chrome_only() {
    PreferencesHarness harness;
    WorkbenchWindow& window = *harness.window;

    FakePublicationState publication{QColor(QStringLiteral("#f7f1e3")),
                                     QColor(QStringLiteral("#ffffff"))};
    auto* scene = new QWidget;
    QPalette scene_palette = scene->palette();
    scene_palette.setColor(QPalette::Window, publication.scene_background);
    scene->setPalette(scene_palette);
    scene->setAutoFillBackground(true);
    QVERIFY(window.document_host()->open_document(
        {"fake-scene", QStringLiteral("Fake Scene"), QStringLiteral("scene")}, scene));

    window.show();
    QTest::qWait(20);

    QMenu* file_menu = nullptr;
    for (QAction* action : window.menuBar()->actions()) {
        if (action->menu() != nullptr) {
            file_menu = action->menu();
            break;
        }
    }
    QVERIFY(file_menu != nullptr);

    const QPalette menu_base = window.menuBar()->palette();
    const QPalette status_base = window.statusBar()->palette();
    const QPalette toolbar_base = window.workbench_toolbar()->palette();
    const QPalette chrome_menu_base = file_menu->palette();
    const QPalette dock_base = window.project_dock()->palette();
    const QPalette tree_base = window.project_tree()->palette();
    const QPalette host_base = window.document_host()->palette();
    const QPalette scene_base = scene->palette();

    Preferences dark = window.preferences();
    dark.interface.appearance = AppearanceMode::dark;
    window.apply_preferences(dark);

    const QPalette dark_chrome = ShellAppearance::dark_palette();
    QCOMPARE(window.property("everygraphAppearance").toString(), QStringLiteral("dark"));
    QCOMPARE(window.menuBar()->palette().color(QPalette::Window),
             dark_chrome.color(QPalette::Window));
    QCOMPARE(window.statusBar()->palette().color(QPalette::Window),
             dark_chrome.color(QPalette::Window));
    QCOMPARE(window.project_dock()->palette().color(QPalette::Window),
             dark_chrome.color(QPalette::Window));
    QCOMPARE(window.workbench_toolbar()->palette().color(QPalette::Window),
             dark_chrome.color(QPalette::Window));
    QCOMPARE(file_menu->palette().color(QPalette::Window),
             dark_chrome.color(QPalette::Window));
    QCOMPARE(window.project_tree()->palette().color(QPalette::Window),
             dark_chrome.color(QPalette::Window));
    QCOMPARE(window.document_host()->palette(), host_base);
    QCOMPARE(scene->palette(), scene_base);
    QCOMPARE(scene->palette().color(QPalette::Window),
             QColor(QStringLiteral("#f7f1e3")));
    QCOMPARE(publication.export_background, QColor(QStringLiteral("#ffffff")));

    Preferences light = dark;
    light.interface.appearance = AppearanceMode::light;
    window.apply_preferences(light);
    const QPalette light_chrome = ShellAppearance::light_palette();
    QCOMPARE(window.menuBar()->palette().color(QPalette::Window),
             light_chrome.color(QPalette::Window));
    QCOMPARE(window.statusBar()->palette().color(QPalette::Window),
             light_chrome.color(QPalette::Window));
    QCOMPARE(window.workbench_toolbar()->palette().color(QPalette::Window),
             light_chrome.color(QPalette::Window));
    QCOMPARE(file_menu->palette().color(QPalette::Window),
             light_chrome.color(QPalette::Window));
    QCOMPARE(window.project_dock()->palette().color(QPalette::Window),
             light_chrome.color(QPalette::Window));
    QCOMPARE(window.project_tree()->palette().color(QPalette::Window),
             light_chrome.color(QPalette::Window));
    QVERIFY(light_chrome.color(QPalette::Window) !=
            dark_chrome.color(QPalette::Window));
    QCOMPARE(window.document_host()->palette(), host_base);
    QCOMPARE(scene->palette(), scene_base);

    Preferences system = light;
    system.interface.appearance = AppearanceMode::system;
    window.apply_preferences(system);
    QCOMPARE(window.property("everygraphAppearance").toString(),
             QStringLiteral("system"));
    QCOMPARE(window.menuBar()->palette(), menu_base);
    QCOMPARE(window.statusBar()->palette(), status_base);
    QCOMPARE(window.workbench_toolbar()->palette(), toolbar_base);
    QCOMPARE(file_menu->palette(), chrome_menu_base);
    QCOMPARE(window.project_dock()->palette(), dock_base);
    QCOMPARE(window.project_tree()->palette(), tree_base);
    QCOMPARE(window.document_host()->palette(), host_base);
    QCOMPARE(scene->palette(), scene_base);
}

void PreferencesTests::focus_modes_toggle_panels_and_shortcuts() {
    PreferencesHarness harness;
    WorkbenchWindow& window = *harness.window;
    window.resize(1024, 600);
    window.show();
    QTest::qWait(20);
    QCOMPARE(window.size(), QSize(1024, 600));
    QVERIFY(window.project_dock()->isVisible());
    QVERIFY(window.properties_dock()->isVisible());
    QVERIFY(window.function_library_dock()->isVisible());
    QVERIFY(window.workbench_toolbar()->isVisible());
    QVERIFY(window.document_host()->isVisible());

    QAction* normal_action = window.find_action(QStringLiteral("normalFocusAction"));
    QAction* data_action = window.find_action(QStringLiteral("dataFocusAction"));
    QAction* commands_action = window.find_action(QStringLiteral("commandsFocusAction"));
    QVERIFY(normal_action != nullptr);
    QVERIFY(data_action != nullptr);
    QVERIFY(commands_action != nullptr);
    QVERIFY(!normal_action->shortcut().isEmpty());
    QVERIFY(!data_action->shortcut().isEmpty());
    QVERIFY(!commands_action->shortcut().isEmpty());
    QVERIFY(normal_action->isCheckable());
    QVERIFY(data_action->isCheckable());
    QVERIFY(commands_action->isCheckable());

    window.activateWindow();
    QTest::qWait(10);

    QTest::keyClick(&window, Qt::Key_D, Qt::AltModifier | Qt::ControlModifier);
    QTRY_COMPARE(window.focus_mode(), FocusMode::data_only);
    QVERIFY(window.project_dock()->isVisible());
    QVERIFY(!window.properties_dock()->isVisible());
    QVERIFY(!window.function_library_dock()->isVisible());
    QVERIFY(!window.workbench_toolbar()->isVisible());
    QVERIFY(window.document_host()->isVisible());
    QVERIFY(data_action->isChecked());
    QCOMPARE(harness.persisted.focus_mode, FocusMode::data_only);
    QVERIFY(harness.save_count > 0);

    QTest::keyClick(&window, Qt::Key_C, Qt::AltModifier | Qt::ControlModifier);
    QTRY_COMPARE(window.focus_mode(), FocusMode::commands_only);
    QVERIFY(!window.project_dock()->isVisible());
    QVERIFY(!window.properties_dock()->isVisible());
    QVERIFY(!window.function_library_dock()->isVisible());
    QVERIFY(window.workbench_toolbar()->isVisible());
    QVERIFY(!window.document_host()->isVisible());
    QVERIFY(commands_action->isChecked());
    QCOMPARE(harness.persisted.focus_mode, FocusMode::commands_only);

    QTest::keyClick(&window, Qt::Key_0, Qt::AltModifier | Qt::ControlModifier);
    QTRY_COMPARE(window.focus_mode(), FocusMode::normal);
    QVERIFY(window.project_dock()->isVisible());
    QVERIFY(window.properties_dock()->isVisible());
    QVERIFY(window.function_library_dock()->isVisible());
    QVERIFY(window.workbench_toolbar()->isVisible());
    QVERIFY(window.document_host()->isVisible());
    QVERIFY(normal_action->isChecked());
    QCOMPARE(harness.persisted.focus_mode, FocusMode::normal);
}

void PreferencesTests::normal_mode_respects_visibility_preferences() {
    Preferences preferences;
    preferences.interface.show_project_panel = false;
    preferences.interface.show_toolbar = false;
    preferences.function_library.visible = false;

    PreferencesHarness harness(preferences);
    WorkbenchWindow& window = *harness.window;
    window.show();
    QTest::qWait(20);
    QVERIFY(!window.project_dock()->isVisible());
    QVERIFY(window.properties_dock()->isVisible());
    QVERIFY(!window.function_library_dock()->isVisible());
    QVERIFY(!window.workbench_toolbar()->isVisible());

    window.set_focus_mode(FocusMode::data_only);
    QVERIFY(window.project_dock()->isVisible());
    QVERIFY(!window.properties_dock()->isVisible());
    QVERIFY(!window.workbench_toolbar()->isVisible());
    QVERIFY(window.document_host()->isVisible());

    window.set_focus_mode(FocusMode::normal);
    QVERIFY(!window.project_dock()->isVisible());
    QVERIFY(window.properties_dock()->isVisible());
    QVERIFY(!window.function_library_dock()->isVisible());
    QVERIFY(!window.workbench_toolbar()->isVisible());
    QCOMPARE(harness.persisted.interface.show_project_panel, false);
    QCOMPARE(harness.persisted.interface.show_toolbar, false);
    QCOMPARE(harness.persisted.function_library.visible, false);
}

void PreferencesTests::default_format_label_reflects_preferences() {
    PreferencesHarness harness;
    WorkbenchWindow& window = *harness.window;
    QCOMPARE(window.default_format_text(), QStringLiteral("Default format: CSV"));

    Preferences parquet = window.preferences();
    parquet.defaults.file_format = DefaultFileFormat::parquet;
    window.apply_preferences(parquet);
    QCOMPARE(window.default_format_text(), QStringLiteral("Default format: Parquet"));

    Preferences project = parquet;
    project.defaults.file_format = DefaultFileFormat::everygraph_project;
    window.apply_preferences(project);
    QCOMPARE(window.default_format_text(),
             QStringLiteral("Default format: EveryGraph project"));
}

void PreferencesTests::layout_rearrangement_survives_restart() {
    PreferencesHarness harness;
    WorkbenchWindow& window = *harness.window;
    window.resize(1100, 720);
    window.show();
    QTest::qWait(20);

    const int original_width = window.properties_dock()->width();
    window.resizeDocks({window.properties_dock()}, {original_width + 120},
                       Qt::Horizontal);
    QTest::qWait(20);
    QVERIFY(window.properties_dock()->width() > original_width);
    const int resized_width = window.properties_dock()->width();

    window.addDockWidget(Qt::RightDockWidgetArea, window.project_dock());
    QCOMPARE(window.dockWidgetArea(window.project_dock()), Qt::RightDockWidgetArea);
    QVERIFY(window.persist_preferences());
    QVERIFY(!harness.persisted.interface.window_state.isEmpty());
    const Preferences saved = harness.persisted;

    PreferencesHarness restored(saved, false);
    WorkbenchWindow& second = *restored.window;
    second.show();
    QTest::qWait(20);
    QCOMPARE(second.dockWidgetArea(second.project_dock()), Qt::RightDockWidgetArea);
    QVERIFY(second.properties_dock()->width() >= resized_width / 2);
}

void PreferencesTests::function_library_is_read_only_placeholders() {
    FunctionLibraryPanel panel;
    QCOMPARE(panel.function_count(), 5);
    const QStringList names = panel.function_names();
    QCOMPARE(names.size(), 5);
    QVERIFY(names.contains(QStringLiteral("FFT Spectrum")));
    QVERIFY(names.contains(QStringLiteral("Column Statistics")));

    auto* list = panel.findChild<QListWidget*>(QStringLiteral("functionLibraryList"));
    QVERIFY(list != nullptr);
    QCOMPARE(list->count(), panel.function_count());
    QCOMPARE(list->editTriggers(), QAbstractItemView::NoEditTriggers);
}

QTEST_MAIN(PreferencesTests)

#include "preferences_tests.moc"
