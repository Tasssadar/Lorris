#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QDateTime>
#include "connection.h"
#include "qserialdevice/abstractserial.h"

class SerialPortThread;

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
    void OpenConcurrent();
   // void concurrentFinished();

private slots:
    void dataReadSer(QByteArray data);
    void connectResultSer(bool opened);

private:
    AbstractSerial::BaudRate m_rate;
    SerialPortThread *thread;
};



#endif // SERIALPORT_H
