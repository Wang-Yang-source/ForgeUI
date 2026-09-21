#include "StudioLink.h"

#include <QSerialPort>
#include <QSerialPortInfo>
#include <algorithm>

#include <forgeui/Protocol.h>

StudioLink::StudioLink() : serial_(new QSerialPort) {}
StudioLink::~StudioLink() { delete serial_; }

QStringList StudioLink::ports() const {
    QStringList result;
    for (const QSerialPortInfo& info : QSerialPortInfo::availablePorts()) result.append(info.portName());
    return result;
}

bool StudioLink::open(const QString& portName, int baud) {
    close();
    serial_->setPortName(portName);
    serial_->setBaudRate(baud);
    return serial_->open(QIODevice::ReadWrite);
}

void StudioLink::close() { if (serial_->isOpen()) serial_->close(); }
bool StudioLink::isOpen() const { return serial_->isOpen(); }

bool StudioLink::pushScene(quint32 revision, const QByteArray& payload) {
    if (!isOpen()) return false;
    if (static_cast<size_t>(payload.size()) > forgeui::protocol::maxScenePayload) return false;
    QByteArray packet;
    packet.resize(static_cast<qsizetype>(forgeui::protocol::encodedSize(static_cast<size_t>(payload.size()))));
    if (!forgeui::protocol::encodeScene(reinterpret_cast<uint8_t*>(packet.data()), static_cast<size_t>(packet.size()),
                                        revision, reinterpret_cast<const uint8_t*>(payload.constData()),
                                        static_cast<size_t>(payload.size()))) return false;
    return serial_->write(packet) == packet.size() && serial_->waitForBytesWritten(1000);
}

bool StudioLink::pushAsset(quint32 assetId, forgeui::protocol::AssetFormat format,
                           quint16 width, quint16 height, const QByteArray& bytes) {
    if (!isOpen() || bytes.isEmpty()) return false;
    const size_t total = static_cast<size_t>(bytes.size());
    const uint32_t fullCrc = forgeui::protocol::crc32(reinterpret_cast<const uint8_t*>(bytes.constData()), total);
    constexpr size_t chunk = 4096;
    for (size_t offset = 0; offset < total; offset += chunk) {
        const size_t size = std::min(chunk, total - offset);
        QByteArray packet(static_cast<qsizetype>(forgeui::protocol::encodedAssetSize(size)), Qt::Uninitialized);
        if (!forgeui::protocol::encodeAssetChunk(reinterpret_cast<uint8_t*>(packet.data()), static_cast<size_t>(packet.size()),
                assetId, static_cast<uint32_t>(offset), static_cast<uint32_t>(total), format, width, height,
                fullCrc, reinterpret_cast<const uint8_t*>(bytes.constData()) + offset, size)) return false;
        if (serial_->write(packet) != packet.size() || !serial_->waitForBytesWritten(1000)) return false;
    }
    return true;
}
