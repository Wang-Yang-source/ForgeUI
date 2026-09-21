#pragma once

#include <QMainWindow>

#include "SceneModel.h"
#include "StudioLink.h"

class PreviewWidget;
class QLineEdit;
class QLabel;
class QPushButton;
class QSpinBox;
class QSlider;
class QTimer;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void saveScene();
    void loadScene();
    void refreshPreview();
    void refreshInspector();
    int selectedIndex() const;

    SceneModel scene_;
    PreviewWidget* preview_ = nullptr;
    QLabel* selectedLabel_ = nullptr;
    QSpinBox* nodeX_ = nullptr;
    QSpinBox* nodeY_ = nullptr;
    QSpinBox* nodeWidth_ = nullptr;
    QSpinBox* nodeHeight_ = nullptr;
    QLineEdit* nodeText_ = nullptr;
    QPushButton* nodeColor_ = nullptr;
    QSlider* timeline_ = nullptr;
    QTimer* timer_ = nullptr;
    StudioLink link_;
};
