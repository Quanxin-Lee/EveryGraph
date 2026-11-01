#pragma once

#include <QString>
#include <QStringList>

class QLineEdit;
class QWidget;

namespace everygraph::test {

// Result of driving the enabled macOS Pinyin input method through the native
// event path. The helper only reads input sources and never selects one:
// selecting changes the system-wide selection, which is outside this task's
// authorization. When Pinyin - Simplified is not already the current source it
// reports the source as unavailable and skips.
//
// Composition rests on three mechanisms. The helper types nothing until this
// widget's own text input context names the Pinyin source, and the first
// syllable letter must open a preedit before the rest follow. The
// candidate-committing key is sent only while a preedit is live, because with
// the composition already closed that key is ordinary text input and the input
// method commits it on top of the candidate. Finally the field is compared
// against the commit strings the input method actually produced, and when the
// committing key's own character reached the field the attempt is counted in
// disturbed_attempts, its text is logged as code points, and the whole
// composition is repeated, bounded by three attempts.
//
// What that does and does not promise: a run whose every attempt is disturbed
// still fails, because the caller asserts on the field text of the attempt that
// last typed into it, and a run that never enters composition fails instead of
// skipping. A run that recovers passes on a later attempt's composition, and
// only the log and disturbed_attempts show how many attempts it took; no
// assertion reads them, because the caller's assertions belong to another task.
struct NativePinyinResult {
    bool available{false};
    bool composed{false};
    QString input_source_id;
    QString committed_text;
    QStringList preedit_steps;
    int disturbed_attempts{0};
    QString error;
};

NativePinyinResult run_native_pinyin_ime(QWidget& host, QLineEdit& target);

}  // namespace everygraph::test
