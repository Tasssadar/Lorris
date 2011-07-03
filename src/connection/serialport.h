#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QDateTime>
#include "connection.h"
#include "qserialdevice/abstractserial.h"

class SerialPort : public Connection
{
    Q_OBJECT
public:
    explicit SerialPort();
    virtual ~SerialPort();

    QString GetIDString() { return QString(""); }

    bool Open();
    void Close();
    void SetNameAndRate(QString name, AbstractSerial::BaudRate rate)
    {
        m_device = name;
        m_rate = rate;
    }

private slots:
    void viewStateSlot(QString stateMsg, QDateTime dt);
    void ReadFromPort();

private:


    AbstractSerial *m_port;
    AbstractSerial::BaudRate m_rate;
    QString m_device;
};

#endif // SERIALPORT_H
