#include <QAbstractButton>
#include <QAccessible>
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QDockWidget>
#include <QFontMetrics>
#include <QFileInfo>
#include <QImage>
#include <QImageWriter>
#include <QInputMethodEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QPalette>
#include <QPixmap>
#include <QPushButton>
#include <QScreen>
#include <QStatusBar>
#include <QTabBar>
#include <QTabWidget>
#include <QTest>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>
#include <QtTest/QtTest>

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "everygraph/application/command_gateway.hpp"
#include "everygraph/application/document.hpp"
#include "everygraph/presentation/command_dispatcher.hpp"
#include "everygraph/presentation/document_host.hpp"
#include "everygraph/presentation/function_library_panel.hpp"
#include "everygraph/presentation/preferences.hpp"
#include "everygraph/presentation/preferences_dialog.hpp"
#include "everygraph/presentation/properties_panel.hpp"
#include "everygraph/presentation/shell_appearance.hpp"
#include "everygraph/presentation/window_placement.hpp"
#include "everygraph/presentation/workbench_window.hpp"

#include "fake_project.hpp"

#if defined(Q_OS_MACOS)
#include "native_pinyin_ime.hpp"
#endif

namespace {

using everygraph::application::ExpectedRevisions;
using everygraph::application::ParameterMap;
using everygraph::desktop::FakeProject;
using everygraph::desktop::FakeProjectController;
using everygraph::desktop::fake_ids::kSpectrumDataset;
using everygraph::foundation::ObjectId;
using everygraph::presentation::AppearanceMode;
using everygraph::presentation::CommandQuery;
using everygraph::presentation::DetachedDocumentWindow;
using everygraph::presentation::DocumentHost;
using everygraph::presentation::PreferencesDialog;
using everygraph::presentation::ShellAppearance;
using everygraph::presentation::ShellCommandDispatcher;
using everygraph::presentation::WindowPlacement;
using everygraph::presentation::WorkbenchBindings;
using everygraph::presentation::WorkbenchWindow;

struct AccessibilityHarness {
    FakeProject project;
    ShellCommandDispatcher dispatcher;
    FakeProjectController controller;
    CommandQuery query;
    WorkbenchBindings bindings;
    std::unique_ptr<WorkbenchWindow> window;

    AccessibilityHarness()
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

class InputMethodRecorder : public QObject {
public:
    QStringList preedits;
    QStringList commits;

    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() == QEvent::InputMethod) {
            auto* input_method = static_cast<QInputMethodEvent*>(event);
            if (!input_method->preeditString().isEmpty()) {
                preedits.append(input_method->preeditString());
            }
            if (!input_method->commitString().isEmpty()) {
                commits.append(input_method->commitString());
            }
        }
        return QObject::eventFilter(watched, event);
    }
};

QString greeting() {
    return QString(QChar(static_cast<char16_t>(0x4F60))) +
           QChar(static_cast<char16_t>(0x597D));
}

QString screenshot_dir() {
    return qEnvironmentVariable("EVERYGRAPH_SCREENSHOT_DIR");
}

QString screenshot_format() {
    // The qualified Qt build does not provide every image format plugin, so the
    // evidence writer picks the first format the runtime can actually save.
    static const QString format = [] {
        const QList<QByteArray> supported = QImageWriter::supportedImageFormats();
        for (const char* candidate : {"PNG", "BMP", "GIF"}) {
            if (supported.contains(QByteArray(candidate))) {
                return QString::fromLatin1(candidate).toLower();
            }
        }
        return QStringLiteral("bmp");
    }();
    return format;
}

void save_widget_screenshot(QWidget* widget, const QString& file_name) {
    const QString directory = screenshot_dir();
    if (directory.isEmpty() || widget == nullptr) {
        return;
    }
    QVERIFY2(QDir().mkpath(directory), qPrintable(directory));
    const QPixmap pixmap = widget->grab();
    QVERIFY2(!pixmap.isNull(), qPrintable(file_name));
    const QFileInfo info(file_name);
    const QString path = QDir(directory).filePath(
        QStringLiteral("%1.%2").arg(info.completeBaseName(), screenshot_format()));
    const QByteArray format = screenshot_format().toUpper().toLatin1();
    QVERIFY2(pixmap.save(path, format.constData()), qPrintable(path));
}

double linear_channel(double value) {
    return value <= 0.03928 ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4);
}

double relative_luminance(const QColor& color) {
    return 0.2126 * linear_channel(color.redF()) +
           0.7152 * linear_channel(color.greenF()) +
           0.0722 * linear_channel(color.blueF());
}

double contrast_ratio(const QColor& first, const QColor& second) {
    const double first_luminance = relative_luminance(first);
    const double second_luminance = relative_luminance(second);
    const double lighter = std::max(first_luminance, second_luminance);
    const double darker = std::min(first_luminance, second_luminance);
    return (lighter + 0.05) / (darker + 0.05);
}

QString widget_label(QWidget* widget) {
    if (widget == nullptr) {
        return QStringLiteral("<null>");
    }
    if (!widget->objectName().isEmpty()) {
        return widget->objectName();
    }
    return QStringLiteral("<%1>").arg(QString::fromLatin1(widget->metaObject()->className()));
}

// A Tab walk that never starts is a window-server condition, so the failure has to
// say whether the top level was active and what held focus.
QString activation_state(QWidget* top_level) {
    return QStringLiteral("active: %1, focused: %2")
        .arg(top_level->isActiveWindow() ? QStringLiteral("yes") : QStringLiteral("no"),
             widget_label(QApplication::focusWidget()));
}

QStringList clipped_text_widgets(QWidget* top_level) {
    QStringList clipped;
    if (top_level == nullptr) {
        return clipped;
    }
    for (QWidget* widget : top_level->findChildren<QWidget*>()) {
        auto* label = qobject_cast<QLabel*>(widget);
        auto* button = qobject_cast<QAbstractButton*>(widget);
        if ((label == nullptr && button == nullptr) || !widget->isVisible() ||
            widget->width() <= 0 || widget->height() <= 0) {
            continue;
        }
        const QString text = label != nullptr ? label->text() : button->text();
        if (text.isEmpty()) {
            continue;
        }
        // Measured with font metrics instead of heightForWidth: that hook only
        // answers for a widget whose layout claims it, so it says nothing about a
        // button, and a label answers for its own preferred size rather than the
        // room the widget actually has.
        int needed_width = 0;
        int needed_height = 0;
        if (label != nullptr && label->hasHeightForWidth()) {
            const QFontMetrics metrics(label->font());
            needed_height = metrics.boundingRect(QRect(0, 0, widget->width(), 0),
                                                 Qt::TextWordWrap | Qt::AlignLeft, text)
                                .height();
            needed_width = widget->width();
        } else if (label != nullptr) {
            const QFontMetrics metrics(label->font());
            needed_width = metrics.horizontalAdvance(text);
            needed_height = metrics.height();
        } else {
            // A button's style hint already folds its icon, margins and focus
            // rect into the extent it needs.
            needed_width = button->sizeHint().width();
            needed_height = button->sizeHint().height();
        }
        if (needed_height > widget->height() + 1 || needed_width > widget->width() + 1) {
            clipped.append(QStringLiteral("%1 (%2x%3 needs %4x%5)")
                               .arg(widget_label(widget))
                               .arg(widget->width())
                               .arg(widget->height())
                               .arg(needed_width)
                               .arg(needed_height));
        }
    }
    return clipped;
}

QStringList visible_focus_chain(QWidget* start) {
    QStringList chain;
    if (start == nullptr) {
        return chain;
    }
    QWidget* widget = start;
    do {
        if (widget->focusPolicy() != Qt::NoFocus && widget->isVisible() &&
            widget->isEnabled()) {
            chain.append(widget_label(widget));
        }
        widget = widget->nextInFocusChain();
    } while (widget != nullptr && widget != start && chain.size() < 200);
    return chain;
}

QStringList walk_focus(QWidget* top_level, QWidget* start, int max_steps) {
    QStringList order;
    if (top_level == nullptr || start == nullptr) {
        return order;
    }
    // macOS finishes activating a window after show() returns, and a Tab click sent
    // to a window that is not yet key never moves focus, so the walk waits for the
    // window server instead of assuming one fixed delay covers it.
    for (int attempt = 0; attempt < 40 && QApplication::focusWidget() != start; ++attempt) {
        top_level->activateWindow();
        start->setFocus(Qt::OtherFocusReason);
        QTest::qWait(25);
    }
    if (QApplication::focusWidget() != start) {
        return order;
    }
    QWidget* previous = nullptr;
    for (int step = 0; step < max_steps; ++step) {
        QWidget* current = QApplication::focusWidget();
        if (current == nullptr) {
            order.append(QStringLiteral("<focus-lost>"));
            break;
        }
        if (current != top_level && !top_level->isAncestorOf(current)) {
            order.append(QStringLiteral("<outside-window>"));
            break;
        }
        if (current == previous) {
            order.append(QStringLiteral("<stuck>"));
            break;
        }
        order.append(widget_label(current));
        previous = current;
        QTest::keyClick(current, Qt::Key_Tab);
        QCoreApplication::processEvents();
        if (QApplication::focusWidget() == start && step >= 3) {
            order.append(widget_label(start));
            break;
        }
    }
    return order;
}

void verify_widget_accessibility(QWidget* widget, const QString& name,
                                 bool require_description) {
    QVERIFY2(widget != nullptr, qPrintable(name));
    QAccessibleInterface* interface = QAccessible::queryAccessibleInterface(widget);
    QVERIFY2(interface != nullptr, qPrintable(name));
    QVERIFY2(interface->role() != QAccessible::NoRole, qPrintable(name));
    QCOMPARE(interface->text(QAccessible::Name), name);
    if (require_description) {
        QVERIFY2(!interface->text(QAccessible::Description).isEmpty(), qPrintable(name));
    }
}

}  // namespace

class AccessibilityTests : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void shell_exposes_accessible_names_and_roles();
    void focus_order_cycles_without_trap();
    void preferences_focus_order_is_explicit();
    void contrast_meets_aa_and_states_have_non_color_cues();
    void ime_composition_commits_only_after_confirmation();
    void scaling_keeps_controls_usable();
    void display_topology_restores_detached_windows();
    void native_pinyin_ime_composes_text();
};

void AccessibilityTests::initTestCase() {
    QVERIFY(QGuiApplication::primaryScreen() != nullptr);
    qInfo() << "platform:" << QGuiApplication::platformName()
            << "devicePixelRatio:" << QGuiApplication::primaryScreen()->devicePixelRatio()
            << "screens:" << QGuiApplication::screens().size()
            << "accessibilityActive:" << QAccessible::isActive();
}

void AccessibilityTests::shell_exposes_accessible_names_and_roles() {
    AccessibilityHarness harness;
    WorkbenchWindow& window = *harness.window;
    window.show();
    QTest::qWait(30);

    verify_widget_accessibility(&window, QStringLiteral("EveryGraph Workbench"), true);
    verify_widget_accessibility(window.menuBar(), QStringLiteral("Main menu"), true);
    verify_widget_accessibility(window.statusBar(), QStringLiteral("Status bar"), true);
    verify_widget_accessibility(window.workbench_toolbar(),
                                 QStringLiteral("Workbench toolbar"), true);
    verify_widget_accessibility(window.project_tree(), QStringLiteral("Project tree"), true);
    verify_widget_accessibility(window.properties_panel(),
                                 QStringLiteral("Properties panel"), true);
    verify_widget_accessibility(window.function_library_panel(),
                                 QStringLiteral("Function library"), true);
    verify_widget_accessibility(window.document_host(), QStringLiteral("Document area"), true);
    verify_widget_accessibility(window.document_host()->tab_widget(),
                                 QStringLiteral("Open documents"), true);
    // Qt names the tab bar from its current tab and exposes no description for it,
    // so the per-tab names carry the document identity.
    QAccessibleInterface* tab_bar = QAccessible::queryAccessibleInterface(
        window.document_host()->tab_widget()->tabBar());
    QVERIFY(tab_bar != nullptr);
    QCOMPARE(tab_bar->role(), QAccessible::PageTabList);
    QCOMPARE(tab_bar->text(QAccessible::Name),
             QStringLiteral("Welcome, note document"));
    // Qt's dock interface takes the pane name from the visible title and exposes no
    // description, so the panels inside each dock carry the descriptive metadata.
    const std::vector<std::pair<QDockWidget*, QString>> docks{
        {window.project_dock(), QStringLiteral("Project")},
        {window.properties_dock(), QStringLiteral("Properties")},
        {window.function_library_dock(), QStringLiteral("Function Library")}};
    for (const auto& dock : docks) {
        QVERIFY2(dock.first != nullptr, qPrintable(dock.second));
        QAccessibleInterface* dock_interface =
            QAccessible::queryAccessibleInterface(dock.first);
        QVERIFY2(dock_interface != nullptr, qPrintable(dock.second));
        QCOMPARE(dock_interface->role(), QAccessible::Pane);
        QCOMPARE(dock_interface->text(QAccessible::Name), dock.second);
        QVERIFY2(dock.first->widget() != nullptr, qPrintable(dock.second));
    }

    QAccessibleInterface* tree = QAccessible::queryAccessibleInterface(window.project_tree());
    QCOMPARE(tree->role(), QAccessible::Tree);
    QAccessibleInterface* list =
        QAccessible::queryAccessibleInterface(window.function_library_panel()
                                                  ->findChild<QListWidget*>(
                                                      QStringLiteral("functionLibraryList")));
    QVERIFY(list != nullptr);
    QCOMPARE(list->role(), QAccessible::List);
    QCOMPARE(list->text(QAccessible::Name),
             QStringLiteral("Placeholder analysis functions"));
    QVERIFY(!list->text(QAccessible::Description).isEmpty());

    QLabel* values = window.findChild<QLabel*>(QStringLiteral("propertiesValues"));
    QVERIFY(values != nullptr);
    QAccessibleInterface* values_interface = QAccessible::queryAccessibleInterface(values);
    QVERIFY(values_interface != nullptr);
    QCOMPARE(values_interface->text(QAccessible::Name), QStringLiteral("Property values"));

    int toolbar_buttons = 0;
    for (QAction* action : window.workbench_toolbar()->actions()) {
        if (action->isSeparator()) {
            continue;
        }
        ++toolbar_buttons;
        auto* button = qobject_cast<QToolButton*>(
            window.workbench_toolbar()->widgetForAction(action));
        QVERIFY2(button != nullptr, qPrintable(action->objectName()));
        QCOMPARE(button->accessibleName(), action->text());
        QVERIFY2(!button->accessibleDescription().isEmpty(), qPrintable(action->objectName()));
        QAccessibleInterface* button_interface = QAccessible::queryAccessibleInterface(button);
        QVERIFY(button_interface != nullptr);
        QVERIFY(button_interface->role() != QAccessible::NoRole);
        QCOMPARE(button_interface->text(QAccessible::Name), action->text());
    }
    QVERIFY(toolbar_buttons >= 9);

    QVERIFY(window.document_host()->detach_document("welcome"));
    DetachedDocumentWindow* detached =
        window.document_host()->detached_window("welcome");
    QVERIFY(detached != nullptr);
    QTRY_VERIFY(detached->isVisible());
    verify_widget_accessibility(
        detached, QStringLiteral("Welcome (detached document)"), true);
    QAccessibleInterface* detached_interface =
        QAccessible::queryAccessibleInterface(detached);
    QVERIFY(detached_interface != nullptr);
    QVERIFY(detached_interface->role() != QAccessible::NoRole);

    PreferencesDialog dialog(window.preferences());
    dialog.show();
    QTest::qWait(20);
    verify_widget_accessibility(&dialog, QStringLiteral("EveryGraph preferences"), true);
    QAccessibleInterface* dialog_interface = QAccessible::queryAccessibleInterface(&dialog);
    QCOMPARE(dialog_interface->role(), QAccessible::Dialog);

    const QStringList control_names{
        QStringLiteral("appearanceCombo"), QStringLiteral("focusModeCombo"),
        QStringLiteral("showFunctionLibraryCheck"),
        QStringLiteral("showProjectPanelCheck"),
        QStringLiteral("showPropertiesPanelCheck"),
        QStringLiteral("showToolbarCheck"),
        QStringLiteral("defaultFileFormatCombo")};
    for (const QString& control_name : control_names) {
        QWidget* control = dialog.findChild<QWidget*>(control_name);
        QVERIFY2(control != nullptr, qPrintable(control_name));
        QVERIFY2(!control->accessibleName().isEmpty(), qPrintable(control_name));
        QVERIFY2(!control->accessibleDescription().isEmpty(), qPrintable(control_name));
        QAccessibleInterface* control_interface =
            QAccessible::queryAccessibleInterface(control);
        QVERIFY(control_interface != nullptr);
        QVERIFY(control_interface->role() != QAccessible::NoRole);
    }

    auto* button_box = dialog.findChild<QDialogButtonBox*>(
        QStringLiteral("preferencesButtonBox"));
    QVERIFY(button_box != nullptr);
    QVERIFY(!button_box->accessibleName().isEmpty());
    QVERIFY(!button_box->accessibleDescription().isEmpty());
    for (QAbstractButton* button : button_box->buttons()) {
        QVERIFY2(!button->text().isEmpty(), qPrintable(button->objectName()));
        QVERIFY2(!button->accessibleDescription().isEmpty(),
                 qPrintable(button->text()));
    }
}

void AccessibilityTests::focus_order_cycles_without_trap() {
    AccessibilityHarness harness;
    WorkbenchWindow& window = *harness.window;
    window.resize(1100, 720);
    window.show();
    QTest::qWait(30);

    const QStringList chain = visible_focus_chain(window.project_tree());
    QVERIFY2(chain.size() >= 4, qPrintable(chain.join(QStringLiteral(", "))));
    for (const QString& expected : {QStringLiteral("projectTree"),
                                    QStringLiteral("documentTabBar"),
                                    QStringLiteral("functionLibraryList"),
                                    QStringLiteral("propertiesPanel")}) {
        QVERIFY2(chain.contains(expected), qPrintable(expected + QStringLiteral(" in ") +
                                                     chain.join(QStringLiteral(", "))));
    }
    QVERIFY(chain.indexOf(QStringLiteral("projectTree")) <
            chain.indexOf(QStringLiteral("documentTabBar")));

    const QStringList order = walk_focus(&window, window.project_tree(), 40);
    qInfo().noquote() << "raw shell focus walk:" << order.join(QStringLiteral(" -> "));
    QVERIFY2(!order.isEmpty(),
             qPrintable(QStringLiteral("the walk never reached the project tree, ") +
                        activation_state(&window)));
    QVERIFY2(order.size() >= 4, qPrintable(order.join(QStringLiteral(" -> "))));
    QVERIFY2(order.first() == order.last(),
             qPrintable(QStringLiteral("focus did not return to the start: ") +
                        order.join(QStringLiteral(" -> "))));
    for (const QString& expected : {QStringLiteral("projectTree"),
                                    QStringLiteral("documentTabBar"),
                                    QStringLiteral("functionLibraryList"),
                                    QStringLiteral("propertiesPanel")}) {
        QVERIFY2(order.contains(expected), qPrintable(expected + QStringLiteral(" in ") +
                                                     order.join(QStringLiteral(" -> "))));
    }
    qInfo().noquote() << "focus cycle:" << order.join(QStringLiteral(" -> "));

    QVERIFY2(clipped_text_widgets(&window).isEmpty(),
             qPrintable(clipped_text_widgets(&window).join(QStringLiteral(", "))));
    window.document_host()->tab_widget()->tabBar()->setFocus(Qt::TabFocusReason);
    QTest::qWait(20);
    save_widget_screenshot(
        &window, QStringLiteral("focus-on-document-tabs-%1.png")
                     .arg(QGuiApplication::platformName()));
}

void AccessibilityTests::preferences_focus_order_is_explicit() {
    AccessibilityHarness harness;
    PreferencesDialog dialog(harness.window->preferences());
    dialog.show();
    QTest::qWait(30);

    auto* appearance = dialog.findChild<QComboBox*>(QStringLiteral("appearanceCombo"));
    auto* focus_mode = dialog.findChild<QComboBox*>(QStringLiteral("focusModeCombo"));
    auto* function_library =
        dialog.findChild<QCheckBox*>(QStringLiteral("showFunctionLibraryCheck"));
    auto* project =
        dialog.findChild<QCheckBox*>(QStringLiteral("showProjectPanelCheck"));
    auto* properties =
        dialog.findChild<QCheckBox*>(QStringLiteral("showPropertiesPanelCheck"));
    auto* toolbar = dialog.findChild<QCheckBox*>(QStringLiteral("showToolbarCheck"));
    auto* default_format =
        dialog.findChild<QComboBox*>(QStringLiteral("defaultFileFormatCombo"));
    QVERIFY(appearance != nullptr);
    QVERIFY(focus_mode != nullptr);
    QVERIFY(function_library != nullptr);
    QVERIFY(project != nullptr);
    QVERIFY(properties != nullptr);
    QVERIFY(toolbar != nullptr);
    QVERIFY(default_format != nullptr);

    const QStringList chain = visible_focus_chain(appearance);
    QVERIFY2(chain.size() >= 7, qPrintable(chain.join(QStringLiteral(", "))));
    const QStringList explicit_order{
        QStringLiteral("appearanceCombo"), QStringLiteral("focusModeCombo"),
        QStringLiteral("showFunctionLibraryCheck"),
        QStringLiteral("showProjectPanelCheck"),
        QStringLiteral("showPropertiesPanelCheck"),
        QStringLiteral("showToolbarCheck"), QStringLiteral("defaultFileFormatCombo")};
    for (const QString& name : explicit_order) {
        QVERIFY2(chain.contains(name), qPrintable(name + QStringLiteral(" in ") +
                                                 chain.join(QStringLiteral(", "))));
    }
    QVERIFY(chain.indexOf(QStringLiteral("appearanceCombo")) <
            chain.indexOf(QStringLiteral("focusModeCombo")));
    QVERIFY(chain.indexOf(QStringLiteral("focusModeCombo")) <
            chain.indexOf(QStringLiteral("showFunctionLibraryCheck")));
    QVERIFY(chain.indexOf(QStringLiteral("showToolbarCheck")) <
            chain.indexOf(QStringLiteral("defaultFileFormatCombo")));

    // On macOS, Qt gives non-editable combos and dialog buttons a TabFocus-only
    // policy and the platform leaves them out of the tab chain until the user turns
    // on System Settings > Keyboard > Keyboard navigation. Promoting them for this
    // walk emulates the enabled state so the declared order gets a real key walk.
    for (QWidget* control : dialog.findChildren<QWidget*>()) {
        if (control->focusPolicy() == Qt::TabFocus) {
            control->setFocusPolicy(Qt::StrongFocus);
        }
    }

    const QStringList order = walk_focus(&dialog, appearance, 30);
    qInfo().noquote() << "raw preferences focus walk:" << order.join(QStringLiteral(" -> "));
    QVERIFY2(!order.isEmpty(),
             qPrintable(QStringLiteral("the walk never reached the appearance combo, ") +
                        activation_state(&dialog)));
    QVERIFY2(order.size() >= 7, qPrintable(order.join(QStringLiteral(" -> "))));
    QVERIFY2(order.first() == order.last(),
             qPrintable(QStringLiteral("focus did not return to the start: ") +
                        order.join(QStringLiteral(" -> "))));
    for (const QString& name : explicit_order) {
        QVERIFY2(order.contains(name), qPrintable(name + QStringLiteral(" in ") +
                                                 order.join(QStringLiteral(" -> "))));
    }
    qInfo().noquote() << "preferences focus cycle:"
                      << order.join(QStringLiteral(" -> "));

    QVERIFY2(clipped_text_widgets(&dialog).isEmpty(),
             qPrintable(clipped_text_widgets(&dialog).join(QStringLiteral(", "))));
    default_format->setFocus(Qt::TabFocusReason);
    QTest::qWait(20);
    save_widget_screenshot(
        &dialog, QStringLiteral("focus-on-default-format-%1.png")
                     .arg(QGuiApplication::platformName()));
}

void AccessibilityTests::contrast_meets_aa_and_states_have_non_color_cues() {
    struct ContrastPair {
        QPalette::ColorRole foreground;
        QPalette::ColorRole background;
        const char* label;
    };
    const std::vector<ContrastPair> pairs{
        {QPalette::WindowText, QPalette::Window, "window text"},
        {QPalette::Text, QPalette::Base, "base text"},
        {QPalette::Text, QPalette::AlternateBase, "alternate text"},
        {QPalette::ButtonText, QPalette::Button, "button text"},
        {QPalette::HighlightedText, QPalette::Highlight, "selected text"},
        {QPalette::Link, QPalette::Window, "link text"}};

    for (const auto& [name, palette] :
         std::vector<std::pair<const char*, QPalette>>{{"light", ShellAppearance::light_palette()},
                                                       {"dark", ShellAppearance::dark_palette()}}) {
        QStringList measured;
        for (const ContrastPair& pair : pairs) {
            const double ratio = contrast_ratio(palette.color(pair.foreground),
                                                palette.color(pair.background));
            measured << QStringLiteral("%1 %2:1").arg(QString::fromLatin1(pair.label)).arg(
                ratio, 0, 'f', 2);
            QVERIFY2(ratio >= 4.5,
                     qPrintable(QStringLiteral("%1 %2 contrast %3 is below 4.5:1")
                                    .arg(QString::fromLatin1(name),
                                         QString::fromLatin1(pair.label))
                                    .arg(ratio, 0, 'f', 2)));
        }
        qInfo().noquote() << QString::fromLatin1(name) << "contrast:"
                          << measured.join(QStringLiteral("; "));
    }
    QVERIFY(ShellAppearance::dark_palette().color(QPalette::Highlight) !=
            QColor(QStringLiteral("#3d7ebd")));
    const double dark_highlight = contrast_ratio(
        ShellAppearance::dark_palette().color(QPalette::HighlightedText),
        ShellAppearance::dark_palette().color(QPalette::Highlight));
    QVERIFY2(dark_highlight >= 4.5,
             qPrintable(QStringLiteral("dark selected text contrast %1")
                            .arg(dark_highlight, 0, 'f', 2)));

    AccessibilityHarness harness;
    WorkbenchWindow& window = *harness.window;
    window.show();
    QTest::qWait(30);
    QVERIFY(!window.statusBar()->currentMessage().isEmpty());
    QTabBar* tab_bar = window.document_host()->tab_widget()->tabBar();
    QVERIFY(!tab_bar->tabToolTip(0).isEmpty());
    QVERIFY(!tab_bar->accessibleTabName(0).isEmpty());
    QAction* data_focus = window.find_action(QStringLiteral("dataFocusAction"));
    QVERIFY(data_focus != nullptr);
    QVERIFY(data_focus->isCheckable());
    QVERIFY(!data_focus->statusTip().isEmpty());
    QVERIFY(window.select_object(kSpectrumDataset));
    QCOMPARE(window.properties_panel()->object_id_text(), QStringLiteral("4"));
    QCOMPARE(window.properties_panel()->object_type_text(), QStringLiteral("dataset"));

    everygraph::presentation::Preferences dark = window.preferences();
    dark.interface.appearance = AppearanceMode::dark;
    window.apply_preferences(dark);
    QCOMPARE(window.property("everygraphAppearance").toString(), QStringLiteral("dark"));
    QCOMPARE(window.menuBar()->palette().color(QPalette::Window),
             ShellAppearance::dark_palette().color(QPalette::Window));
    QCOMPARE(window.statusBar()->palette().color(QPalette::Window),
             ShellAppearance::dark_palette().color(QPalette::Window));
    QCOMPARE(window.project_dock()->palette().color(QPalette::Window),
             ShellAppearance::dark_palette().color(QPalette::Window));
    QTest::qWait(20);
    save_widget_screenshot(&window, QStringLiteral("workbench-dark.png"));

    everygraph::presentation::Preferences light = dark;
    light.interface.appearance = AppearanceMode::light;
    window.apply_preferences(light);
    QCOMPARE(window.menuBar()->palette().color(QPalette::Window),
             ShellAppearance::light_palette().color(QPalette::Window));
    QTest::qWait(20);
    save_widget_screenshot(&window, QStringLiteral("workbench-light.png"));
}

void AccessibilityTests::ime_composition_commits_only_after_confirmation() {
    QWidget host;
    host.resize(420, 120);
    auto* layout = new QVBoxLayout(&host);
    auto* edit = new QLineEdit(&host);
    edit->setObjectName(QStringLiteral("imeFixtureEdit"));
    layout->addWidget(edit);
    host.show();
    QTest::qWait(20);
    edit->setFocus(Qt::OtherFocusReason);
    QCoreApplication::processEvents();

    InputMethodRecorder recorder;
    edit->installEventFilter(&recorder);

    QInputMethodEvent preedit(QStringLiteral("ni hao"),
                              {QInputMethodEvent::Attribute(
                                  QInputMethodEvent::TextFormat, 0, 6, QVariant())});
    QApplication::sendEvent(edit, &preedit);
    QCOMPARE(edit->text(), QString());
    QVERIFY(recorder.preedits.contains(QStringLiteral("ni hao")));

    QInputMethodEvent commit;
    commit.setCommitString(greeting());
    QApplication::sendEvent(edit, &commit);
    QCOMPARE(edit->text(), greeting());
    QVERIFY(recorder.commits.contains(greeting()));

    edit->clear();
    QInputMethodEvent cancelled(QStringLiteral("hao"),
                                 {QInputMethodEvent::Attribute(
                                     QInputMethodEvent::TextFormat, 0, 3, QVariant())});
    QApplication::sendEvent(edit, &cancelled);
    QInputMethodEvent cancel;
    cancel.setCommitString(QString());
    QApplication::sendEvent(edit, &cancel);
    QCOMPARE(edit->text(), QString());
    edit->removeEventFilter(&recorder);
}

void AccessibilityTests::scaling_keeps_controls_usable() {
    const QString expected_text = qEnvironmentVariable("EVERYGRAPH_EXPECTED_SCALE");
    if (expected_text.isEmpty()) {
        QSKIP("the scale check runs through the dedicated 1x/2x CTest entries");
    }
    const qreal expected_scale = expected_text.toDouble();
    const qreal device_ratio = QGuiApplication::primaryScreen()->devicePixelRatio();
    QVERIFY2(std::abs(device_ratio - expected_scale) < 0.01,
             qPrintable(QStringLiteral("expected scale %1 but the screen reports %2")
                            .arg(expected_scale)
                            .arg(device_ratio)));

    AccessibilityHarness harness;
    WorkbenchWindow& window = *harness.window;
    window.resize(1200, 760);
    window.show();
    QTest::qWait(50);
    QCOMPARE(window.size(), QSize(1200, 760));
    QVERIFY2(clipped_text_widgets(&window).isEmpty(),
             qPrintable(clipped_text_widgets(&window).join(QStringLiteral(", "))));

    QToolBar* toolbar = window.workbench_toolbar();
    QCOMPARE(toolbar->iconSize(), QSize(16, 16));
    for (QAction* action : toolbar->actions()) {
        if (action->isSeparator()) {
            continue;
        }
        auto* button = qobject_cast<QToolButton*>(toolbar->widgetForAction(action));
        QVERIFY2(button != nullptr, qPrintable(action->objectName()));
        QVERIFY2(button->isVisible(), qPrintable(action->objectName()));
        QVERIFY2(button->width() >= button->sizeHint().width() - 1,
                 qPrintable(QStringLiteral("%1 width %2 below hint %3")
                                .arg(action->objectName())
                                .arg(button->width())
                                .arg(button->sizeHint().width())));
        // A command that lost its icon would still pass a size check, so the
        // presence of the icon is asserted separately. Whether its bitmap really
        // carries 2x detail is not checked here.
        QVERIFY2(!action->icon().isNull(), qPrintable(action->objectName()));
    }

    QTabBar* tab_bar = window.document_host()->tab_widget()->tabBar();
    QVERIFY(tab_bar->count() >= 1);
    const QRect tab_rect = tab_bar->tabRect(0);
    const QFontMetrics metrics(tab_bar->font());
    QVERIFY2(tab_rect.width() >= metrics.horizontalAdvance(tab_bar->tabText(0)),
             qPrintable(QStringLiteral("tab width %1 below text width %2")
                            .arg(tab_rect.width())
                            .arg(metrics.horizontalAdvance(tab_bar->tabText(0)))));
    QVERIFY(tab_rect.height() >= metrics.height());

    QLabel* format_label =
        window.findChild<QLabel*>(QStringLiteral("defaultFormatLabel"));
    QVERIFY(format_label != nullptr);
    QVERIFY(format_label->width() >= format_label->sizeHint().width() - 1);
    QVERIFY(format_label->height() >= format_label->sizeHint().height() - 1);

    QVERIFY(window.project_dock()->width() > 0);
    QVERIFY(window.properties_dock()->width() > 0);
    QVERIFY(window.function_library_dock()->width() > 0);
    QVERIFY(window.document_host()->width() > 0);
    QVERIFY(window.document_host()->height() > 0);

    save_widget_screenshot(
        &window, QStringLiteral("workbench-%1x.png").arg(expected_text));
}

void AccessibilityTests::display_topology_restores_detached_windows() {
    AccessibilityHarness harness;
    WorkbenchWindow& window = *harness.window;
    window.show();
    QTest::qWait(30);
    DocumentHost* host = window.document_host();
    QVERIFY(host->detach_document("welcome"));
    DetachedDocumentWindow* detached = host->detached_window("welcome");
    QVERIFY(detached != nullptr);
    QTRY_VERIFY(detached->isVisible());

    const std::vector<QRect> primary{QRect(0, 0, 1920, 1080)};
    const std::vector<QRect> dual{QRect(0, 0, 1920, 1080), QRect(1920, 0, 1920, 1080)};
    const std::vector<QRect> left_of_primary{QRect(-1920, 0, 1920, 1080),
                                             QRect(0, 0, 1920, 1080)};

    const QRect on_secondary(2200, 300, 700, 500);
    detached->setGeometry(on_secondary);
    QTest::qWait(20);
    host->restore_detached_windows(dual);
    QCOMPARE(detached->geometry(), on_secondary);

    host->restore_detached_windows(primary);
    QVERIFY(primary.front().intersects(detached->frameGeometry()));
    const QRect visible_on_primary =
        primary.front().intersected(detached->frameGeometry());
    QVERIFY(visible_on_primary.width() >= 64);
    QVERIFY(visible_on_primary.height() >= 32);
    QCOMPARE(detached->size(), on_secondary.size());

    const QRect on_left_screen(-1700, 200, 700, 500);
    detached->setGeometry(on_left_screen);
    QTest::qWait(20);
    host->restore_detached_windows(left_of_primary);
    const QRect left_frame = detached->frameGeometry();
    QVERIFY2(left_of_primary.front().contains(left_frame),
             qPrintable(QStringLiteral("detached frame x=%1 y=%2 w=%3 h=%4")
                            .arg(left_frame.x())
                            .arg(left_frame.y())
                            .arg(left_frame.width())
                            .arg(left_frame.height())));
    QCOMPARE(detached->size(), on_left_screen.size());

    detached->setGeometry(QRect(5000, 4000, 640, 480));
    QTest::qWait(20);
    host->restore_detached_windows(primary);
    QVERIFY(primary.front().intersects(detached->frameGeometry()));

    QCOMPARE(WindowPlacement::restore_to_visible(on_secondary, dual), on_secondary);
    QVERIFY(primary.front().contains(
        WindowPlacement::restore_to_visible(on_secondary, primary)));
    QCOMPARE(WindowPlacement::restore_to_visible(on_left_screen, left_of_primary),
             on_left_screen);
    QVERIFY(primary.front().contains(
        WindowPlacement::restore_to_visible(on_left_screen, primary)));

    QVERIFY(host->dock_document("welcome"));
    QCOMPARE(host->tab_count(), 1);
    QCOMPARE(host->document_count(), 1);
}

void AccessibilityTests::native_pinyin_ime_composes_text() {
#if defined(Q_OS_MACOS)
    if (QGuiApplication::platformName() != QStringLiteral("cocoa")) {
        QSKIP("the native Pinyin check requires the cocoa platform");
    }
    QWidget host;
    host.resize(560, 160);
    auto* layout = new QVBoxLayout(&host);
    auto* edit = new QLineEdit(&host);
    edit->setObjectName(QStringLiteral("nativeImeEdit"));
    layout->addWidget(edit);
    host.show();
    host.raise();
    host.activateWindow();
    edit->setFocus(Qt::OtherFocusReason);
    QTest::qWait(300);

    const everygraph::test::NativePinyinResult result =
        everygraph::test::run_native_pinyin_ime(host, *edit);
    qInfo().noquote() << "native IME source:" << result.input_source_id
                      << "preedits:" << result.preedit_steps.join(QStringLiteral(" | "))
                      << "commit:" << result.committed_text
                      << "error:" << result.error;
    if (!result.available) {
        if (result.error.contains(QStringLiteral("not enabled")) ||
            result.error.contains(QStringLiteral("not selectable")) ||
            result.error.contains(QStringLiteral("not the current input source"))) {
            QSKIP(qPrintable(result.error));
        }
        QFAIL(qPrintable(result.error));
    }
    QVERIFY2(result.composed, qPrintable(result.error));
    QVERIFY2(!result.preedit_steps.isEmpty(),
             "the input method must expose intermediate preedit text");
    QCOMPARE(result.committed_text, greeting());
    QCOMPARE(edit->text(), greeting());
    save_widget_screenshot(&host, QStringLiteral("native-pinyin-window.png"));
    QTest::qWait(20);
    host.hide();
#else
    QSKIP("the native Pinyin helper is macOS-only");
#endif
}

int main(int argc, char* argv[]) {
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }
    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("EveryGraph"));
    QCoreApplication::setApplicationName(QStringLiteral("EveryGraph Accessibility Tests"));
    AccessibilityTests tests;
    return QTest::qExec(&tests, argc, argv);
}

#include "accessibility_tests.moc"
