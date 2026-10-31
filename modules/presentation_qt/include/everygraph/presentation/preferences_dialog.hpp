#pragma once

#include <QDialog>

#include "everygraph/presentation/preferences.hpp"

class QCheckBox;
class QComboBox;
class QDialogButtonBox;

namespace everygraph::presentation {

// Preferences surface for user-interface settings. It edits only the typed
// Preferences value; persistence and application belong to the workbench.
class PreferencesDialog : public QDialog {
    Q_OBJECT

public:
    explicit PreferencesDialog(const Preferences& preferences, QWidget* parent = nullptr);

    [[nodiscard]] Preferences preferences() const;
    void set_preferences(const Preferences& preferences);

public slots:
    void restore_defaults();
    void accept() override;

signals:
    void preferences_accepted(const Preferences& preferences);

private:
    void build_ui();
    void sync_controls(const Preferences& preferences);
    [[nodiscard]] Preferences collect() const;

    Preferences preferences_;
    QComboBox* appearance_{nullptr};
    QComboBox* focus_mode_{nullptr};
    QCheckBox* show_function_library_{nullptr};
    QCheckBox* show_project_panel_{nullptr};
    QCheckBox* show_properties_panel_{nullptr};
    QCheckBox* show_toolbar_{nullptr};
    QComboBox* default_file_format_{nullptr};
    QDialogButtonBox* buttons_{nullptr};
};

}  // namespace everygraph::presentation
