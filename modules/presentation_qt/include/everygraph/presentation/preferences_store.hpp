#pragma once

#include <QString>

#include <vector>

#include "everygraph/presentation/preferences.hpp"

namespace everygraph::presentation {

struct PreferenceDiagnostic {
    QString code;
    QString message;

    friend bool operator==(const PreferenceDiagnostic& left,
                           const PreferenceDiagnostic& right) = default;
};

struct PreferencesLoadResult {
    Preferences preferences;
    std::vector<PreferenceDiagnostic> diagnostics;
    bool loaded_from_file{false};
};

// Versioned JSON store for user-interface preferences. It is deliberately not
// a project format: the file holds no document, dataset or publication state.
// Invalid or unknown values keep their documented defaults and produce a
// diagnostic instead of failing the application.
class PreferencesStore {
public:
    static constexpr int kSchemaVersion = 1;

    inline static constexpr auto kInvalidJsonCode = "settings.invalid_json";
    inline static constexpr auto kInvalidRootCode = "settings.invalid_root";
    inline static constexpr auto kUnsupportedSchemaCode = "settings.unsupported_schema";
    inline static constexpr auto kUnknownKeyCode = "settings.unknown_key";
    inline static constexpr auto kInvalidTypeCode = "settings.invalid_type";
    inline static constexpr auto kInvalidValueCode = "settings.invalid_value";
    inline static constexpr auto kInvalidWindowStateCode = "settings.invalid_window_state";
    inline static constexpr auto kReadFailedCode = "settings.read_failed";
    inline static constexpr auto kWriteFailedCode = "settings.write_failed";

    explicit PreferencesStore(QString file_path);

    // Resolves to QStandardPaths::AppConfigLocation/preferences.json, or to
    // EVERYGRAPH_PREFERENCES_PATH when that variable is set.
    [[nodiscard]] static QString default_file_path();

    [[nodiscard]] const QString& file_path() const noexcept;

    [[nodiscard]] PreferencesLoadResult load() const;
    [[nodiscard]] bool save(const Preferences& preferences,
                            QString* error_message = nullptr) const;

private:
    QString file_path_;
};

}  // namespace everygraph::presentation
