#pragma once

#include <QString>
#include <QStringList>

class QLineEdit;
class QWidget;

namespace everygraph::test {

// Result of driving the enabled macOS Pinyin input method through the native
// event path. The helper only reads the current keyboard input source: when
// Pinyin - Simplified is not already selected it reports the source as
// unavailable and skips, because selecting an input source changes the
// system-wide selection, which this task is not authorized to do.
struct NativePinyinResult {
    bool available{false};
    bool composed{false};
    QString input_source_id;
    QString committed_text;
    QStringList preedit_steps;
    QString error;
};

NativePinyinResult run_native_pinyin_ime(QWidget& host, QLineEdit& target);

}  // namespace everygraph::test
