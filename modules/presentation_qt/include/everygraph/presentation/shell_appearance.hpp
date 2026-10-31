#pragma once

#include <QPalette>

#include "everygraph/presentation/preferences.hpp"

class QMainWindow;

namespace everygraph::presentation {

// Applies appearance to shell chrome (menu bar, toolbars, docks, status bar)
// and never to document content, scene colors or export settings.
class ShellAppearance {
public:
    static void apply(QMainWindow& window, AppearanceMode mode);

    [[nodiscard]] static QPalette dark_palette();
    [[nodiscard]] static QPalette light_palette();
};

}  // namespace everygraph::presentation
