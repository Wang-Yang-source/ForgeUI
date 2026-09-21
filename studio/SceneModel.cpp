#include "SceneModel.h"

#include <QJsonArray>
#include <QJsonDocument>

QJsonObject SceneModel::toJson() const {
    QJsonObject root;
    root["version"] = 1;
    root["revision"] = static_cast<qint64>(revision);
    root["width"] = canvasSize.width();
    root["height"] = canvasSize.height();

    QJsonArray items;
    for (const SceneNode& node : nodes) {
        QJsonObject item;
        item["id"] = node.id;
        item["type"] = node.type;
        item["x"] = node.rect.x();
        item["y"] = node.rect.y();
        item["width"] = node.rect.width();
        item["height"] = node.rect.height();
        item["text"] = node.text;
        item["color"] = node.color.name(QColor::HexArgb);
        items.append(item);
    }
    root["nodes"] = items;
    return root;
}

bool SceneModel::fromJson(const QJsonObject& root, QString* error) {
    const int width = root.value("width").toInt(0);
    const int height = root.value("height").toInt(0);
    if (width <= 0 || height <= 0 || width > 4096 || height > 4096) {
        if (error) *error = "Invalid canvas dimensions";
        return false;
    }

    QVector<SceneNode> parsed;
    const QJsonArray items = root.value("nodes").toArray();
    for (const QJsonValue& value : items) {
        const QJsonObject item = value.toObject();
        SceneNode node;
        node.id = item.value("id").toString();
        node.type = item.value("type").toString("box");
        node.rect = QRectF(item.value("x").toDouble(), item.value("y").toDouble(),
                           item.value("width").toDouble(), item.value("height").toDouble());
        node.text = item.value("text").toString();
        node.color = QColor(item.value("color").toString("#ff00dcff"));
        if (node.id.isEmpty() || node.rect.width() < 0 || node.rect.height() < 0) {
            if (error) *error = "Invalid scene node";
            return false;
        }
        parsed.append(node);
    }

    canvasSize = QSize(width, height);
    nodes = parsed;
    revision = static_cast<quint32>(root.value("revision").toInteger(1));
    return true;
}

QByteArray SceneModel::payload() const {
    return QJsonDocument(toJson()).toJson(QJsonDocument::Compact);
}

void SceneModel::addLabel() {
    SceneNode node;
    node.id = QString("label_%1").arg(nodes.size() + 1);
    node.type = "label";
    node.rect = QRectF(4, 18 + nodes.size() * 8, 44, 8);
    node.text = "FORGEUI";
    nodes.append(node);
    ++revision;
}

void SceneModel::addBox() {
    SceneNode node;
    node.id = QString("box_%1").arg(nodes.size() + 1);
    node.type = "box";
    node.rect = QRectF(3, 3, canvasSize.width() - 6, canvasSize.height() - 6);
    nodes.append(node);
    ++revision;
}

void SceneModel::removeAt(int index) {
    if (index < 0 || index >= nodes.size()) return;
    nodes.removeAt(index);
    ++revision;
}
