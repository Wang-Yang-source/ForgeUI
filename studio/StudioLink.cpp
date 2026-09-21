#include "StudioLink.h"

#include <QSerialPort>
#include <QSerialPortInfo>

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
