#pragma once

#include <QByteArray>
#include <QStringList>
#include <QString>

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

private:
    QSerialPort* serial_ = nullptr;
};
