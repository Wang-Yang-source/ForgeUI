#include "MainWindow.h"

#include "PreviewWidget.h"

#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QJsonDocument>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
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
    auto* ports = new QComboBox(panel);
    ports->addItems(link_.ports());
    auto* connectButton = new QPushButton("Connect serial", panel);
    auto* push = new QPushButton("Push scene", panel);
    layout->addWidget(addLabel);
    layout->addWidget(addBox);
    layout->addSpacing(12);
    layout->addWidget(save);
    layout->addWidget(load);
    layout->addSpacing(12);
    layout->addWidget(ports);
    layout->addWidget(connectButton);
    layout->addWidget(push);
    layout->addStretch();

    setCentralWidget(splitter);
    connect(width, &QSpinBox::valueChanged, this, [this](int value) { scene_.canvasSize.setWidth(value); refreshPreview(); });
    connect(height, &QSpinBox::valueChanged, this, [this](int value) { scene_.canvasSize.setHeight(value); refreshPreview(); });
    connect(addLabel, &QPushButton::clicked, this, [this] { scene_.addLabel(); refreshPreview(); });
    connect(addBox, &QPushButton::clicked, this, [this] { scene_.addBox(); refreshPreview(); });
    connect(save, &QPushButton::clicked, this, &MainWindow::saveScene);
    connect(load, &QPushButton::clicked, this, &MainWindow::loadScene);
    connect(connectButton, &QPushButton::clicked, this, [this, ports] {
        if (!link_.open(ports->currentText())) QMessageBox::warning(this, "ForgeUI", "Unable to open serial port");
    });
    connect(push, &QPushButton::clicked, this, [this] {
        if (!link_.pushScene(scene_.revision, scene_.payload()))
            QMessageBox::warning(this, "ForgeUI", "Connect to an ESP32 serial port first");
    });
    refreshPreview();
}

void MainWindow::refreshPreview() { preview_->setScene(&scene_); }

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
