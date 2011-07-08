#include "serialport.h"
#include <QtConcurrentRun>
#include "connectionmgr.h"

SerialPort::SerialPort() : Connection()
{
    m_port = NULL;
    m_type = CONNECTION_SERIAL_PORT;
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
    m_port->setDeviceName(m_idString);

    m_port->enableEmitStatus(true);
    if(m_port->open(AbstractSerial::ReadWrite | AbstractSerial::Unbuffered))
    {
        m_port->setBaudRate(m_rate, AbstractSerial::AllBaud);
        m_port->setParity(AbstractSerial::ParityNone);
        m_port->setDataBits(AbstractSerial::DataBits8);
        m_port->setStopBits(AbstractSerial::StopBits1);
        m_port->setFlowControl(AbstractSerial::FlowControlOff);
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

void SerialPort::SendData(QByteArray data)
{
    m_port->write(data);
}
