#ifndef SERIALPORTTHREAD_H
#define SERIALPORTTHREAD_H

#include <QThread>
#include <QMutex>

#include "connection.h"
#include "qserialdevice/abstractserial.h"
#include "serialport.h"

class SerialPortThread : public QThread
{
    Q_OBJECT

Q_SIGNALS:
    void connectResult(bool opened);
    void dataRead(QByteArray data);

public:
    SerialPortThread(QString name, AbstractSerial::BaudRate rate, SerialPort *con);
    ~SerialPortThread();

    bool Open();
    void Send(QByteArray data);

    void run();

private slots:
    void viewStateSlot(QString stateMsg, QDateTime dt);
    void stop();

private:
    bool runTh;
    AbstractSerial *m_port;
    AbstractSerial::BaudRate m_rate;
    QString devName;
    bool opened;

    SerialPort *m_con;
    QMutex m_portMutex;
};
#endif // SERIALPORTTHREAD_H
