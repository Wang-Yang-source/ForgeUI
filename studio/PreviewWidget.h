#pragma once

#include <functional>

#include <QWidget>
#include <QPointF>

#include "SceneModel.h"

class PreviewWidget final : public QWidget {
public:
    explicit PreviewWidget(QWidget* parent = nullptr);
    void setScene(SceneModel* scene);
    int selectedIndex() const { return selectedIndex_; }
    void setSelectedIndex(int index);
    void setPreviewTime(uint32_t timeMs);
    std::function<void(int)> onSelectionChanged;
    std::function<void()> onSceneChanged;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

public:
    enum class Handle { None, TopLeft, Top, TopRight, Right, BottomRight, Bottom, BottomLeft, Left };

private:
    double fitScale() const;
    double viewScale() const;
    QPointF canvasOrigin() const;
    QRectF viewRect(const QRectF& sceneRect) const;
    QPointF toScene(const QPointF& point) const;
    int hitTest(const QPointF& point) const;
    Handle hitHandle(const QPointF& point) const;

    SceneModel* scene_ = nullptr;
    int selectedIndex_ = -1;
    QPointF dragOffset_;
    QPointF resizeStartScene_;
    QRectF resizeStartRect_;
    bool dragging_ = false;
    bool resizing_ = false;
    Handle resizeHandle_ = Handle::None;
    double zoom_ = 1.0;
    uint32_t previewTimeMs_ = 0;
};
