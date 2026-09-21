#include "PreviewWidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>
#include <QtGlobal>
#include <cmath>

namespace {
QPointF mousePosition(const QMouseEvent* event) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event->position();
#else
    return event->localPos();
#endif
}

QRectF handleRect(const QRectF& rect, PreviewWidget::Handle handle) {
    const QPointF center = [&] {
        switch (handle) {
        case PreviewWidget::Handle::TopLeft: return rect.topLeft();
        case PreviewWidget::Handle::Top: return QPointF(rect.center().x(), rect.top());
        case PreviewWidget::Handle::TopRight: return rect.topRight();
        case PreviewWidget::Handle::Right: return QPointF(rect.right(), rect.center().y());
        case PreviewWidget::Handle::BottomRight: return rect.bottomRight();
        case PreviewWidget::Handle::Bottom: return QPointF(rect.center().x(), rect.bottom());
        case PreviewWidget::Handle::BottomLeft: return rect.bottomLeft();
        case PreviewWidget::Handle::Left: return QPointF(rect.left(), rect.center().y());
        default: return QPointF();
        }
    }();
    return QRectF(center - QPointF(5, 5), QSizeF(10, 10));
}
}

PreviewWidget::PreviewWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(220, 220);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
    const double scale = viewScale();
    const QSizeF canvasSize(scene_->canvasSize.width() * scale, scene_->canvasSize.height() * scale);
    const QPointF origin = canvasOrigin();
    return {(point.x() - origin.x()) / scale, (point.y() - origin.y()) / scale};
}

double PreviewWidget::fitScale() const {
    if (!scene_ || scene_->canvasSize.isEmpty()) return 1.0;
    return std::min(std::max(1, width() - 24) / static_cast<double>(scene_->canvasSize.width()),
                    std::max(1, height() - 24) / static_cast<double>(scene_->canvasSize.height()));
}

double PreviewWidget::viewScale() const { return fitScale() * zoom_; }

QPointF PreviewWidget::canvasOrigin() const {
    const double scale = viewScale();
    const QSizeF size(scene_->canvasSize.width() * scale, scene_->canvasSize.height() * scale);
    return {(width() - size.width()) / 2.0, (height() - size.height()) / 2.0};
}

QRectF PreviewWidget::viewRect(const QRectF& sceneRect) const {
    const double scale = viewScale();
    const QPointF origin = canvasOrigin();
    return {origin.x() + sceneRect.x() * scale, origin.y() + sceneRect.y() * scale,
            sceneRect.width() * scale, sceneRect.height() * scale};
}

int PreviewWidget::hitTest(const QPointF& point) const {
    if (!scene_) return -1;
    for (int index = scene_->nodes.size() - 1; index >= 0; --index)
        if (scene_->rectAt(index, previewTimeMs_).contains(point)) return index;
    return -1;
}

PreviewWidget::Handle PreviewWidget::hitHandle(const QPointF& point) const {
    if (!scene_ || selectedIndex_ < 0 || selectedIndex_ >= scene_->nodes.size()) return Handle::None;
    const QRectF rect = viewRect(scene_->rectAt(selectedIndex_, previewTimeMs_));
    const Handle handles[] = {Handle::TopLeft, Handle::Top, Handle::TopRight, Handle::Right,
                              Handle::BottomRight, Handle::Bottom, Handle::BottomLeft, Handle::Left};
    for (Handle handle : handles) if (handleRect(rect, handle).contains(point)) return handle;
    return Handle::None;
}

void PreviewWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(rect(), QColor(24, 26, 32));
    if (!scene_ || scene_->canvasSize.isEmpty()) return;

    const double scale = viewScale();
    const QSizeF canvasSize(scene_->canvasSize.width() * scale, scene_->canvasSize.height() * scale);
    const QPointF origin = canvasOrigin();
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
        const QRectF animated = scene_->rectAt(index, previewTimeMs_);
        const QRectF r(origin.x() + animated.x() * scale, origin.y() + animated.y() * scale,
                       std::max<qreal>(1, animated.width()) * scale,
                       std::max<qreal>(1, animated.height()) * scale);
        painter.setPen(node.color);
        if (node.type == "label") {
            painter.setFont(QFont("Monospace", std::max(6, static_cast<int>(scale * 1.5)), QFont::Normal));
            painter.drawText(r, Qt::AlignCenter, node.text);
        } else {
            painter.setBrush(node.filled ? node.color : Qt::NoBrush);
            painter.drawRect(r.adjusted(0, 0, -1, -1));
            painter.setBrush(Qt::NoBrush);
        }
        if (index == selectedIndex_) {
            painter.setPen(QPen(Qt::white, std::max(1, static_cast<int>(scale / 2.0)), Qt::DashLine));
            painter.drawRect(r.adjusted(-2, -2, 1, 1));
            painter.setPen(Qt::black);
            painter.setBrush(Qt::white);
            const Handle handles[] = {Handle::TopLeft, Handle::Top, Handle::TopRight, Handle::Right,
                                      Handle::BottomRight, Handle::Bottom, Handle::BottomLeft, Handle::Left};
            for (Handle handle : handles) painter.drawRect(handleRect(r, handle));
            painter.setBrush(Qt::NoBrush);
        }
    }
}

void PreviewWidget::setPreviewTime(uint32_t timeMs) {
    previewTimeMs_ = timeMs;
    update();
}

void PreviewWidget::mousePressEvent(QMouseEvent* event) {
    if (!scene_ || event->button() != Qt::LeftButton) return;
    const QPointF point = toScene(mousePosition(event));
    const Handle handle = hitHandle(mousePosition(event));
    if (handle != Handle::None) {
        resizing_ = true;
        resizeHandle_ = handle;
        resizeStartScene_ = point;
        resizeStartRect_ = scene_->nodes[selectedIndex_].rect;
        return;
    }
    setSelectedIndex(hitTest(point));
    if (selectedIndex_ >= 0) {
        dragOffset_ = point - scene_->nodes[selectedIndex_].rect.topLeft();
        dragging_ = true;
        if (onSelectionChanged) onSelectionChanged(selectedIndex_);
    }
}

void PreviewWidget::mouseMoveEvent(QMouseEvent* event) {
    if (!scene_ || selectedIndex_ < 0) return;
    const QPointF point = toScene(mousePosition(event));
    if (resizing_) {
        QRectF rect = resizeStartRect_;
        const QPointF delta = point - resizeStartScene_;
        const double left = resizeStartRect_.left(), right = resizeStartRect_.right();
        const double top = resizeStartRect_.top(), bottom = resizeStartRect_.bottom();
        if (resizeHandle_ == Handle::TopLeft || resizeHandle_ == Handle::Top || resizeHandle_ == Handle::TopRight)
            rect.setTop(std::min(top + delta.y(), bottom - 1.0));
        if (resizeHandle_ == Handle::BottomLeft || resizeHandle_ == Handle::Bottom || resizeHandle_ == Handle::BottomRight)
            rect.setBottom(std::max(bottom + delta.y(), top + 1.0));
        if (resizeHandle_ == Handle::TopLeft || resizeHandle_ == Handle::Left || resizeHandle_ == Handle::BottomLeft)
            rect.setLeft(std::min(left + delta.x(), right - 1.0));
        if (resizeHandle_ == Handle::TopRight || resizeHandle_ == Handle::Right || resizeHandle_ == Handle::BottomRight)
            rect.setRight(std::max(right + delta.x(), left + 1.0));
        rect.moveLeft(std::floor(rect.left() + 0.5));
        rect.moveTop(std::floor(rect.top() + 0.5));
        rect.setWidth(std::max<qreal>(1, std::floor(rect.width() + 0.5)));
        rect.setHeight(std::max<qreal>(1, std::floor(rect.height() + 0.5)));
        scene_->nodes[selectedIndex_].rect = rect;
        ++scene_->revision;
        update();
        if (onSceneChanged) onSceneChanged();
        return;
    }
    if (!dragging_) return;
    QRectF& rect = scene_->nodes[selectedIndex_].rect;
    const QPointF snapped(std::floor(point.x() - dragOffset_.x() + 0.5),
                          std::floor(point.y() - dragOffset_.y() + 0.5));
    rect.moveTopLeft(snapped);
    rect.setWidth(std::max<qreal>(1, rect.width()));
    rect.setHeight(std::max<qreal>(1, rect.height()));
    ++scene_->revision;
    update();
    if (onSceneChanged) onSceneChanged();
}

void PreviewWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) { dragging_ = false; resizing_ = false; resizeHandle_ = Handle::None; }
}

void PreviewWidget::wheelEvent(QWheelEvent* event) {
    if (!scene_ || event->angleDelta().y() == 0) return;
    const double factor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
    zoom_ = std::max(0.25, std::min(16.0, zoom_ * factor));
    update();
    event->accept();
}
