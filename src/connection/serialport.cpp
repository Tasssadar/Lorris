#include "serialport.h"
#include <QtConcurrentRun>
#include <QApplication>
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
        thread->terminate();
        thread->wait();
        delete thread;
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
}

void SerialPort::Close()
{
    if(thread)
    {
        thread->terminate();
        thread->wait();
        delete thread;
        thread = NULL;
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
    {
        thread->terminate();
        thread->wait();
        delete thread;
    }
    thread = new SerialPortThread(m_idString, m_rate);
    connect(thread, SIGNAL(dataRead(QByteArray)), this, SLOT(dataReadSer(QByteArray)));
    connect(thread, SIGNAL(connectResult(bool)), this, SLOT(connectResultSer(bool)));
    thread->start();
}
