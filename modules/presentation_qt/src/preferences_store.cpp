#include "everygraph/presentation/preferences_store.hpp"

#include <QByteArray>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStringList>

#include <cmath>
#include <utility>

namespace everygraph::presentation {
namespace {

constexpr auto kSchemaVersionKey = "schema_version";
constexpr auto kInterfaceKey = "interface";
constexpr auto kFocusModeKey = "focus_mode";
constexpr auto kFunctionLibraryKey = "function_library";
constexpr auto kDefaultsKey = "defaults";

constexpr auto kAppearanceKey = "appearance";
constexpr auto kShowProjectPanelKey = "show_project_panel";
constexpr auto kShowPropertiesPanelKey = "show_properties_panel";
constexpr auto kShowToolbarKey = "show_toolbar";
constexpr auto kWindowStateKey = "window_state";

constexpr auto kFunctionLibraryVisibleKey = "visible";
constexpr auto kFileFormatKey = "file_format";

void add_diagnostic(PreferencesLoadResult& result, const char* code, const QString& message) {
    result.diagnostics.push_back(PreferenceDiagnostic{QString::fromLatin1(code), message});
}

void emit_diagnostics(const PreferencesLoadResult& result) {
    for (const PreferenceDiagnostic& diagnostic : result.diagnostics) {
        qWarning().noquote() << diagnostic.code + QLatin1Char(':') << diagnostic.message;
    }
}

void report_unknown_keys(const QJsonObject& object, const QStringList& known_keys,
                         const QString& prefix, PreferencesLoadResult& result) {
    for (const QString& key : object.keys()) {
        if (!known_keys.contains(key)) {
            add_diagnostic(
                result, PreferencesStore::kUnknownKeyCode,
                QStringLiteral("%1.%2 is not a known preference and was ignored")
                    .arg(prefix, key));
        }
    }
}

void read_boolean(const QJsonObject& object, const QString& key, const QString& path,
                  bool& target, PreferencesLoadResult& result) {
    const QJsonValue value = object.value(key);
    if (value.isUndefined()) {
        return;
    }
    if (!value.isBool()) {
        add_diagnostic(result, PreferencesStore::kInvalidTypeCode,
                       QStringLiteral("%1 must be a boolean; using the default %2")
                           .arg(path, target ? QStringLiteral("true") : QStringLiteral("false")));
        return;
    }
    target = value.toBool();
}

void read_string(const QJsonObject& object, const QString& key, const QString& path,
                 QString& target, PreferencesLoadResult& result) {
    const QJsonValue value = object.value(key);
    if (value.isUndefined()) {
        return;
    }
    if (!value.isString()) {
        add_diagnostic(result, PreferencesStore::kInvalidTypeCode,
                       QStringLiteral("%1 must be a string; using the default \"%2\"")
                           .arg(path, target));
        return;
    }
    target = value.toString();
}

void read_window_state(const QJsonObject& object, PreferencesLoadResult& result) {
    const QJsonValue value = object.value(QLatin1String(kWindowStateKey));
    if (value.isUndefined()) {
        return;
    }
    if (!value.isString()) {
        add_diagnostic(result, PreferencesStore::kInvalidTypeCode,
                       QStringLiteral("%1.window_state must be a base64 string; using the default layout")
                           .arg(QLatin1String(kInterfaceKey)));
        return;
    }
    const QString encoded = value.toString();
    if (encoded.isEmpty()) {
        return;
    }
    const auto decoded = QByteArray::fromBase64Encoding(
        encoded.toLatin1(),
        QByteArray::Base64Encoding | QByteArray::AbortOnBase64DecodingErrors);
    if (decoded.decodingStatus != QByteArray::Base64DecodingStatus::Ok) {
        add_diagnostic(result, PreferencesStore::kInvalidWindowStateCode,
                       QStringLiteral("interface.window_state is not valid base64; "
                                      "using the default layout"));
        return;
    }
    result.preferences.interface.window_state = decoded.decoded;
}

void read_interface(const QJsonDocument& document, PreferencesLoadResult& result) {
    const QJsonValue group = document.object().value(QLatin1String(kInterfaceKey));
    if (group.isUndefined()) {
        return;
    }
    if (!group.isObject()) {
        add_diagnostic(result, PreferencesStore::kInvalidTypeCode,
                       QStringLiteral("interface must be an object; using interface defaults"));
        return;
    }
    const QJsonObject object = group.toObject();
    report_unknown_keys(object,
                        {QStringLiteral("appearance"), QStringLiteral("show_project_panel"),
                         QStringLiteral("show_properties_panel"),
                         QStringLiteral("show_toolbar"), QStringLiteral("window_state")},
                        QStringLiteral("interface"), result);

    QString appearance = appearance_mode_name(result.preferences.interface.appearance);
    read_string(object, QLatin1String(kAppearanceKey), QStringLiteral("interface.appearance"),
                appearance, result);
    if (const auto parsed = appearance_mode_from_name(appearance); parsed.has_value()) {
        result.preferences.interface.appearance = *parsed;
    } else {
        add_diagnostic(
            result, PreferencesStore::kInvalidValueCode,
            QStringLiteral("interface.appearance \"%1\" is not system, light or dark; "
                           "using the default \"system\"")
                .arg(appearance));
        result.preferences.interface.appearance = AppearanceMode::system;
    }

    read_boolean(object, QLatin1String(kShowProjectPanelKey),
                 QStringLiteral("interface.show_project_panel"),
                 result.preferences.interface.show_project_panel, result);
    read_boolean(object, QLatin1String(kShowPropertiesPanelKey),
                 QStringLiteral("interface.show_properties_panel"),
                 result.preferences.interface.show_properties_panel, result);
    read_boolean(object, QLatin1String(kShowToolbarKey),
                 QStringLiteral("interface.show_toolbar"),
                 result.preferences.interface.show_toolbar, result);

    read_window_state(object, result);
}

void read_function_library(const QJsonDocument& document, PreferencesLoadResult& result) {
    const QJsonValue group = document.object().value(QLatin1String(kFunctionLibraryKey));
    if (group.isUndefined()) {
        return;
    }
    if (!group.isObject()) {
        add_diagnostic(result, PreferencesStore::kInvalidTypeCode,
                       QStringLiteral("function_library must be an object; "
                                      "using function-library defaults"));
        return;
    }
    const QJsonObject object = group.toObject();
    report_unknown_keys(object, {QStringLiteral("visible")},
                        QStringLiteral("function_library"), result);
    read_boolean(object, QLatin1String(kFunctionLibraryVisibleKey),
                 QStringLiteral("function_library.visible"),
                 result.preferences.function_library.visible, result);
}

void read_defaults(const QJsonDocument& document, PreferencesLoadResult& result) {
    const QJsonValue group = document.object().value(QLatin1String(kDefaultsKey));
    if (group.isUndefined()) {
        return;
    }
    if (!group.isObject()) {
        add_diagnostic(result, PreferencesStore::kInvalidTypeCode,
                       QStringLiteral("defaults must be an object; using default-format defaults"));
        return;
    }
    const QJsonObject object = group.toObject();
    report_unknown_keys(object, {QStringLiteral("file_format")},
                        QStringLiteral("defaults"), result);

    QString format = default_file_format_name(result.preferences.defaults.file_format);
    read_string(object, QLatin1String(kFileFormatKey), QStringLiteral("defaults.file_format"),
                format, result);
    if (const auto parsed = default_file_format_from_name(format); parsed.has_value()) {
        result.preferences.defaults.file_format = *parsed;
    } else {
        add_diagnostic(
            result, PreferencesStore::kInvalidValueCode,
            QStringLiteral("defaults.file_format \"%1\" is not csv, parquet or "
                           "everygraph_project; using the default \"csv\"")
                .arg(format));
        result.preferences.defaults.file_format = DefaultFileFormat::csv;
    }
}

void read_focus_mode(const QJsonDocument& document, PreferencesLoadResult& result) {
    const QJsonValue value = document.object().value(QLatin1String(kFocusModeKey));
    if (value.isUndefined()) {
        return;
    }
    if (!value.isString()) {
        add_diagnostic(result, PreferencesStore::kInvalidTypeCode,
                       QStringLiteral("focus_mode must be a string; using the default \"normal\""));
        return;
    }
    const QString name = value.toString();
    if (const auto parsed = focus_mode_from_name(name); parsed.has_value()) {
        result.preferences.focus_mode = *parsed;
        return;
    }
    add_diagnostic(result, PreferencesStore::kInvalidValueCode,
                   QStringLiteral("focus_mode \"%1\" is not normal, data_only or commands_only; "
                                  "using the default \"normal\"")
                       .arg(name));
    result.preferences.focus_mode = FocusMode::normal;
}

bool appearance_is_known(AppearanceMode mode) {
    switch (mode) {
        case AppearanceMode::system:
        case AppearanceMode::light:
        case AppearanceMode::dark:
            return true;
    }
    return false;
}

bool focus_mode_is_known(FocusMode mode) {
    switch (mode) {
        case FocusMode::normal:
        case FocusMode::data_only:
        case FocusMode::commands_only:
            return true;
    }
    return false;
}

bool file_format_is_known(DefaultFileFormat format) {
    switch (format) {
        case DefaultFileFormat::csv:
        case DefaultFileFormat::parquet:
        case DefaultFileFormat::everygraph_project:
            return true;
    }
    return false;
}

bool report_write_failure(const QString& path, const QString& reason,
                          QString* error_message) {
    const QString message = QStringLiteral("cannot write %1: %2").arg(path, reason);
    qWarning().noquote() << QString::fromLatin1(PreferencesStore::kWriteFailedCode)
                                + QLatin1Char(':')
                         << message;
    if (error_message != nullptr) {
        *error_message = message;
    }
    return false;
}

}  // namespace

PreferencesStore::PreferencesStore(QString file_path) : file_path_(std::move(file_path)) {}

QString PreferencesStore::default_file_path() {
    const QByteArray override_path = qgetenv("EVERYGRAPH_PREFERENCES_PATH");
    if (!override_path.isEmpty()) {
        return QFile::decodeName(override_path);
    }
    QString directory =
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (directory.isEmpty()) {
        directory = QDir::home().filePath(QStringLiteral(".everygraph"));
    }
    return QDir(directory).filePath(QStringLiteral("preferences.json"));
}

const QString& PreferencesStore::file_path() const noexcept {
    return file_path_;
}

PreferencesLoadResult PreferencesStore::load() const {
    PreferencesLoadResult result;
    QFile file(file_path_);
    if (!file.exists()) {
        return result;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        add_diagnostic(result, kReadFailedCode,
                       QStringLiteral("cannot read %1: %2").arg(file_path_, file.errorString()));
        emit_diagnostics(result);
        return result;
    }
    const QByteArray payload = file.readAll();
    if (file.error() != QFileDevice::NoError) {
        add_diagnostic(result, kReadFailedCode,
                       QStringLiteral("cannot read %1: %2").arg(file_path_, file.errorString()));
        emit_diagnostics(result);
        return result;
    }

    QJsonParseError parse_error;
    const QJsonDocument document = QJsonDocument::fromJson(payload, &parse_error);
    if (parse_error.error != QJsonParseError::NoError) {
        add_diagnostic(result, kInvalidJsonCode,
                       QStringLiteral("%1 is not valid JSON: %2")
                           .arg(file_path_, parse_error.errorString()));
        emit_diagnostics(result);
        return result;
    }
    if (!document.isObject()) {
        add_diagnostic(result, kInvalidRootCode,
                       QStringLiteral("%1 must contain a JSON object; using defaults")
                           .arg(file_path_));
        emit_diagnostics(result);
        return result;
    }

    const QJsonObject root = document.object();
    const QJsonValue version = root.value(QLatin1String(kSchemaVersionKey));
    if (!version.isDouble() || std::floor(version.toDouble()) != version.toDouble() ||
        version.toInt() != kSchemaVersion) {
        add_diagnostic(
            result, kUnsupportedSchemaCode,
            QStringLiteral("%1 has schema_version %2; this build supports version %3 and "
                           "uses defaults")
                .arg(file_path_)
                .arg(version.isDouble() ? QString::number(version.toDouble())
                                        : QStringLiteral("(missing or non-numeric)"))
                .arg(kSchemaVersion));
        emit_diagnostics(result);
        return result;
    }

    report_unknown_keys(
        root,
        {QStringLiteral("schema_version"), QStringLiteral("interface"),
         QStringLiteral("focus_mode"), QStringLiteral("function_library"),
         QStringLiteral("defaults")},
        QStringLiteral("preferences"), result);

    read_interface(document, result);
    read_focus_mode(document, result);
    read_function_library(document, result);
    read_defaults(document, result);
    result.loaded_from_file = true;
    emit_diagnostics(result);
    return result;
}

bool PreferencesStore::save(const Preferences& preferences, QString* error_message) const {
    if (!appearance_is_known(preferences.interface.appearance) ||
        !focus_mode_is_known(preferences.focus_mode) ||
        !file_format_is_known(preferences.defaults.file_format)) {
        const QString message = QStringLiteral("preferences contain an unknown enum value");
        if (error_message != nullptr) {
            *error_message = message;
        }
        return false;
    }

    QJsonObject root;
    root.insert(QLatin1String(kSchemaVersionKey), kSchemaVersion);

    QJsonObject interface;
    interface.insert(QLatin1String(kAppearanceKey),
                     appearance_mode_name(preferences.interface.appearance));
    interface.insert(QLatin1String(kShowProjectPanelKey),
                     preferences.interface.show_project_panel);
    interface.insert(QLatin1String(kShowPropertiesPanelKey),
                     preferences.interface.show_properties_panel);
    interface.insert(QLatin1String(kShowToolbarKey), preferences.interface.show_toolbar);
    interface.insert(QLatin1String(kWindowStateKey),
                     QString::fromLatin1(
                         preferences.interface.window_state.toBase64()));
    root.insert(QLatin1String(kInterfaceKey), interface);
    root.insert(QLatin1String(kFocusModeKey), focus_mode_name(preferences.focus_mode));

    QJsonObject function_library;
    function_library.insert(QLatin1String(kFunctionLibraryVisibleKey),
                            preferences.function_library.visible);
    root.insert(QLatin1String(kFunctionLibraryKey), function_library);

    QJsonObject defaults;
    defaults.insert(QLatin1String(kFileFormatKey),
                    default_file_format_name(preferences.defaults.file_format));
    root.insert(QLatin1String(kDefaultsKey), defaults);

    const QDir directory = QFileInfo(file_path_).absoluteDir();
    if (!directory.exists() && !directory.mkpath(QStringLiteral("."))) {
        return report_write_failure(file_path_,
                                    QStringLiteral("cannot create directory %1")
                                        .arg(directory.absolutePath()),
                                    error_message);
    }

    QSaveFile file(file_path_);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return report_write_failure(file_path_, file.errorString(), error_message);
    }
    const QByteArray payload = QJsonDocument(root).toJson(QJsonDocument::Indented);
    if (file.write(payload) != payload.size()) {
        const QString reason = file.errorString();
        file.cancelWriting();
        return report_write_failure(file_path_, reason, error_message);
    }
    if (!file.commit()) {
        return report_write_failure(file_path_, file.errorString(), error_message);
    }
    return true;
}

}  // namespace everygraph::presentation
