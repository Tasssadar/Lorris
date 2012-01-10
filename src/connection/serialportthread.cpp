/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "serialportthread.h"

SerialPortThread::SerialPortThread(QString name, AbstractSerial::BaudRate rate, SerialPort *con) : QThread(0)
{
    m_port = NULL;
    runTh = true;
    devName = name;
    m_rate = rate;
    m_con = con;

    m_writeErrorCount = 0;
}

SerialPortThread::~SerialPortThread()
{

}

void SerialPortThread::Send(const QByteArray& data)
{
    if(runTh && m_port)
    {
        m_portMutex.lock();
        m_port->write(data);
        m_portMutex.unlock();
    }
}

bool SerialPortThread::Open()
{
    m_port = new AbstractSerial();
    connect(m_port, SIGNAL(signalStatus(const QString, QDateTime)), this, SLOT(viewStateSlot(QString, QDateTime)), Qt::QueuedConnection);

    m_port->setDeviceName(devName);
    m_port->enableEmitStatus(true);

    if(m_port->open(AbstractSerial::ReadWrite | AbstractSerial::Unbuffered))
    {
        m_port->setBaudRate(m_rate, AbstractSerial::AllBaud);
        m_port->setParity(AbstractSerial::ParityNone);
        m_port->setDataBits(AbstractSerial::DataBits8);
        m_port->setStopBits(AbstractSerial::StopBits1);
        m_port->setFlowControl(AbstractSerial::FlowControlOff);
        opened = true;
    }
    else
    {
        opened = false;
        delete m_port;
        m_port = NULL;
    }
    return opened;
}

void SerialPortThread::run()
{
    emit connectResult(Open());

    if(!opened)
        return;

    connect(m_con, SIGNAL(stopThread()), this, SLOT(stop()));

    while(runTh)
    {
        m_portMutex.lock();
        if(m_port->bytesAvailable() > 0)
        {
            QByteArray ba = m_port->readAll();
            emit dataRead(ba);
        }
        m_portMutex.unlock();
        msleep(10);
    }
    m_port->close();
    delete m_port;
    m_port = NULL;
}

void SerialPortThread::stop()
{
    runTh = false;
}

void SerialPortThread::viewStateSlot(QString /*stateMsg*/, QDateTime /*dt*/)
{
   /* if(stateMsg == "Controls::Write data to device - i/o problem. Error!")
    {
        ++m_writeErrorCount;
        if(m_writeErrorCount >= 10)
        {
            emit connectResult(false);
            opened = false;
            runTh = false;
        }
    }*/
}
