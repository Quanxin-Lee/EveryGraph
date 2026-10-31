#include "everygraph/presentation/preferences_dialog.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFontMetrics>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <utility>

namespace everygraph::presentation {
namespace {

int combo_index_for_data(const QComboBox& combo, int data) {
    const int index = combo.findData(data);
    return index >= 0 ? index : 0;
}

// QFormLayout sizes a wrapped label from its single-line hint, which clips the
// second line. The note keeps the height its text needs at the width it is
// guaranteed to keep, so the two stay consistent if the dialog is resized.
void reserve_wrapped_height(QLabel* note) {
    static constexpr int kNoteWidth = 360;
    const QFontMetrics metrics(note->font());
    const QRect bounds = metrics.boundingRect(QRect(0, 0, kNoteWidth, 0),
                                              Qt::TextWordWrap | Qt::AlignLeft,
                                              note->text());
    note->setMinimumWidth(kNoteWidth);
    note->setMinimumHeight(bounds.height());
}

}  // namespace

PreferencesDialog::PreferencesDialog(const Preferences& preferences, QWidget* parent)
    : QDialog(parent), preferences_(preferences) {
    setObjectName(QStringLiteral("preferencesDialog"));
    setWindowTitle(QStringLiteral("Preferences"));
    setAccessibleName(QStringLiteral("EveryGraph preferences"));
    setAccessibleDescription(
        QStringLiteral("Shell appearance, focus mode, panel visibility and default "
                       "format settings. Changes apply to this prototype shell only."));
    setModal(true);
    build_ui();
    sync_controls(preferences_);
    resize(440, 420);
}

void PreferencesDialog::build_ui() {
    auto* layout = new QVBoxLayout(this);

    auto* appearance_group = new QGroupBox(QStringLiteral("Appearance"), this);
    appearance_group->setAccessibleName(QStringLiteral("Appearance settings"));
    appearance_group->setAccessibleDescription(
        QStringLiteral("Interface appearance for the shell chrome only."));
    auto* appearance_form = new QFormLayout(appearance_group);
    appearance_ = new QComboBox(appearance_group);
    appearance_->setObjectName(QStringLiteral("appearanceCombo"));
    appearance_->setAccessibleName(QStringLiteral("Appearance mode"));
    appearance_->setAccessibleDescription(
        QStringLiteral("Choose System, Light or Dark shell chrome."));
    appearance_->addItem(QStringLiteral("System"),
                         static_cast<int>(AppearanceMode::system));
    appearance_->addItem(QStringLiteral("Light"), static_cast<int>(AppearanceMode::light));
    appearance_->addItem(QStringLiteral("Dark"), static_cast<int>(AppearanceMode::dark));
    appearance_form->addRow(QStringLiteral("Interface appearance:"), appearance_);
    auto* appearance_note = new QLabel(
        QStringLiteral("Appearance changes shell chrome only. Publication scene colors "
                       "and export backgrounds are not affected."),
        appearance_group);
    appearance_note->setAccessibleName(QStringLiteral("Appearance notice"));
    appearance_note->setAccessibleDescription(
        QStringLiteral("Explains that appearance does not affect publication colors."));
    appearance_note->setWordWrap(true);
    reserve_wrapped_height(appearance_note);
    appearance_form->addRow(appearance_note);
    layout->addWidget(appearance_group);

    auto* focus_group = new QGroupBox(QStringLiteral("Focus mode"), this);
    focus_group->setAccessibleName(QStringLiteral("Focus mode settings"));
    focus_group->setAccessibleDescription(
        QStringLiteral("Choose which panels are visible in the workbench."));
    auto* focus_form = new QFormLayout(focus_group);
    focus_mode_ = new QComboBox(focus_group);
    focus_mode_->setObjectName(QStringLiteral("focusModeCombo"));
    focus_mode_->setAccessibleName(QStringLiteral("Focus mode"));
    focus_mode_->setAccessibleDescription(
        QStringLiteral("Choose Normal, Data only or Commands only."));
    focus_mode_->addItem(QStringLiteral("Normal layout"), static_cast<int>(FocusMode::normal));
    focus_mode_->addItem(QStringLiteral("Data only"), static_cast<int>(FocusMode::data_only));
    focus_mode_->addItem(QStringLiteral("Commands only"),
                         static_cast<int>(FocusMode::commands_only));
    focus_form->addRow(QStringLiteral("Focus mode:"), focus_mode_);
    layout->addWidget(focus_group);

    auto* interface_group = new QGroupBox(QStringLiteral("Interface"), this);
    interface_group->setAccessibleName(QStringLiteral("Interface panel visibility"));
    interface_group->setAccessibleDescription(
        QStringLiteral("Show or hide the normal-layout panels and toolbar."));
    auto* interface_form = new QFormLayout(interface_group);
    show_function_library_ = new QCheckBox(QStringLiteral("Show the function library panel"),
                                           interface_group);
    show_function_library_->setObjectName(QStringLiteral("showFunctionLibraryCheck"));
    show_function_library_->setAccessibleName(QStringLiteral("Show function library"));
    show_function_library_->setAccessibleDescription(
        QStringLiteral("Show the function library dock in the normal layout."));
    interface_form->addRow(show_function_library_);
    show_project_panel_ =
        new QCheckBox(QStringLiteral("Show the project panel"), interface_group);
    show_project_panel_->setObjectName(QStringLiteral("showProjectPanelCheck"));
    show_project_panel_->setAccessibleName(QStringLiteral("Show project panel"));
    show_project_panel_->setAccessibleDescription(
        QStringLiteral("Show the project dock in the normal layout."));
    interface_form->addRow(show_project_panel_);
    show_properties_panel_ =
        new QCheckBox(QStringLiteral("Show the properties panel"), interface_group);
    show_properties_panel_->setObjectName(QStringLiteral("showPropertiesPanelCheck"));
    show_properties_panel_->setAccessibleName(QStringLiteral("Show properties panel"));
    show_properties_panel_->setAccessibleDescription(
        QStringLiteral("Show the properties dock in the normal layout."));
    interface_form->addRow(show_properties_panel_);
    show_toolbar_ = new QCheckBox(QStringLiteral("Show the command toolbar"), interface_group);
    show_toolbar_->setObjectName(QStringLiteral("showToolbarCheck"));
    show_toolbar_->setAccessibleName(QStringLiteral("Show command toolbar"));
    show_toolbar_->setAccessibleDescription(
        QStringLiteral("Show the command toolbar in the normal layout."));
    interface_form->addRow(show_toolbar_);
    layout->addWidget(interface_group);

    auto* defaults_group = new QGroupBox(QStringLiteral("Default format"), this);
    defaults_group->setAccessibleName(QStringLiteral("Default format settings"));
    defaults_group->setAccessibleDescription(
        QStringLiteral("Preferred format for new documents; it does not export."));
    auto* defaults_form = new QFormLayout(defaults_group);
    default_file_format_ = new QComboBox(defaults_group);
    default_file_format_->setObjectName(QStringLiteral("defaultFileFormatCombo"));
    default_file_format_->setAccessibleName(QStringLiteral("Default file format"));
    default_file_format_->setAccessibleDescription(
        QStringLiteral("Choose CSV, Parquet or EveryGraph project."));
    default_file_format_->addItem(QStringLiteral("CSV"),
                                  static_cast<int>(DefaultFileFormat::csv));
    default_file_format_->addItem(QStringLiteral("Parquet"),
                                  static_cast<int>(DefaultFileFormat::parquet));
    default_file_format_->addItem(QStringLiteral("EveryGraph project"),
                                  static_cast<int>(DefaultFileFormat::everygraph_project));
    defaults_form->addRow(QStringLiteral("Default file format:"), default_file_format_);
    auto* defaults_note = new QLabel(
        QStringLiteral("The default format is a preference only. This prototype does not "
                       "save projects or export files."),
        defaults_group);
    defaults_note->setAccessibleName(QStringLiteral("Default format notice"));
    defaults_note->setAccessibleDescription(
        QStringLiteral("Explains that the default format does not save or export files."));
    defaults_note->setWordWrap(true);
    reserve_wrapped_height(defaults_note);
    defaults_form->addRow(defaults_note);
    layout->addWidget(defaults_group);

    layout->addStretch(1);
    buttons_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel |
                                        QDialogButtonBox::RestoreDefaults,
                                    this);
    buttons_->setObjectName(QStringLiteral("preferencesButtonBox"));
    buttons_->setAccessibleName(QStringLiteral("Preferences actions"));
    buttons_->setAccessibleDescription(
        QStringLiteral("Apply, discard or restore the preferences."));
    QPushButton* ok_button = buttons_->button(QDialogButtonBox::Ok);
    QPushButton* cancel_button = buttons_->button(QDialogButtonBox::Cancel);
    QPushButton* restore_button = buttons_->button(QDialogButtonBox::RestoreDefaults);
    if (ok_button != nullptr) {
        ok_button->setAccessibleDescription(QStringLiteral("Apply the preferences and close."));
    }
    if (cancel_button != nullptr) {
        cancel_button->setAccessibleDescription(QStringLiteral("Discard the changes and close."));
    }
    if (restore_button != nullptr) {
        restore_button->setAccessibleDescription(
            QStringLiteral("Restore the built-in defaults and keep the saved layout."));
    }
    connect(buttons_, &QDialogButtonBox::accepted, this, &PreferencesDialog::accept);
    connect(buttons_, &QDialogButtonBox::rejected, this, &PreferencesDialog::reject);
    if (restore_button != nullptr) {
        connect(restore_button, &QPushButton::clicked, this,
                &PreferencesDialog::restore_defaults);
    }
    layout->addWidget(buttons_);

    setTabOrder(appearance_, focus_mode_);
    setTabOrder(focus_mode_, show_function_library_);
    setTabOrder(show_function_library_, show_project_panel_);
    setTabOrder(show_project_panel_, show_properties_panel_);
    setTabOrder(show_properties_panel_, show_toolbar_);
    setTabOrder(show_toolbar_, default_file_format_);
    if (ok_button != nullptr) {
        setTabOrder(default_file_format_, ok_button);
    }
}

void PreferencesDialog::sync_controls(const Preferences& preferences) {
    appearance_->setCurrentIndex(
        combo_index_for_data(*appearance_,
                             static_cast<int>(preferences.interface.appearance)));
    focus_mode_->setCurrentIndex(
        combo_index_for_data(*focus_mode_, static_cast<int>(preferences.focus_mode)));
    show_function_library_->setChecked(preferences.function_library.visible);
    show_project_panel_->setChecked(preferences.interface.show_project_panel);
    show_properties_panel_->setChecked(preferences.interface.show_properties_panel);
    show_toolbar_->setChecked(preferences.interface.show_toolbar);
    default_file_format_->setCurrentIndex(combo_index_for_data(
        *default_file_format_, static_cast<int>(preferences.defaults.file_format)));
}

Preferences PreferencesDialog::collect() const {
    Preferences collected = preferences_;
    collected.interface.appearance =
        static_cast<AppearanceMode>(appearance_->currentData().toInt());
    collected.focus_mode = static_cast<FocusMode>(focus_mode_->currentData().toInt());
    collected.function_library.visible = show_function_library_->isChecked();
    collected.interface.show_project_panel = show_project_panel_->isChecked();
    collected.interface.show_properties_panel = show_properties_panel_->isChecked();
    collected.interface.show_toolbar = show_toolbar_->isChecked();
    collected.defaults.file_format =
        static_cast<DefaultFileFormat>(default_file_format_->currentData().toInt());
    return collected;
}

Preferences PreferencesDialog::preferences() const {
    return collect();
}

void PreferencesDialog::set_preferences(const Preferences& preferences) {
    preferences_ = preferences;
    sync_controls(preferences_);
}

void PreferencesDialog::restore_defaults() {
    // The layout is owned by the workbench and has no dialog control, so the
    // defaults reset every editable field and keep the current saved layout.
    Preferences restored;
    restored.interface.window_state = preferences_.interface.window_state;
    set_preferences(restored);
}

void PreferencesDialog::accept() {
    preferences_ = collect();
    emit preferences_accepted(preferences_);
    QDialog::accept();
}

}  // namespace everygraph::presentation
