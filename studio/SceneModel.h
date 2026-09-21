#pragma once

#include <QColor>
#include <QJsonObject>
#include <QRectF>
#include <QSize>
#include <QString>
#include <QVector>
#include <cstdint>

struct SceneNode {
    QString id;
    QString type = "box";
    QRectF rect;
    QString text;
    QColor color = QColor(0, 220, 255);
};

struct SceneAnimation {
    int nodeIndex = -1;
    QString property = "y";
    int from = 0;
    int to = 0;
    uint32_t delayMs = 0;
    uint32_t durationMs = 800;
};

class SceneModel {
public:
    QSize canvasSize{52, 52};
    QVector<SceneNode> nodes;
    QVector<SceneAnimation> animations;
    quint32 revision = 1;

    QJsonObject toJson() const;
    bool fromJson(const QJsonObject& object, QString* error = nullptr);
    QByteArray payload() const;
    void addLabel();
    void addBox();
    void removeAt(int index);
    void addEntranceAnimation(int nodeIndex);
    QRectF rectAt(int nodeIndex, uint32_t timeMs) const;
};
