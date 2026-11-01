#include "native_pinyin_ime.hpp"

#import <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEvent>
#include <QGuiApplication>
#include <QInputMethodEvent>
#include <QLineEdit>
#include <QMessageLogger>
#include <QThread>
#include <QWidget>
#include <QWindow>

namespace everygraph::test {
namespace {

constexpr auto kPinyinSourceId = "com.apple.inputmethod.SCIM.ITABC";
constexpr unsigned short kSpaceKeyCode = 49;
constexpr QChar kCommitKeyCharacter = u' ';
constexpr unsigned short kPinyinKeyCodes[] = {45, 34, 4, 0, 31};  // n i h a o
constexpr int kPinyinKeyCount =
    static_cast<int>(sizeof(kPinyinKeyCodes) / sizeof(kPinyinKeyCodes[0]));
constexpr int kPreconditionBudgetMs = 1000;
constexpr int kPreeditSettleBudgetMs = 1000;
constexpr int kCompositionAttempts = 3;
constexpr int kPumpIntervalMs = 20;
constexpr int kCommitSettleMs = 250;

// Reports text as code points so that a stray character is visible in the log instead of
// looking like the expected commit.
QString code_points(const QString& text) {
    if (text.isEmpty()) {
        return QStringLiteral("<empty>");
    }
    QStringList parts;
    for (const QChar& character : text) {
        parts.append(QStringLiteral("U+%1").arg(
            QString::number(character.unicode(), 16).toUpper().rightJustified(4, u'0')));
    }
    return parts.join(QStringLiteral(" "));
}

// Keeps each commit's own code points together, so a second commit is visible as a separate
// entry instead of running into the first.
QString code_points(const QStringList& texts) {
    QStringList parts;
    for (const QString& text : texts) {
        parts.append(code_points(text));
    }
    return parts.join(QStringLiteral(" | "));
}

QString cf_string(CFTypeRef value) {
    if (value == nullptr || CFGetTypeID(value) != CFStringGetTypeID()) {
        return {};
    }
    const CFIndex length = CFStringGetLength(static_cast<CFStringRef>(value));
    const CFIndex capacity =
        CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
    QByteArray buffer(static_cast<int>(capacity), '\0');
    if (!CFStringGetCString(static_cast<CFStringRef>(value), buffer.data(), capacity,
                            kCFStringEncodingUTF8)) {
        return {};
    }
    return QString::fromUtf8(buffer.constData());
}

QString source_id(TISInputSourceRef source) {
    if (source == nullptr) {
        return {};
    }
    return cf_string(TISGetInputSourceProperty(source, kTISPropertyInputSourceID));
}

TISInputSourceRef copy_enabled_pinyin_source() {
    CFStringRef target =
        CFStringCreateWithCString(kCFAllocatorDefault, kPinyinSourceId, kCFStringEncodingASCII);
    if (target == nullptr) {
        return nullptr;
    }
    const void* keys[] = {kTISPropertyInputSourceID};
    const void* values[] = {target};
    CFDictionaryRef filter = CFDictionaryCreate(
        kCFAllocatorDefault, keys, values, 1, &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);
    CFRelease(target);
    if (filter == nullptr) {
        return nullptr;
    }
    CFArrayRef sources = TISCreateInputSourceList(filter, false);
    CFRelease(filter);
    if (sources == nullptr) {
        return nullptr;
    }
    TISInputSourceRef result = nullptr;
    if (CFArrayGetCount(sources) > 0) {
        auto* candidate = static_cast<TISInputSourceRef>(
            const_cast<void*>(CFArrayGetValueAtIndex(sources, 0)));
        CFTypeRef selectable =
            TISGetInputSourceProperty(candidate, kTISPropertyInputSourceIsSelectCapable);
        if (selectable != nullptr && CFBooleanGetValue(static_cast<CFBooleanRef>(selectable))) {
            result = (TISInputSourceRef)CFRetain(candidate);
        }
    }
    CFRelease(sources);
    return result;
}
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

// The Qt event loop has to run for the window-server and text-input notifications to
// arrive, and the keystroke pacing below relies on that.
void pump_events(unsigned long milliseconds) {
    QThread::msleep(milliseconds);
    QCoreApplication::processEvents();
}

// Qt's own marked range is the live-composition state of this widget. The return value of
// -[NSTextInputContext handleEvent:] cannot serve: it reports the text input system
// consumed the key both when that key selected a candidate and when it inserted the
// character itself.
bool has_live_preedit(NSTextInputContext* context) {
    if (context == nil) {
        return false;
    }
    const NSRange marked = [context.client markedRange];
    return marked.location != NSNotFound && marked.length > 0;
}

bool owns_text_input(NSTextInputContext* context) {
    return context != nil && [NSTextInputContext currentInputContext] == context;
}

QString context_keyboard_source(NSTextInputContext* context) {
    if (context == nil || context.selectedKeyboardInputSource == nil) {
        return {};
    }
    return QString::fromNSString(context.selectedKeyboardInputSource);
}

// False means the key never reached the input method, so the caller can say so instead of
// reporting a composition that silently received fewer keys than it asked for.
bool send_key(NSTextInputContext* context, unsigned short key_code) {
    CGEventRef event = CGEventCreateKeyboardEvent(nullptr, key_code, true);
    if (event == nullptr) {
        return false;
    }
    NSEvent* native_event = [NSEvent eventWithCGEvent:event];
    CFRelease(event);
    if (native_event == nil) {
        return false;
    }
    [context handleEvent:native_event];
    QThread::msleep(120);
    return true;
}

// True when the committing key's own character reached the field on top of the candidate,
// which is the input method re-dispatching the key after it closed the composition. It
// arrives in one of two shapes: as text the commit strings never carried, or as a commit
// whose whole content is the key. Neither can be a candidate, so the field cannot match
// what the input method was asked to compose and the attempt is repeated.
bool has_pass_through_artifact(const QLineEdit& target, const QStringList& commits,
                               QChar commit_key_character) {
    if (commits.isEmpty()) {
        return false;
    }
    if (commits.last() == QString(commit_key_character)) {
        return true;
    }
    QString committed;
    for (const QString& commit : commits) {
        committed.append(commit);
    }
    return target.text().contains(commit_key_character) &&
           !committed.contains(commit_key_character);
}

// Waits for this widget's context to name the input method the system-wide selection
// already names, because a key delivered to a context left on another source lands in the
// field as raw text. Context currency is deliberately not required: [NSTextInputContext
// currentInputContext] does not name this view's context in every healthy run, so waiting
// on it would fail runs that compose. The budget only delays the verdict: the caller types
// nothing until this returns satisfied.
bool wait_for_pinyin_context(NSTextInputContext* context, const QString& pinyin_id,
                             int* elapsed_ms) {
    QElapsedTimer timer;
    timer.start();
    while (true) {
        if (context_keyboard_source(context) == pinyin_id) {
            *elapsed_ms += static_cast<int>(timer.elapsed());
            return true;
        }
        if (timer.elapsed() >= kPreconditionBudgetMs) {
            *elapsed_ms += static_cast<int>(timer.elapsed());
            return false;
        }
        pump_events(kPumpIntervalMs);
    }
}

}  // namespace

NativePinyinResult run_native_pinyin_ime(QWidget& host, QLineEdit& target) {
    NativePinyinResult result;
    if (QGuiApplication::platformName() != QStringLiteral("cocoa")) {
        result.error = QStringLiteral("the native IME check requires the cocoa platform");
        return result;
    }
    if (host.windowHandle() == nullptr || !host.isVisible()) {
        result.error = QStringLiteral("the host widget has no visible native window");
        return result;
    }
    target.clear();
    target.setFocus(Qt::OtherFocusReason);
    QCoreApplication::processEvents();

    TISInputSourceRef current = TISCopyCurrentKeyboardInputSource();
    result.input_source_id = source_id(current);
    if (current != nullptr) {
        CFRelease(current);
    }
    if (result.input_source_id != QString::fromLatin1(kPinyinSourceId)) {
        TISInputSourceRef pinyin = copy_enabled_pinyin_source();
        if (pinyin == nullptr) {
            result.error = QStringLiteral(
                "Pinyin - Simplified is not enabled or not selectable; "
                "the native composition check was skipped");
        } else {
            CFRelease(pinyin);
            result.error = QStringLiteral(
                "Pinyin - Simplified is enabled but is not the current input source; "
                "selecting it would change the system-wide selection, which is not "
                "authorized, so the native composition check was skipped");
        }
        return result;
    }
    result.available = true;

    NSView* view = (NSView*)(void*)host.windowHandle()->winId();
    NSWindow* window = [view window];
    NSTextInputContext* context = [view inputContext];
    if (view == nil || window == nil || context == nil) {
        result.error = QStringLiteral("the cocoa input context is unavailable");
        return result;
    }
    const QString pinyin_id = QString::fromLatin1(kPinyinSourceId);
    [NSApp activateIgnoringOtherApps:YES];
    [window makeKeyAndOrderFront:nil];
    [view becomeFirstResponder];
    pump_events(200);

    InputMethodRecorder recorder;
    target.installEventFilter(&recorder);

    // Each attempt re-activates the context, types only after the source precondition holds,
    // and repeats the whole composition when the input method leaves the field unmatched: a
    // context that has not taken the Pinyin method never receives the syllable keys, and a
    // commit the input method disturbed is recomposed from an empty field instead of reported.
    int attempts = 0;
    int precondition_wait_ms = 0;
    int undelivered_keys = 0;
    int artifact_attempts = 0;
    bool ever_composed = false;
    bool commit_key_sent = false;
    const auto deliver = [context, &undelivered_keys](unsigned short key_code) {
        if (!send_key(context, key_code)) {
            ++undelivered_keys;
        }
    };
    for (int attempt = 1; attempt <= kCompositionAttempts; ++attempt) {
        attempts = attempt;
        [context deactivate];
        [context activate];
        if (!wait_for_pinyin_context(context, pinyin_id, &precondition_wait_ms)) {
            continue;
        }
        // Cleared here rather than before the activation above, because deactivating and
        // re-activating can flush a late commit of the previous attempt into the field, and
        // that text would then survive the clear and read as an extra character. The recorder
        // is reset at the same moment for the same reason, so the commit strings reported for
        // an attempt are only the ones that attempt's own keystrokes produced.
        target.clear();
        recorder.preedits.clear();
        recorder.commits.clear();
        deliver(kPinyinKeyCodes[0]);
        // The input method takes the first syllable letter as a preedit; if it did not,
        // this context is not composing and the remaining letters would land raw.
        QElapsedTimer settle;
        settle.start();
        while (!has_live_preedit(context) && settle.elapsed() < kPreeditSettleBudgetMs) {
            pump_events(kPumpIntervalMs);
        }
        const bool preedit_open = has_live_preedit(context);
        if (!preedit_open) {
            continue;
        }
        ever_composed = true;
        for (int index = 1; index < kPinyinKeyCount; ++index) {
            deliver(kPinyinKeyCodes[index]);
        }

        // A committing key is only a command while a preedit is live. With the composition
        // already closed the same key is text input, and the input method commits that
        // character into the field on top of the candidate.
        commit_key_sent = has_live_preedit(context);
        if (commit_key_sent) {
            deliver(kSpaceKeyCode);
        }
        pump_events(kCommitSettleMs);
        if (has_pass_through_artifact(target, recorder.commits, kCommitKeyCharacter)) {
            ++artifact_attempts;
            // Printed here rather than only in the summary, because the attempt being repeated
            // is the one whose text is about to be cleared: the disturbed commit stays in the
            // log even when a later attempt succeeds.
            qInfo().noquote() << "[native-ime] attempt" << attempt
                              << "pass-through: commits" << code_points(recorder.commits)
                              << "widget-text" << code_points(target.text());
            continue;
        }
        if (commit_key_sent && !recorder.commits.isEmpty()) {
            break;
        }
    }

    if (!ever_composed) {
        target.removeEventFilter(&recorder);
        result.preedit_steps = recorder.preedits;
        result.committed_text = target.text();
        // Characters delivered to a context on another source can still arrive as a commit
        // string, so this helper only calls text composed when the input method actually
        // opened a preedit for it.
        result.composed = false;
        result.error = QStringLiteral(
            "the input method never entered composition mode: attempts %1, context owner %2, "
            "context source %3, system source %4, precondition wait %5 ms, undelivered keys "
            "%6; the committing key was withheld so it could not add text to the field")
                           .arg(QString::number(attempts),
                                owns_text_input(context) ? QStringLiteral("yes")
                                                         : QStringLiteral("no"),
                                context_keyboard_source(context).isEmpty()
                                    ? QStringLiteral("<none>")
                                    : context_keyboard_source(context),
                                result.input_source_id,
                                QString::number(precondition_wait_ms),
                                QString::number(undelivered_keys));
        qInfo().noquote() << "[native-ime]" << result.error;
        return result;
    }

    target.removeEventFilter(&recorder);

    result.preedit_steps = recorder.preedits;
    result.committed_text = target.text();
    result.composed = !recorder.commits.isEmpty() || !target.text().isEmpty();
    result.disturbed_attempts = artifact_attempts;
    qInfo().noquote() << "[native-ime] attempts:" << attempts
                      << "precondition-wait-ms:" << precondition_wait_ms
                      << "preedit-steps:" << result.preedit_steps.size()
                      << "undelivered-keys:" << undelivered_keys
                      << "artifact-attempts:" << artifact_attempts
                      << "commit-key:" << (commit_key_sent ? QStringLiteral("sent")
                                                            : QStringLiteral("withheld"))
                      << "commits:" << code_points(recorder.commits)
                      << "widget-text:" << code_points(result.committed_text);
    if (!result.composed) {
        result.error = QStringLiteral("no preedit or commit was observed");
    }
    return result;
}

}  // namespace everygraph::test
