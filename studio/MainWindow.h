#pragma once

#include <QMainWindow>

#include "SceneModel.h"
#include "StudioLink.h"

class PreviewWidget;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void saveScene();
    void loadScene();
    void refreshPreview();

    SceneModel scene_;
    PreviewWidget* preview_ = nullptr;
    StudioLink link_;
};
