#include "SceneModel.h"

#include <QJsonArray>
#include <QJsonDocument>

namespace {
int sample(int from, int to, uint32_t elapsed, uint32_t duration) {
    if (elapsed >= duration) return to;
    const int64_t delta = static_cast<int64_t>(to) - from;
    return static_cast<int>(from + (delta * elapsed) / duration);
}
}

QJsonObject SceneModel::toJson() const {
    QJsonObject root;
    root["version"] = 1;
    root["revision"] = static_cast<qint64>(revision);
    root["width"] = canvasSize.width();
    root["height"] = canvasSize.height();
    root["physical_width"] = physicalSize.width();
    root["physical_height"] = physicalSize.height();
    root["pixel_scale"] = pixelScale;

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
        item["filled"] = node.filled;
        items.append(item);
    }
    root["nodes"] = items;
    QJsonArray animations;
    for (const SceneAnimation& animation : this->animations) {
        QJsonObject item;
        item["node"] = animation.nodeIndex;
        item["property"] = animation.property;
        item["from"] = animation.from;
        item["to"] = animation.to;
        item["delay"] = static_cast<qint64>(animation.delayMs);
        item["duration"] = static_cast<qint64>(animation.durationMs);
        animations.append(item);
    }
    root["animations"] = animations;
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
        node.filled = item.value("filled").toBool(true);
        if (node.id.isEmpty() || node.rect.width() < 1 || node.rect.height() < 1) {
            if (error) *error = "Invalid scene node";
            return false;
        }
        parsed.append(node);
    }

    QVector<SceneAnimation> parsedAnimations;
    for (const QJsonValue& value : root.value("animations").toArray()) {
        const QJsonObject item = value.toObject();
        SceneAnimation animation;
        animation.nodeIndex = item.value("node").toInt(-1);
        animation.property = item.value("property").toString("y");
        animation.from = item.value("from").toInt();
        animation.to = item.value("to").toInt();
        animation.delayMs = static_cast<uint32_t>(item.value("delay").toInteger(0));
        animation.durationMs = static_cast<uint32_t>(item.value("duration").toInteger(800));
        if (animation.nodeIndex < 0 || animation.nodeIndex >= parsed.size() || animation.durationMs == 0) {
            if (error) *error = "Invalid scene animation";
            return false;
        }
        parsedAnimations.append(animation);
    }

    canvasSize = QSize(width, height);
    physicalSize = QSize(root.value("physical_width").toInt(width), root.value("physical_height").toInt(height));
    pixelScale = qMax(1, root.value("pixel_scale").toInt(1));
    nodes = parsed;
    animations = parsedAnimations;
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
    const qreal x = qMax<qreal>(0, (canvasSize.width() - 1) / 2.0);
    const qreal y = qMax<qreal>(0, (canvasSize.height() - 1) / 2.0);
    node.rect = QRectF(x, y, 1, 1);
    nodes.append(node);
    ++revision;
}

void SceneModel::removeAt(int index) {
    if (index < 0 || index >= nodes.size()) return;
    nodes.removeAt(index);
    for (int i = animations.size() - 1; i >= 0; --i) {
        if (animations[i].nodeIndex == index) animations.removeAt(i);
        else if (animations[i].nodeIndex > index) --animations[i].nodeIndex;
    }
    ++revision;
}

void SceneModel::addEntranceAnimation(int nodeIndex) {
    if (nodeIndex < 0 || nodeIndex >= nodes.size()) return;
    SceneAnimation animation;
    animation.nodeIndex = nodeIndex;
    animation.property = "y";
    animation.to = static_cast<int>(nodes[nodeIndex].rect.y());
    animation.from = animation.to - 12;
    animation.durationMs = 800;
    animations.append(animation);
    ++revision;
}

QRectF SceneModel::rectAt(int nodeIndex, uint32_t timeMs) const {
    if (nodeIndex < 0 || nodeIndex >= nodes.size()) return {};
    QRectF result = nodes[nodeIndex].rect;
    for (const SceneAnimation& animation : animations) {
        if (animation.nodeIndex != nodeIndex || timeMs < animation.delayMs) continue;
        const int value = sample(animation.from, animation.to, timeMs - animation.delayMs, animation.durationMs);
        if (animation.property == "x") result.moveLeft(value);
        else if (animation.property == "y") result.moveTop(value);
        else if (animation.property == "width") result.setWidth(value);
        else if (animation.property == "height") result.setHeight(value);
    }
    return result;
}

QStringList SceneModel::validationWarnings() const {
    QStringList warnings;
    const qint64 pixels = static_cast<qint64>(canvasSize.width()) * canvasSize.height();
    if (canvasSize.width() <= 0 || canvasSize.height() <= 0) warnings << "Canvas dimensions must be positive";
    if (pixels * 2 > 1024 * 1024) warnings << "RGB565 framebuffer exceeds 1 MiB";
    for (const SceneNode& node : nodes) {
        if (!QRectF(QPointF(0, 0), canvasSize).contains(node.rect))
            warnings << QString("Node '%1' is outside the canvas").arg(node.id);
    }
    for (const SceneAnimation& animation : animations) {
        if (animation.nodeIndex < 0 || animation.nodeIndex >= nodes.size())
            warnings << "Animation references a missing node";
        if (animation.durationMs == 0) warnings << "Animation duration must be non-zero";
    }
    return warnings;
}
