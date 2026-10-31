#pragma once

#include <QByteArray>
#include <QString>

#include <optional>

namespace everygraph::presentation {

// User-interface preferences are stored outside project documents. They never
// carry scientific defaults, scene colors or dataset values.
enum class AppearanceMode {
    system,
    light,
    dark,
};

enum class FocusMode {
    normal,
    data_only,
    commands_only,
};

enum class DefaultFileFormat {
    csv,
    parquet,
    everygraph_project,
};

struct InterfacePreferences {
    AppearanceMode appearance{AppearanceMode::system};
    bool show_project_panel{true};
    bool show_properties_panel{true};
    bool show_toolbar{true};
    QByteArray window_state;

    friend bool operator==(const InterfacePreferences& left,
                           const InterfacePreferences& right) = default;
};

struct FunctionLibraryPreferences {
    bool visible{true};

    friend bool operator==(const FunctionLibraryPreferences& left,
                           const FunctionLibraryPreferences& right) = default;
};

struct DefaultFormatPreferences {
    DefaultFileFormat file_format{DefaultFileFormat::csv};

    friend bool operator==(const DefaultFormatPreferences& left,
                           const DefaultFormatPreferences& right) = default;
};

struct Preferences {
    InterfacePreferences interface;
    FocusMode focus_mode{FocusMode::normal};
    FunctionLibraryPreferences function_library;
    DefaultFormatPreferences defaults;

    friend bool operator==(const Preferences& left, const Preferences& right) = default;
};

[[nodiscard]] QString appearance_mode_name(AppearanceMode mode);
[[nodiscard]] QString focus_mode_name(FocusMode mode);
[[nodiscard]] QString default_file_format_name(DefaultFileFormat format);
[[nodiscard]] QString default_file_format_display_name(DefaultFileFormat format);

[[nodiscard]] std::optional<AppearanceMode> appearance_mode_from_name(
    const QString& name);
[[nodiscard]] std::optional<FocusMode> focus_mode_from_name(const QString& name);
[[nodiscard]] std::optional<DefaultFileFormat> default_file_format_from_name(
    const QString& name);

}  // namespace everygraph::presentation
