#include "MainWindow.h"

#include "PreviewWidget.h"
#include "Exporter.h"

#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QColorDialog>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("ForgeUI Studio");
    resize(920, 640);

    auto* splitter = new QSplitter(this);
    preview_ = new PreviewWidget(splitter);
    auto* panel = new QWidget(splitter);
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
        spin->setRange(-4096, 4096);
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
    auto* remove = new QPushButton("Delete selected", panel);
    layout->addWidget(remove);
    layout->addStretch();

    setCentralWidget(splitter);
    connect(width, &QSpinBox::valueChanged, this, [this](int value) { scene_.canvasSize.setWidth(value); refreshPreview(); });
    connect(height, &QSpinBox::valueChanged, this, [this](int value) { scene_.canvasSize.setHeight(value); refreshPreview(); });
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
        node.rect.setX(nodeX_->value()); node.rect.setY(nodeY_->value());
        node.rect.setWidth(nodeWidth_->value()); node.rect.setHeight(nodeHeight_->value());
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
    connect(remove, &QPushButton::clicked, this, [this] {
        const int index = selectedIndex();
        if (index < 0) return;
        scene_.removeAt(index);
        preview_->setSelectedIndex(-1);
        refreshPreview();
        refreshInspector();
    });
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
                            static_cast<QWidget*>(nodeText_), static_cast<QWidget*>(nodeColor_)}) widget->setEnabled(enabled);
    if (!enabled) return;
    const QRectF rect = scene_.nodes[index].rect;
    const QSignalBlocker blockX(nodeX_);
    const QSignalBlocker blockY(nodeY_);
    const QSignalBlocker blockWidth(nodeWidth_);
    const QSignalBlocker blockHeight(nodeHeight_);
    const QSignalBlocker blockText(nodeText_);
    nodeX_->setValue(static_cast<int>(rect.x()));
    nodeY_->setValue(static_cast<int>(rect.y()));
    nodeWidth_->setValue(static_cast<int>(rect.width()));
    nodeHeight_->setValue(static_cast<int>(rect.height()));
    nodeText_->setText(scene_.nodes[index].text);
    nodeColor_->setText(QString("Color: %1").arg(scene_.nodes[index].color.name(QColor::HexRgb)));
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
