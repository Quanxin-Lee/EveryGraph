#include "native_pinyin_ime.hpp"

#import <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>

#include <QCoreApplication>
#include <QEvent>
#include <QGuiApplication>
#include <QInputMethodEvent>
#include <QLineEdit>
#include <QThread>
#include <QWidget>
#include <QWindow>

namespace everygraph::test {
namespace {

constexpr auto kPinyinSourceId = "com.apple.inputmethod.SCIM.ITABC";

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
    CFStringRef target = CFSTR("com.apple.inputmethod.SCIM.ITABC");
    const void* keys[] = {kTISPropertyInputSourceID};
    const void* values[] = {target};
    CFDictionaryRef filter = CFDictionaryCreate(
        kCFAllocatorDefault, keys, values, 1, &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);
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

void send_key(NSTextInputContext* context, unsigned short key_code) {
    CGEventRef event = CGEventCreateKeyboardEvent(nullptr, key_code, true);
    if (event == nullptr) {
        return;
    }
    NSEvent* native_event = [NSEvent eventWithCGEvent:event];
    CFRelease(event);
    if (native_event != nil) {
        [context handleEvent:native_event];
    }
    QThread::msleep(120);
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
    [NSApp activateIgnoringOtherApps:YES];
    [window makeKeyAndOrderFront:nil];
    [view becomeFirstResponder];
    QThread::msleep(200);
    [context deactivate];
    [context activate];
    QThread::msleep(150);

    InputMethodRecorder recorder;
    target.installEventFilter(&recorder);
    const unsigned short keys[] = {45, 34, 4, 0, 31};  // n i h a o
    for (unsigned short key_code : keys) {
        send_key(context, key_code);
    }
    send_key(context, 49);  // space commits the first candidate
    QThread::msleep(250);
    QCoreApplication::processEvents();
    target.removeEventFilter(&recorder);

    result.preedit_steps = recorder.preedits;
    result.committed_text = target.text();
    result.composed = !recorder.commits.isEmpty() || !target.text().isEmpty();
    if (!result.composed) {
        result.error = QStringLiteral("no preedit or commit was observed");
    }
    return result;
}

}  // namespace everygraph::test
