#include "serialportthread.h"

SerialPortThread::SerialPortThread(QextSerialPort *port, QObject *parent) :
    QThread(parent)
{
    m_port = port;
    m_run = true;
}

void SerialPortThread::run()
{
    while(m_run)
    {
        if(m_port->bytesAvailable())
            emit dataRead(m_port->readAll());
        msleep(50);
    }
}

void SerialPortThread::stop()
{
    m_run = false;
}
