#include "PreviewWidget.h"

#include <QPainter>

PreviewWidget::PreviewWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(420, 420);
    setAutoFillBackground(false);
}

void PreviewWidget::setScene(const SceneModel* scene) {
    scene_ = scene;
    update();
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

    for (const SceneNode& node : scene_->nodes) {
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
    }
}
