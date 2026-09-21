#include "StudioLink.h"

#include <QDataStream>
#include <QSerialPort>
#include <QSerialPortInfo>

namespace {
quint32 crc32(const QByteArray& data) {
    quint32 crc = 0xffffffffu;
    for (unsigned char byte : data) {
        crc ^= byte;
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc >> 1u) ^ (0xedb88320u & (-(static_cast<qint32>(crc & 1u))));
    }
    return ~crc;
}
}

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
    QByteArray packet("FUI1", 4);
    packet.append(char(1));
    for (int shift = 0; shift < 4; ++shift) packet.append(char((revision >> (shift * 8)) & 0xff));
    const quint32 length = static_cast<quint32>(payload.size());
    for (int shift = 0; shift < 4; ++shift) packet.append(char((length >> (shift * 8)) & 0xff));
    const quint32 checksum = crc32(payload);
    for (int shift = 0; shift < 4; ++shift) packet.append(char((checksum >> (shift * 8)) & 0xff));
    packet.append(payload);
    return serial_->write(packet) == packet.size() && serial_->waitForBytesWritten(1000);
}
