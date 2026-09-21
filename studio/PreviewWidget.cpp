#include "PreviewWidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QtGlobal>

namespace {
QPointF mousePosition(const QMouseEvent* event) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event->position();
#else
    return event->localPos();
#endif
}
}

PreviewWidget::PreviewWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(420, 420);
    setAutoFillBackground(false);
}

void PreviewWidget::setScene(SceneModel* scene) {
    scene_ = scene;
    if (!scene_ || selectedIndex_ >= scene_->nodes.size()) selectedIndex_ = -1;
    update();
}

void PreviewWidget::setSelectedIndex(int index) {
    selectedIndex_ = (scene_ && index >= 0 && index < scene_->nodes.size()) ? index : -1;
    update();
}

QPointF PreviewWidget::toScene(const QPointF& point) const {
    if (!scene_ || scene_->canvasSize.isEmpty()) return {};
    const double scale = std::min(width() / static_cast<double>(scene_->canvasSize.width()),
                                  height() / static_cast<double>(scene_->canvasSize.height()));
    const QSizeF canvasSize(scene_->canvasSize.width() * scale, scene_->canvasSize.height() * scale);
    const QPointF origin((width() - canvasSize.width()) / 2.0, (height() - canvasSize.height()) / 2.0);
    return {(point.x() - origin.x()) / scale, (point.y() - origin.y()) / scale};
}

int PreviewWidget::hitTest(const QPointF& point) const {
    if (!scene_) return -1;
    for (int index = scene_->nodes.size() - 1; index >= 0; --index)
        if (scene_->nodes[index].rect.contains(point)) return index;
    return -1;
}

void PreviewWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(rect(), QColor(24, 26, 32));
    if (!scene_ || scene_->canvasSize.isEmpty()) return;

    const double scale = std::min(width() / static_cast<double>(scene_->canvasSize.width()),
                                  height() / static_cast<double>(scene_->canvasSize.height()));
    const QSizeF canvasSize(scene_->canvasSize.width() * scale, scene_->canvasSize.height() * scale);
    const QPointF origin((width() - canvasSize.width()) / 2.0, (height() - canvasSize.height()) / 2.0);
    const QRectF canvasRect(origin, canvasSize);

    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.fillRect(canvasRect, QColor(5, 8, 12));
    painter.setPen(QColor(80, 88, 100));
    painter.drawRect(canvasRect.adjusted(0, 0, -1, -1));

    if (scale >= 8.0) {
        painter.setPen(QColor(30, 38, 48));
        for (int x = 1; x < scene_->canvasSize.width(); ++x)
            painter.drawLine(origin.x() + x * scale, origin.y(), origin.x() + x * scale, origin.y() + canvasSize.height());
        for (int y = 1; y < scene_->canvasSize.height(); ++y)
            painter.drawLine(origin.x(), origin.y() + y * scale, origin.x() + canvasSize.width(), origin.y() + y * scale);
    }

    for (int index = 0; index < scene_->nodes.size(); ++index) {
        const SceneNode& node = scene_->nodes[index];
        const QRectF r(origin.x() + node.rect.x() * scale,
                       origin.y() + node.rect.y() * scale,
                       node.rect.width() * scale,
                       node.rect.height() * scale);
        painter.setPen(node.color);
        if (node.type == "label") {
            painter.setFont(QFont("Monospace", std::max(6, static_cast<int>(scale * 1.5)), QFont::Normal));
            painter.drawText(r, Qt::AlignCenter, node.text);
        } else {
            painter.drawRect(r.adjusted(0, 0, -1, -1));
        }
        if (index == selectedIndex_) {
            painter.setPen(QPen(Qt::white, std::max(1, static_cast<int>(scale / 2.0)), Qt::DashLine));
            painter.drawRect(r.adjusted(-2, -2, 1, 1));
        }
    }
}

void PreviewWidget::mousePressEvent(QMouseEvent* event) {
    if (!scene_ || event->button() != Qt::LeftButton) return;
    const QPointF point = toScene(mousePosition(event));
    setSelectedIndex(hitTest(point));
    if (selectedIndex_ >= 0) {
        dragOffset_ = point - scene_->nodes[selectedIndex_].rect.topLeft();
        dragging_ = true;
        if (onSelectionChanged) onSelectionChanged(selectedIndex_);
    }
}

void PreviewWidget::mouseMoveEvent(QMouseEvent* event) {
    if (!dragging_ || !scene_ || selectedIndex_ < 0) return;
    const QPointF point = toScene(mousePosition(event));
    scene_->nodes[selectedIndex_].rect.moveTopLeft(point - dragOffset_);
    ++scene_->revision;
    update();
    if (onSceneChanged) onSceneChanged();
}

void PreviewWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) dragging_ = false;
}
