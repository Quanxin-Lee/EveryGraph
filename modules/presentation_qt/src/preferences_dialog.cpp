#include "everygraph/presentation/preferences_dialog.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
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

}  // namespace

PreferencesDialog::PreferencesDialog(const Preferences& preferences, QWidget* parent)
    : QDialog(parent), preferences_(preferences) {
    setObjectName(QStringLiteral("preferencesDialog"));
    setWindowTitle(QStringLiteral("Preferences"));
    setAccessibleName(QStringLiteral("EveryGraph preferences"));
    setModal(true);
    build_ui();
    sync_controls(preferences_);
    resize(440, 420);
}

void PreferencesDialog::build_ui() {
    auto* layout = new QVBoxLayout(this);

    auto* appearance_group = new QGroupBox(QStringLiteral("Appearance"), this);
    auto* appearance_form = new QFormLayout(appearance_group);
    appearance_ = new QComboBox(appearance_group);
    appearance_->setObjectName(QStringLiteral("appearanceCombo"));
    appearance_->setAccessibleName(QStringLiteral("Appearance mode"));
    appearance_->addItem(QStringLiteral("System"),
                         static_cast<int>(AppearanceMode::system));
    appearance_->addItem(QStringLiteral("Light"), static_cast<int>(AppearanceMode::light));
    appearance_->addItem(QStringLiteral("Dark"), static_cast<int>(AppearanceMode::dark));
    appearance_form->addRow(QStringLiteral("Interface appearance:"), appearance_);
    auto* appearance_note = new QLabel(
        QStringLiteral("Appearance changes shell chrome only. Publication scene colors "
                       "and export backgrounds are not affected."),
        appearance_group);
    appearance_note->setWordWrap(true);
    appearance_form->addRow(appearance_note);
    layout->addWidget(appearance_group);

    auto* focus_group = new QGroupBox(QStringLiteral("Focus mode"), this);
    auto* focus_form = new QFormLayout(focus_group);
    focus_mode_ = new QComboBox(focus_group);
    focus_mode_->setObjectName(QStringLiteral("focusModeCombo"));
    focus_mode_->setAccessibleName(QStringLiteral("Focus mode"));
    focus_mode_->addItem(QStringLiteral("Normal layout"), static_cast<int>(FocusMode::normal));
    focus_mode_->addItem(QStringLiteral("Data only"), static_cast<int>(FocusMode::data_only));
    focus_mode_->addItem(QStringLiteral("Commands only"),
                         static_cast<int>(FocusMode::commands_only));
    focus_form->addRow(QStringLiteral("Focus mode:"), focus_mode_);
    layout->addWidget(focus_group);

    auto* interface_group = new QGroupBox(QStringLiteral("Interface"), this);
    auto* interface_form = new QFormLayout(interface_group);
    show_function_library_ = new QCheckBox(QStringLiteral("Show the function library panel"),
                                           interface_group);
    show_function_library_->setObjectName(QStringLiteral("showFunctionLibraryCheck"));
    show_function_library_->setAccessibleName(QStringLiteral("Show function library"));
    interface_form->addRow(show_function_library_);
    show_project_panel_ =
        new QCheckBox(QStringLiteral("Show the project panel"), interface_group);
    show_project_panel_->setObjectName(QStringLiteral("showProjectPanelCheck"));
    show_project_panel_->setAccessibleName(QStringLiteral("Show project panel"));
    interface_form->addRow(show_project_panel_);
    show_properties_panel_ =
        new QCheckBox(QStringLiteral("Show the properties panel"), interface_group);
    show_properties_panel_->setObjectName(QStringLiteral("showPropertiesPanelCheck"));
    show_properties_panel_->setAccessibleName(QStringLiteral("Show properties panel"));
    interface_form->addRow(show_properties_panel_);
    show_toolbar_ = new QCheckBox(QStringLiteral("Show the command toolbar"), interface_group);
    show_toolbar_->setObjectName(QStringLiteral("showToolbarCheck"));
    show_toolbar_->setAccessibleName(QStringLiteral("Show command toolbar"));
    interface_form->addRow(show_toolbar_);
    layout->addWidget(interface_group);

    auto* defaults_group = new QGroupBox(QStringLiteral("Default format"), this);
    auto* defaults_form = new QFormLayout(defaults_group);
    default_file_format_ = new QComboBox(defaults_group);
    default_file_format_->setObjectName(QStringLiteral("defaultFileFormatCombo"));
    default_file_format_->setAccessibleName(QStringLiteral("Default file format"));
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
    defaults_note->setWordWrap(true);
    defaults_form->addRow(defaults_note);
    layout->addWidget(defaults_group);

    layout->addStretch(1);
    buttons_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel |
                                        QDialogButtonBox::RestoreDefaults,
                                    this);
    buttons_->setObjectName(QStringLiteral("preferencesButtonBox"));
    connect(buttons_, &QDialogButtonBox::accepted, this, &PreferencesDialog::accept);
    connect(buttons_, &QDialogButtonBox::rejected, this, &PreferencesDialog::reject);
    connect(buttons_->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this,
            &PreferencesDialog::restore_defaults);
    layout->addWidget(buttons_);
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
