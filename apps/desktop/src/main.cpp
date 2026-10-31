#include <QApplication>
#include <QString>
#include <QTimer>

#include <string_view>
#include <utility>

#include "everygraph/presentation/command_dispatcher.hpp"
#include "everygraph/presentation/preferences_store.hpp"
#include "everygraph/presentation/workbench_window.hpp"

#include "fake_project.hpp"

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    QApplication::setApplicationName(QStringLiteral("EveryGraph Workbench"));
    QApplication::setOrganizationName(QStringLiteral("EveryGraph"));

    using everygraph::application::ExpectedRevisions;
    using everygraph::application::ParameterMap;
    using everygraph::foundation::ObjectId;
    using everygraph::presentation::Preferences;
    using everygraph::presentation::PreferencesLoadResult;
    using everygraph::presentation::PreferencesStore;

    PreferencesStore preferences_store(PreferencesStore::default_file_path());
    const PreferencesLoadResult loaded_preferences = preferences_store.load();

    everygraph::desktop::FakeProject project;
    everygraph::presentation::ShellCommandDispatcher dispatcher(project.gateway());
    everygraph::presentation::CommandQuery query(project.gateway());
    everygraph::desktop::FakeProjectController controller(
        project.gateway(),
        [&dispatcher](std::string_view command_id, ParameterMap arguments,
                      ExpectedRevisions expected) {
            return dispatcher.execute(command_id, std::move(arguments), std::move(expected));
        });

    everygraph::presentation::WorkbenchBindings bindings;
    bindings.create_folder = [&controller](ObjectId parent) {
        return controller.create_folder(parent);
    };
    bindings.create_dataset = [&controller](ObjectId parent) {
        return controller.create_dataset(parent);
    };
    bindings.move_object = [&controller](ObjectId object) {
        return controller.move_object(object);
    };
    bindings.find_object = [&controller](ObjectId object) {
        return controller.find_object(object);
    };
    bindings.undo = [&dispatcher] { return dispatcher.undo(); };
    bindings.redo = [&dispatcher] { return dispatcher.redo(); };

    everygraph::presentation::WorkbenchWindow window(
        query, std::move(bindings), loaded_preferences.preferences,
        [&preferences_store](const Preferences& preferences, QString* error_message) {
            return preferences_store.save(preferences, error_message);
        });
    QObject::connect(&application, &QCoreApplication::aboutToQuit, &window, [&window] {
        (void)window.persist_preferences();
    });
    window.show();

    const bool smoke_test = application.arguments().contains(QStringLiteral("--smoke-test"));
    if (smoke_test) {
        QTimer::singleShot(0, &application, &QCoreApplication::quit);
    }
    return application.exec();
}
