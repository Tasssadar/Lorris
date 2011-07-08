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

    bool Open();
    void Close();
    void SendData(QByteArray data);
    void SetNameAndRate(QString name, AbstractSerial::BaudRate rate)
    {
        m_idString = name;
        m_rate = rate;
    }

private slots:
    void viewStateSlot(QString stateMsg, QDateTime dt);
    void ReadFromPort();

private:


    AbstractSerial *m_port;
    AbstractSerial::BaudRate m_rate;
};

#endif // SERIALPORT_H
