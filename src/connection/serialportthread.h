#ifndef SERIALPORTTHREAD_H
#define SERIALPORTTHREAD_H

#include <QThread>
#include "connection.h"
#include "qserialdevice/abstractserial.h"

class SerialPortThread : public QThread
{
    Q_OBJECT

Q_SIGNALS:
    void connectResult(bool opened);
    void dataRead(QByteArray data);

public:
    SerialPortThread(QString name, AbstractSerial::BaudRate rate);
    ~SerialPortThread();

    void Stop();
    bool Open();

    void Send(QByteArray data);

    void run();

private slots:
    void viewStateSlot(QString stateMsg, QDateTime dt);

private:
    bool runTh;
    AbstractSerial *m_port;
    AbstractSerial::BaudRate m_rate;
    QString devName;
    bool opened;
};
#endif // SERIALPORTTHREAD_H
