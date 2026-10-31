#include "everygraph/presentation/window_placement.hpp"

#include <QPoint>
#include <QWidget>

#include <algorithm>
#include <cstdint>
#include <limits>

namespace everygraph::presentation {
namespace {

constexpr int kPlacementMargin = 32;

std::int64_t intersection_area(const QRect& left, const QRect& right) {
    const QRect intersection = left.intersected(right);
    if (intersection.isEmpty()) {
        return 0;
    }
    return static_cast<std::int64_t>(intersection.width()) *
           static_cast<std::int64_t>(intersection.height());
}

bool adequately_visible(const QRect& desired, const std::vector<QRect>& available_areas,
                        const QSize& minimum_visible) {
    const int required_width = std::min(desired.width(), std::max(1, minimum_visible.width()));
    const int required_height = std::min(desired.height(), std::max(1, minimum_visible.height()));
    for (const QRect& area : available_areas) {
        const QRect visible = desired.intersected(area);
        if (visible.width() >= required_width && visible.height() >= required_height) {
            return true;
        }
    }
    return false;
}

}  // namespace

QRect WindowPlacement::restore_to_visible(const QRect& desired,
                                          const std::vector<QRect>& available_areas,
                                          const QSize& minimum_visible) {
    if (desired.isNull() || available_areas.empty()) {
        return desired;
    }
    if (adequately_visible(desired, available_areas, minimum_visible)) {
        return desired;
    }

    const QRect* target = &available_areas.front();
    std::int64_t best_area = intersection_area(desired, *target);
    for (const QRect& area : available_areas) {
        const std::int64_t area_overlap = intersection_area(desired, area);
        if (area_overlap > best_area) {
            best_area = area_overlap;
            target = &area;
        }
    }

    QSize size(std::min(desired.width(), target->width()),
               std::min(desired.height(), target->height()));
    size.setWidth(std::max(size.width(), 1));
    size.setHeight(std::max(size.height(), 1));

    int x = target->left() + kPlacementMargin;
    int y = target->top() + kPlacementMargin;
    if (x + size.width() > target->left() + target->width()) {
        x = target->left() + target->width() - size.width();
    }
    if (y + size.height() > target->top() + target->height()) {
        y = target->top() + target->height() - size.height();
    }
    x = std::max(x, target->left());
    y = std::max(y, target->top());
    return QRect(QPoint(x, y), size);
}

QRect WindowPlacement::client_geometry_for_frame(const QRect& frame, const QRect& client,
                                                 const QRect& restored_frame) {
    const int left_inset = client.left() - frame.left();
    const int top_inset = client.top() - frame.top();
    const int right_inset = frame.right() - client.right();
    const int bottom_inset = frame.bottom() - client.bottom();
    const QSize size(std::max(1, restored_frame.width() - left_inset - right_inset),
                     std::max(1, restored_frame.height() - top_inset - bottom_inset));
    return QRect(QPoint(restored_frame.left() + left_inset,
                        restored_frame.top() + top_inset),
                 size);
}

void WindowPlacement::ensure_visible(QWidget* window,
                                     const std::vector<QRect>& available_areas) {
    if (window == nullptr || available_areas.empty()) {
        return;
    }
    const QRect frame = window->frameGeometry();
    const QRect restored = restore_to_visible(frame, available_areas);
    if (restored == frame) {
        return;
    }
    window->setGeometry(client_geometry_for_frame(frame, window->geometry(), restored));
}

}  // namespace everygraph::presentation
