#include "serialport.h"
#include <QtConcurrentRun>

SerialPort::SerialPort() : Connection()
{
    m_port = NULL;
}

SerialPort::~SerialPort()
{
    if(m_port)
        delete m_port;
}

bool SerialPort::Open()
{
    m_port = new AbstractSerial();
    connect(m_port, SIGNAL(signalStatus(const QString, QDateTime)), this, SLOT(viewStateSlot(QString, QDateTime)));
    m_port->setDeviceName(m_device);

    m_port->enableEmitStatus(true);
    if(m_port->open(AbstractSerial::ReadWrite))
    {
        m_port->setBaudRate(m_rate, AbstractSerial::AllBaud);
        opened = true;
        connect(m_port, SIGNAL(readyRead()), this, SLOT(ReadFromPort()));
    }
    return opened;
}

void SerialPort::Close()
{
    m_port->close();
    opened = false;
}

void SerialPort::viewStateSlot(QString stateMsg, QDateTime dt)
{

}

void SerialPort::ReadFromPort()
{
    QByteArray ba = m_port->readAll();
    emit dataRead(ba);
}
