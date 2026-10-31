#pragma once

#include <QRect>
#include <QSize>

#include <vector>

class QWidget;

namespace everygraph::presentation {

// Restores windows that a display change left partly or fully off-screen.
// The functions are pure geometry helpers so their behavior is testable
// without creating a platform window.
class WindowPlacement {
public:
    // Returns desired unchanged when it is adequately visible on one of the
    // available areas; otherwise returns a frame on the most relevant area,
    // preserving the size when it fits and shrinking an oversized frame to
    // that area.
    [[nodiscard]] static QRect restore_to_visible(
        const QRect& desired, const std::vector<QRect>& available_areas,
        const QSize& minimum_visible = QSize(64, 32));

    // Converts a restored frame rectangle into the client rectangle that
    // preserves the current frame/client decoration insets.
    [[nodiscard]] static QRect client_geometry_for_frame(
        const QRect& frame, const QRect& client, const QRect& restored_frame);

    static void ensure_visible(QWidget* window, const std::vector<QRect>& available_areas);
};

}  // namespace everygraph::presentation
