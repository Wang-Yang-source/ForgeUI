#pragma once

#include <QWidget>

#include "SceneModel.h"

class PreviewWidget final : public QWidget {
public:
    explicit PreviewWidget(QWidget* parent = nullptr);
    void setScene(const SceneModel* scene);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    const SceneModel* scene_ = nullptr;
};
