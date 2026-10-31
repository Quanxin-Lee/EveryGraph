#include "everygraph/presentation/shell_appearance.hpp"

#include <QColor>
#include <QDockWidget>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QVariant>
#include <QWidget>

#include <array>

namespace everygraph::presentation {
namespace {

constexpr auto kBasePaletteProperty = "everygraphBasePalette";
constexpr auto kBaseAutoFillProperty = "everygraphBaseAutoFill";
constexpr auto kAppearanceProperty = "everygraphAppearance";

void store_base(QWidget* widget) {
    if (!widget->property(kBasePaletteProperty).isValid()) {
        widget->setProperty(kBasePaletteProperty, QVariant::fromValue(widget->palette()));
        widget->setProperty(kBaseAutoFillProperty, widget->autoFillBackground());
    }
}

void restore_base(QWidget* widget) {
    const QVariant palette = widget->property(kBasePaletteProperty);
    if (palette.isValid()) {
        widget->setPalette(palette.value<QPalette>());
        widget->setAutoFillBackground(
            widget->property(kBaseAutoFillProperty).toBool());
    }
}

std::array<QWidget*, 2> windows_chrome(QMainWindow& window) {
    return {window.menuBar(), window.statusBar()};
}

QPalette themed_palette(const QColor& window_color, const QColor& base_color,
                        const QColor& alternate_color, const QColor& text_color,
                        const QColor& button_color, const QColor& highlight_color,
                        const QColor& link_color) {
    QPalette palette;
    palette.setColor(QPalette::Window, window_color);
    palette.setColor(QPalette::WindowText, text_color);
    palette.setColor(QPalette::Base, base_color);
    palette.setColor(QPalette::AlternateBase, alternate_color);
    palette.setColor(QPalette::ToolTipBase, base_color);
    palette.setColor(QPalette::ToolTipText, text_color);
    palette.setColor(QPalette::Text, text_color);
    palette.setColor(QPalette::PlaceholderText, text_color);
    palette.setColor(QPalette::Button, button_color);
    palette.setColor(QPalette::ButtonText, text_color);
    palette.setColor(QPalette::BrightText, Qt::white);
    palette.setColor(QPalette::Highlight, highlight_color);
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::Link, link_color);
    palette.setColor(QPalette::Disabled, QPalette::Text, text_color.darker(150));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, text_color.darker(150));
    palette.setColor(QPalette::Disabled, QPalette::WindowText, text_color.darker(150));
    return palette;
}

}  // namespace

QPalette ShellAppearance::dark_palette() {
    // #3d7ebd with white selected text measured about 4.27:1, below the 4.5:1
    // normal-text floor; #2f6fab measures about 5.27:1.
    return themed_palette(QColor(QStringLiteral("#2b2b2b")),
                          QColor(QStringLiteral("#1e1e1e")),
                          QColor(QStringLiteral("#262626")),
                          QColor(QStringLiteral("#e6e6e6")),
                          QColor(QStringLiteral("#3a3a3a")),
                          QColor(QStringLiteral("#2f6fab")),
                          QColor(QStringLiteral("#8ab8e8")));
}

QPalette ShellAppearance::light_palette() {
    return themed_palette(QColor(QStringLiteral("#f2f2f2")),
                          QColor(QStringLiteral("#ffffff")),
                          QColor(QStringLiteral("#f7f7f7")),
                          QColor(QStringLiteral("#202020")),
                          QColor(QStringLiteral("#e4e4e4")),
                          QColor(QStringLiteral("#2f6fab")),
                          QColor(QStringLiteral("#2f6fab")));
}

void ShellAppearance::apply(QMainWindow& window, AppearanceMode mode) {
    QList<QWidget*> chrome;
    for (QWidget* widget : windows_chrome(window)) {
        if (widget != nullptr) {
            chrome.append(widget);
        }
    }
    for (QToolBar* toolbar : window.findChildren<QToolBar*>()) {
        chrome.append(toolbar);
    }
    for (QDockWidget* dock : window.findChildren<QDockWidget*>()) {
        chrome.append(dock);
    }
    for (QMenu* menu : window.findChildren<QMenu*>()) {
        chrome.append(menu);
    }

    for (QWidget* widget : chrome) {
        if (widget == nullptr) {
            continue;
        }
        if (mode == AppearanceMode::system) {
            restore_base(widget);
            continue;
        }
        store_base(widget);
        widget->setPalette(mode == AppearanceMode::dark ? dark_palette()
                                                        : light_palette());
        widget->setAutoFillBackground(true);
    }

    window.setProperty(kAppearanceProperty, appearance_mode_name(mode));
}

}  // namespace everygraph::presentation
