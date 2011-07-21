#include "serialport.h"
#include "connectionmgr.h"
#include "serialportthread.h"

SerialPort::SerialPort() : Connection()
{
    m_type = CONNECTION_SERIAL_PORT;
    thread = NULL;
}

SerialPort::~SerialPort()
{
    if(thread)
    {
        emit stopThread();
        thread = NULL;
    }
}

bool SerialPort::Open()
{
    return false;
}

void SerialPort::dataReadSer(QByteArray data)
{
    emit dataRead(data);
}

void SerialPort::connectResultSer(bool opened)
{
    this->opened = opened;
    emit connectResult(this, opened);

    if(!opened)
        thread = NULL;
    else
        emit connected(true);
}

void SerialPort::Close()
{
    if(thread)
    {
        emit stopThread();
        thread = NULL;
        emit connected(false);
    }
    opened = false;
}

void SerialPort::SendData(QByteArray data)
{
    if(opened)
        thread->Send(data);
}

void SerialPort::OpenConcurrent()
{
    if(thread)
        emit stopThread();
    thread = new SerialPortThread(m_idString, m_rate, this);
    connect(thread, SIGNAL(dataRead(QByteArray)), this, SLOT(dataReadSer(QByteArray)));
    connect(thread, SIGNAL(connectResult(bool)), this, SLOT(connectResultSer(bool)));
    thread->start();
}
