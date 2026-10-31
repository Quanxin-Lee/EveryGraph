#include "everygraph/presentation/preferences.hpp"

#include <array>
#include <utility>

namespace everygraph::presentation {
namespace {

struct AppearanceName {
    AppearanceMode mode;
    const char* name;
};

struct FocusName {
    FocusMode mode;
    const char* name;
};

struct FormatName {
    DefaultFileFormat format;
    const char* name;
};

constexpr std::array<AppearanceName, 3> kAppearanceNames{{
    {AppearanceMode::system, "system"},
    {AppearanceMode::light, "light"},
    {AppearanceMode::dark, "dark"},
}};

constexpr std::array<FocusName, 3> kFocusNames{{
    {FocusMode::normal, "normal"},
    {FocusMode::data_only, "data_only"},
    {FocusMode::commands_only, "commands_only"},
}};

constexpr std::array<FormatName, 3> kFormatNames{{
    {DefaultFileFormat::csv, "csv"},
    {DefaultFileFormat::parquet, "parquet"},
    {DefaultFileFormat::everygraph_project, "everygraph_project"},
}};

}  // namespace

QString appearance_mode_name(AppearanceMode mode) {
    for (const auto& entry : kAppearanceNames) {
        if (entry.mode == mode) {
            return QString::fromLatin1(entry.name);
        }
    }
    return QStringLiteral("system");
}

QString focus_mode_name(FocusMode mode) {
    for (const auto& entry : kFocusNames) {
        if (entry.mode == mode) {
            return QString::fromLatin1(entry.name);
        }
    }
    return QStringLiteral("normal");
}

QString default_file_format_name(DefaultFileFormat format) {
    for (const auto& entry : kFormatNames) {
        if (entry.format == format) {
            return QString::fromLatin1(entry.name);
        }
    }
    return QStringLiteral("csv");
}

QString default_file_format_display_name(DefaultFileFormat format) {
    switch (format) {
        case DefaultFileFormat::csv:
            return QStringLiteral("CSV");
        case DefaultFileFormat::parquet:
            return QStringLiteral("Parquet");
        case DefaultFileFormat::everygraph_project:
            return QStringLiteral("EveryGraph project");
    }
    return QStringLiteral("CSV");
}

std::optional<AppearanceMode> appearance_mode_from_name(const QString& name) {
    for (const auto& entry : kAppearanceNames) {
        if (name == QLatin1String(entry.name)) {
            return entry.mode;
        }
    }
    return std::nullopt;
}

std::optional<FocusMode> focus_mode_from_name(const QString& name) {
    for (const auto& entry : kFocusNames) {
        if (name == QLatin1String(entry.name)) {
            return entry.mode;
        }
    }
    return std::nullopt;
}

std::optional<DefaultFileFormat> default_file_format_from_name(const QString& name) {
    for (const auto& entry : kFormatNames) {
        if (name == QLatin1String(entry.name)) {
            return entry.format;
        }
    }
    return std::nullopt;
}

}  // namespace everygraph::presentation
