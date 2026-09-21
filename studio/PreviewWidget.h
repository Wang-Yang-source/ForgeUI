#pragma once

#include <functional>

#include <QWidget>

#include "SceneModel.h"

class PreviewWidget final : public QWidget {
public:
    explicit PreviewWidget(QWidget* parent = nullptr);
    void setScene(SceneModel* scene);
    int selectedIndex() const { return selectedIndex_; }
    void setSelectedIndex(int index);
    std::function<void(int)> onSelectionChanged;
    std::function<void()> onSceneChanged;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QPointF toScene(const QPointF& point) const;
    int hitTest(const QPointF& point) const;

    SceneModel* scene_ = nullptr;
    int selectedIndex_ = -1;
    QPointF dragOffset_;
    bool dragging_ = false;
};
