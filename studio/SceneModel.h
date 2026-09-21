#pragma once

#include <QColor>
#include <QJsonObject>
#include <QRectF>
#include <QSize>
#include <QString>
#include <QVector>

struct SceneNode {
    QString id;
    QString type = "box";
    QRectF rect;
    QString text;
    QColor color = QColor(0, 220, 255);
};

class SceneModel {
public:
    QSize canvasSize{52, 52};
    QVector<SceneNode> nodes;
    quint32 revision = 1;

    QJsonObject toJson() const;
    bool fromJson(const QJsonObject& object, QString* error = nullptr);
    QByteArray payload() const;
    void addLabel();
    void addBox();
    void removeAt(int index);
};
