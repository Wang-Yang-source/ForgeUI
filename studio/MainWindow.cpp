#include "MainWindow.h"

#include "PreviewWidget.h"
#include "Exporter.h"

#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QColorDialog>
#include <QCheckBox>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>
#include <QSplitter>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("ForgeUI Studio");
    resize(1280, 820);
    setMinimumSize(960, 640);

    auto* splitter = new QSplitter(this);
    preview_ = new PreviewWidget(splitter);
    auto* panelScroll = new QScrollArea(splitter);
    panelScroll->setWidgetResizable(true);
    panelScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    panelScroll->setMinimumWidth(300);
    panelScroll->setMaximumWidth(380);
    auto* panel = new QWidget;
    panelScroll->setWidget(panel);
    auto* layout = new QVBoxLayout(panel);

    layout->addWidget(new QLabel("Canvas", panel));
    auto* width = new QSpinBox(panel);
    width->setRange(1, 4096);
    width->setValue(scene_.canvasSize.width());
    auto* height = new QSpinBox(panel);
    height->setRange(1, 4096);
    height->setValue(scene_.canvasSize.height());
    layout->addWidget(width);
    layout->addWidget(height);
    auto* profile = new QComboBox(panel);
    auto addProfile = [profile](const QString& name, QSize logical, QSize physical, int scale = 1) {
        QVariantMap data;
        data["logical"] = logical;
        data["physical"] = physical;
        data["scale"] = scale;
        profile->addItem(name, data);
    };
    addProfile("Custom", {52, 52}, {52, 52});
    addProfile("LED 52×52", {52, 52}, {52, 52});
    addProfile("LED 104×104 (2×)", {52, 52}, {104, 104}, 2);
    addProfile("LED 260×260 (5×)", {52, 52}, {260, 260}, 5);
    addProfile("OLED 128×32", {128, 32}, {128, 32});
    addProfile("OLED 128×64", {128, 64}, {128, 64});
    addProfile("OLED 128×128", {128, 128}, {128, 128});
    addProfile("TFT 128×160", {128, 160}, {128, 160});
    addProfile("TFT 240×240", {240, 240}, {240, 240});
    addProfile("TFT 240×320", {240, 320}, {240, 320});
    addProfile("TFT 320×240", {320, 240}, {320, 240});
    addProfile("TFT 320×480", {320, 480}, {320, 480});
    addProfile("TFT 480×272", {480, 272}, {480, 272});
    addProfile("TFT 800×480", {800, 480}, {800, 480});
    layout->addWidget(profile);

    auto* addLabel = new QPushButton("Add pixel label", panel);
    auto* addBox = new QPushButton("Add box", panel);
    auto* save = new QPushButton("Save scene", panel);
    auto* load = new QPushButton("Load scene", panel);
    auto* exportForgeUi = new QPushButton("Export ForgeUI C++", panel);
    auto* exportLvgl = new QPushButton("Export LVGL C", panel);
    auto* exportSlint = new QPushButton("Export Slint", panel);
    auto* ports = new QComboBox(panel);
    ports->addItems(link_.ports());
    auto* connectButton = new QPushButton("Connect serial", panel);
    auto* push = new QPushButton("Push scene", panel);
    layout->addWidget(addLabel);
    layout->addWidget(addBox);
    layout->addSpacing(12);
    layout->addWidget(save);
    layout->addWidget(load);
    layout->addWidget(exportForgeUi);
    layout->addWidget(exportLvgl);
    layout->addWidget(exportSlint);
    layout->addSpacing(12);
    layout->addWidget(ports);
    layout->addWidget(connectButton);
    layout->addWidget(push);
    layout->addSpacing(12);
    selectedLabel_ = new QLabel("No node selected", panel);
    layout->addWidget(selectedLabel_);
    auto makeSpin = [panel](int value) {
        auto* spin = new QSpinBox(panel);
        spin->setRange(1, 4096);
        spin->setValue(value);
        return spin;
    };
    nodeX_ = makeSpin(0);
    nodeY_ = makeSpin(0);
    nodeWidth_ = makeSpin(0);
    nodeHeight_ = makeSpin(0);
    layout->addWidget(new QLabel("X", panel)); layout->addWidget(nodeX_);
    layout->addWidget(new QLabel("Y", panel)); layout->addWidget(nodeY_);
    layout->addWidget(new QLabel("Width", panel)); layout->addWidget(nodeWidth_);
    layout->addWidget(new QLabel("Height", panel)); layout->addWidget(nodeHeight_);
    nodeText_ = new QLineEdit(panel);
    nodeText_->setPlaceholderText("Label text");
    layout->addWidget(nodeText_);
    nodeColor_ = new QPushButton("Change color", panel);
    layout->addWidget(nodeColor_);
    nodeFilled_ = new QCheckBox("Fill shape", panel);
    layout->addWidget(nodeFilled_);
    auto* remove = new QPushButton("Delete selected", panel);
    layout->addWidget(remove);
    layout->addSpacing(12);
    auto* addAnimation = new QPushButton("Add entrance animation", panel);
    auto* play = new QPushButton("Play animation", panel);
    auto* pause = new QPushButton("Pause", panel);
    auto* stepBack = new QPushButton("Frame -16 ms", panel);
    auto* stepForward = new QPushButton("Frame +16 ms", panel);
    loop_ = new QCheckBox("Loop", panel);
    timeline_ = new QSlider(Qt::Horizontal, panel);
    timeline_->setRange(0, 2500);
    timeline_->setValue(0);
    layout->addWidget(addAnimation);
    layout->addWidget(play);
    layout->addWidget(pause);
    layout->addWidget(stepBack);
    layout->addWidget(stepForward);
    layout->addWidget(loop_);
    layout->addWidget(timeline_);
    layout->addStretch();

    setCentralWidget(splitter);
    connect(width, &QSpinBox::valueChanged, this, [this](int value) { scene_.canvasSize.setWidth(value); refreshPreview(); });
    connect(height, &QSpinBox::valueChanged, this, [this](int value) { scene_.canvasSize.setHeight(value); refreshPreview(); });
    connect(profile, qOverload<int>(&QComboBox::currentIndexChanged), this, [this, profile, width, height](int index) {
        if (index == 0) return;
        const QVariantMap data = profile->itemData(index).toMap();
        const QSize logical = data.value("logical").toSize();
        scene_.physicalSize = data.value("physical").toSize();
        scene_.pixelScale = qMax(1, data.value("scale").toInt());
        width->setValue(logical.width());
        height->setValue(logical.height());
    });
    connect(addLabel, &QPushButton::clicked, this, [this] { scene_.addLabel(); refreshPreview(); });
    connect(addBox, &QPushButton::clicked, this, [this] { scene_.addBox(); refreshPreview(); });
    connect(save, &QPushButton::clicked, this, &MainWindow::saveScene);
    connect(load, &QPushButton::clicked, this, &MainWindow::loadScene);
    connect(exportForgeUi, &QPushButton::clicked, this, [this] {
        const QString path = QFileDialog::getSaveFileName(this, "Export ForgeUI C++", {}, "C++ source (*.cpp)");
        if (path.isEmpty()) return;
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
            file.write(forgeui_studio::exportForgeUiSource(scene_).toUtf8());
    });
    auto exportText = [this](const QString& title, const QString& filter, const QString& source) {
        const QString path = QFileDialog::getSaveFileName(this, title, {}, filter);
        if (path.isEmpty()) return;
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) file.write(source.toUtf8());
    };
    connect(exportLvgl, &QPushButton::clicked, this, [this, exportText] {
        exportText("Export LVGL C", "C source (*.c)", forgeui_studio::exportLvglSource(scene_));
    });
    connect(exportSlint, &QPushButton::clicked, this, [this, exportText] {
        exportText("Export Slint", "Slint source (*.slint)", forgeui_studio::exportSlintSource(scene_));
    });
    connect(connectButton, &QPushButton::clicked, this, [this, ports] {
        if (!link_.open(ports->currentText())) QMessageBox::warning(this, "ForgeUI", "Unable to open serial port");
    });
    connect(push, &QPushButton::clicked, this, [this] {
        if (!link_.pushScene(scene_.revision, scene_.payload()))
            QMessageBox::warning(this, "ForgeUI", "Connect to an ESP32 serial port first");
    });
    preview_->onSelectionChanged = [this](int) { refreshInspector(); };
    preview_->onSceneChanged = [this] { refreshInspector(); };
    auto updateGeometry = [this] {
        const int index = selectedIndex();
        if (index < 0) return;
        SceneNode& node = scene_.nodes[index];
        node.rect.setX(qMax(0, nodeX_->value())); node.rect.setY(qMax(0, nodeY_->value()));
        node.rect.setWidth(qMax(1, nodeWidth_->value())); node.rect.setHeight(qMax(1, nodeHeight_->value()));
        ++scene_.revision;
        refreshPreview();
    };
    connect(nodeX_, qOverload<int>(&QSpinBox::valueChanged), this, [updateGeometry](int) { updateGeometry(); });
    connect(nodeY_, qOverload<int>(&QSpinBox::valueChanged), this, [updateGeometry](int) { updateGeometry(); });
    connect(nodeWidth_, qOverload<int>(&QSpinBox::valueChanged), this, [updateGeometry](int) { updateGeometry(); });
    connect(nodeHeight_, qOverload<int>(&QSpinBox::valueChanged), this, [updateGeometry](int) { updateGeometry(); });
    connect(nodeText_, &QLineEdit::editingFinished, this, [this] {
        const int index = selectedIndex();
        if (index < 0) return;
        scene_.nodes[index].text = nodeText_->text();
        ++scene_.revision;
        refreshPreview();
    });
    connect(nodeColor_, &QPushButton::clicked, this, [this] {
        const int index = selectedIndex();
        if (index < 0) return;
        const QColor color = QColorDialog::getColor(scene_.nodes[index].color, this, "Node color");
        if (!color.isValid()) return;
        scene_.nodes[index].color = color;
        ++scene_.revision;
        refreshPreview();
        refreshInspector();
    });
    connect(nodeFilled_, &QCheckBox::toggled, this, [this](bool filled) {
        const int index = selectedIndex();
        if (index < 0 || scene_.nodes[index].type == "label") return;
        scene_.nodes[index].filled = filled;
        ++scene_.revision;
        refreshPreview();
    });
    connect(remove, &QPushButton::clicked, this, [this] {
        const int index = selectedIndex();
        if (index < 0) return;
        scene_.removeAt(index);
        preview_->setSelectedIndex(-1);
        refreshPreview();
        refreshInspector();
    });
    connect(addAnimation, &QPushButton::clicked, this, [this] {
        scene_.addEntranceAnimation(selectedIndex());
        refreshPreview();
    });
    connect(timeline_, &QSlider::valueChanged, this, [this](int value) {
        preview_->setPreviewTime(static_cast<uint32_t>(value));
    });
    timer_ = new QTimer(this);
    timer_->setInterval(16);
    connect(timer_, &QTimer::timeout, this, [this] {
        const int next = timeline_->value() + 16;
        if (next >= timeline_->maximum()) {
            if (loop_->isChecked()) timeline_->setValue(0);
            else { timeline_->setValue(timeline_->maximum()); timer_->stop(); }
        } else timeline_->setValue(next);
    });
    connect(play, &QPushButton::clicked, this, [this] { timer_->start(); });
    connect(pause, &QPushButton::clicked, this, [this] { timer_->stop(); });
    connect(stepBack, &QPushButton::clicked, this, [this] { timeline_->setValue(std::max(0, timeline_->value() - 16)); });
    connect(stepForward, &QPushButton::clicked, this, [this] { timeline_->setValue(std::min(timeline_->maximum(), timeline_->value() + 16)); });
    refreshPreview();
    refreshInspector();
}

void MainWindow::refreshPreview() { preview_->setScene(&scene_); }

int MainWindow::selectedIndex() const { return preview_ ? preview_->selectedIndex() : -1; }

void MainWindow::refreshInspector() {
    const int index = selectedIndex();
    const bool enabled = index >= 0 && index < scene_.nodes.size();
    selectedLabel_->setText(enabled ? scene_.nodes[index].id : "No node selected");
    for (QWidget* widget : {static_cast<QWidget*>(nodeX_), static_cast<QWidget*>(nodeY_),
                            static_cast<QWidget*>(nodeWidth_), static_cast<QWidget*>(nodeHeight_),
                            static_cast<QWidget*>(nodeText_), static_cast<QWidget*>(nodeColor_),
                            static_cast<QWidget*>(nodeFilled_)}) widget->setEnabled(enabled);
    if (!enabled) return;
    const QRectF rect = scene_.nodes[index].rect;
    const QSignalBlocker blockX(nodeX_);
    const QSignalBlocker blockY(nodeY_);
    const QSignalBlocker blockWidth(nodeWidth_);
    const QSignalBlocker blockHeight(nodeHeight_);
    const QSignalBlocker blockText(nodeText_);
    const QSignalBlocker blockFilled(nodeFilled_);
    nodeX_->setValue(static_cast<int>(rect.x()));
    nodeY_->setValue(static_cast<int>(rect.y()));
    nodeWidth_->setValue(static_cast<int>(rect.width()));
    nodeHeight_->setValue(static_cast<int>(rect.height()));
    nodeText_->setText(scene_.nodes[index].text);
    nodeColor_->setText(QString("Color: %1").arg(scene_.nodes[index].color.name(QColor::HexRgb)));
    nodeFilled_->setChecked(scene_.nodes[index].filled);
    nodeFilled_->setEnabled(scene_.nodes[index].type != "label");
}

void MainWindow::saveScene() {
    const QString path = QFileDialog::getSaveFileName(this, "Save ForgeUI scene", {}, "ForgeUI scene (*.json)");
    if (path.isEmpty()) return;
    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) file.write(QJsonDocument(scene_.toJson()).toJson(QJsonDocument::Indented));
}

void MainWindow::loadScene() {
    const QString path = QFileDialog::getOpenFileName(this, "Load ForgeUI scene", {}, "ForgeUI scene (*.json)");
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return;
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    QString error = parseError.error == QJsonParseError::NoError ? QString() : parseError.errorString();
    if (!document.isObject() || !scene_.fromJson(document.object(), &error)) {
        QMessageBox::warning(this, "ForgeUI", error.isEmpty() ? "Invalid scene" : error);
        return;
    }
    refreshPreview();
}
