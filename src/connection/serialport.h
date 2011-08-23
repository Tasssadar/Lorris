#ifndef SERIALPORT_H
#define SERIALPORT_H

#include "connection.h"
#include "qserialdevice/abstractserial.h"
#include "qserialdeviceenumerator/serialdeviceenumerator.h"

class SerialPortThread;

class SerialPort : public Connection
{
    Q_OBJECT

Q_SIGNALS:
    void stopThread();

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
    void OpenConcurrent();

private slots:
    void dataReadSer(QByteArray data);
    void connectResultSer(bool opened);

private:
    AbstractSerial::BaudRate m_rate;
    SerialPortThread *thread;
};



#endif // SERIALPORT_H
