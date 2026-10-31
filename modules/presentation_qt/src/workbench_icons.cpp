#include "everygraph/presentation/workbench_icons.hpp"

#include <QColor>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

#include <array>

namespace everygraph::presentation {
namespace {

const QColor kStroke(51, 65, 85);
const QColor kFill(219, 234, 254);
const QColor kPaper(248, 250, 252);

void draw_arrow_head(QPainter& painter, const QPointF& tip, const QPointF& first,
                     const QPointF& second) {
    painter.drawLine(tip, first);
    painter.drawLine(tip, second);
}

void draw_folder(QPainter& painter) {
    QPainterPath path;
    path.moveTo(4.0, 10.0);
    path.lineTo(4.0, 7.0);
    path.quadTo(4.0, 5.0, 6.0, 5.0);
    path.lineTo(12.0, 5.0);
    path.lineTo(15.0, 9.0);
    path.lineTo(26.0, 9.0);
    path.quadTo(28.0, 9.0, 28.0, 11.0);
    path.lineTo(28.0, 24.0);
    path.quadTo(28.0, 26.0, 26.0, 26.0);
    path.lineTo(6.0, 26.0);
    path.quadTo(4.0, 26.0, 4.0, 24.0);
    path.closeSubpath();
    painter.fillPath(path, kFill);
    painter.drawPath(path);
}

void draw_dataset(QPainter& painter) {
    const QRectF grid(4.0, 6.0, 24.0, 20.0);
    painter.drawRoundedRect(grid, 2.0, 2.0);
    painter.drawLine(QPointF(4.0, 12.0), QPointF(28.0, 12.0));
    painter.drawLine(QPointF(4.0, 18.0), QPointF(28.0, 18.0));
    painter.drawLine(QPointF(4.0, 24.0), QPointF(28.0, 24.0));
    painter.drawLine(QPointF(13.0, 6.0), QPointF(13.0, 26.0));
    painter.drawLine(QPointF(21.0, 6.0), QPointF(21.0, 26.0));
}

void draw_move(QPainter& painter) {
    painter.drawLine(QPointF(6.0, 26.0), QPointF(24.0, 8.0));
    draw_arrow_head(painter, QPointF(24.0, 8.0), QPointF(16.0, 8.0), QPointF(24.0, 16.0));
}

void draw_find(QPainter& painter) {
    painter.drawEllipse(QRectF(5.0, 4.0, 17.0, 17.0));
    painter.drawLine(QPointF(19.5, 19.0), QPointF(27.5, 27.0));
}

void draw_document(QPainter& painter) {
    QPainterPath page;
    page.moveTo(8.0, 4.0);
    page.lineTo(19.0, 4.0);
    page.lineTo(25.0, 10.0);
    page.lineTo(25.0, 28.0);
    page.lineTo(8.0, 28.0);
    page.closeSubpath();
    painter.fillPath(page, kPaper);
    painter.drawPath(page);
    painter.drawLine(QPointF(19.0, 4.0), QPointF(19.0, 10.0));
    painter.drawLine(QPointF(19.0, 10.0), QPointF(25.0, 10.0));
    painter.drawLine(QPointF(12.0, 16.0), QPointF(21.0, 16.0));
    painter.drawLine(QPointF(12.0, 20.0), QPointF(21.0, 20.0));
}

void draw_undo(QPainter& painter) {
    QPainterPath path;
    path.moveTo(25.0, 23.0);
    path.cubicTo(26.0, 13.0, 16.0, 8.0, 9.0, 13.0);
    painter.drawPath(path);
    draw_arrow_head(painter, QPointF(9.0, 13.0), QPointF(9.0, 6.5), QPointF(15.0, 12.0));
}

void draw_redo(QPainter& painter) {
    painter.save();
    painter.translate(32.0, 0.0);
    painter.scale(-1.0, 1.0);
    draw_undo(painter);
    painter.restore();
}

void draw_detach(QPainter& painter) {
    painter.drawRoundedRect(QRectF(4.0, 9.0, 16.0, 17.0), 2.0, 2.0);
    painter.drawLine(QPointF(16.0, 16.0), QPointF(27.0, 5.0));
    draw_arrow_head(painter, QPointF(27.0, 5.0), QPointF(20.0, 5.0), QPointF(27.0, 12.0));
}

void draw_dock(QPainter& painter) {
    painter.drawRoundedRect(QRectF(4.0, 6.0, 18.0, 18.0), 2.0, 2.0);
    painter.drawLine(QPointF(29.0, 29.0), QPointF(17.0, 17.0));
    draw_arrow_head(painter, QPointF(17.0, 17.0), QPointF(24.0, 17.0), QPointF(17.0, 24.0));
}

void draw_project_tree(QPainter& painter) {
    painter.drawRoundedRect(QRectF(3.0, 4.0, 9.0, 6.0), 1.0, 1.0);
    painter.drawLine(QPointF(7.5, 10.0), QPointF(7.5, 26.0));
    painter.drawLine(QPointF(7.5, 14.0), QPointF(15.0, 14.0));
    painter.drawLine(QPointF(7.5, 25.0), QPointF(15.0, 25.0));
    painter.drawRoundedRect(QRectF(15.0, 10.0, 14.0, 7.0), 1.0, 1.0);
    painter.drawRoundedRect(QRectF(15.0, 21.0, 14.0, 7.0), 1.0, 1.0);
}

void draw_properties(QPainter& painter) {
    painter.drawLine(QPointF(5.0, 9.0), QPointF(27.0, 9.0));
    painter.drawLine(QPointF(5.0, 16.0), QPointF(27.0, 16.0));
    painter.drawLine(QPointF(5.0, 23.0), QPointF(27.0, 23.0));
    painter.setBrush(kPaper);
    painter.drawEllipse(QPointF(12.0, 9.0), 3.0, 3.0);
    painter.drawEllipse(QPointF(21.0, 16.0), 3.0, 3.0);
    painter.drawEllipse(QPointF(9.0, 23.0), 3.0, 3.0);
}

void draw_icon(QPainter& painter, WorkbenchIcon icon) {
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(kStroke, 2.0);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    switch (icon) {
        case WorkbenchIcon::new_folder:
            draw_folder(painter);
            break;
        case WorkbenchIcon::new_dataset:
            draw_dataset(painter);
            break;
        case WorkbenchIcon::move_object:
            draw_move(painter);
            break;
        case WorkbenchIcon::find_object:
            draw_find(painter);
            break;
        case WorkbenchIcon::open_document:
            draw_document(painter);
            break;
        case WorkbenchIcon::undo:
            draw_undo(painter);
            break;
        case WorkbenchIcon::redo:
            draw_redo(painter);
            break;
        case WorkbenchIcon::detach_document:
            draw_detach(painter);
            break;
        case WorkbenchIcon::dock_document:
            draw_dock(painter);
            break;
        case WorkbenchIcon::project_tree:
            draw_project_tree(painter);
            break;
        case WorkbenchIcon::properties:
            draw_properties(painter);
            break;
    }
}

}  // namespace

QIcon make_workbench_icon(WorkbenchIcon icon) {
    QIcon result;
    constexpr std::array<int, 3> kDeviceSizes{16, 32, 64};
    for (const int device_size : kDeviceSizes) {
        QPixmap pixmap(device_size, device_size);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.scale(static_cast<double>(device_size) / 32.0,
                      static_cast<double>(device_size) / 32.0);
        draw_icon(painter, icon);
        painter.end();
        pixmap.setDevicePixelRatio(static_cast<double>(device_size) / 16.0);
        result.addPixmap(pixmap);
    }
    return result;
}

}  // namespace everygraph::presentation
