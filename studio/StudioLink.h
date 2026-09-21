#pragma once

#include <QByteArray>
#include <QStringList>
#include <QString>
#include <cstdint>
#include <forgeui/Protocol.h>

class QSerialPort;

class StudioLink {
public:
    StudioLink();
    ~StudioLink();

    QStringList ports() const;
    bool open(const QString& portName, int baud = 921600);
    void close();
    bool isOpen() const;
    bool pushScene(quint32 revision, const QByteArray& payload);
    bool pushAsset(quint32 assetId, forgeui::protocol::AssetFormat format,
                   quint16 width, quint16 height, const QByteArray& bytes);

private:
    QSerialPort* serial_ = nullptr;
};
