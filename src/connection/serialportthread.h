#ifndef SERIALPORTTHREAD_H
#define SERIALPORTTHREAD_H

#include <QThread>

#include "serialport.h"

class SerialPortThread : public QThread
{
    Q_OBJECT

Q_SIGNALS:
    void dataRead(const QByteArray& data);

public:
    SerialPortThread(QextSerialPort *port, QObject *parent = 0);

    void stop();
    
protected:
    void run();

private:
    QextSerialPort *m_port;
    volatile bool m_run;
};

#endif // SERIALPORTTHREAD_H
