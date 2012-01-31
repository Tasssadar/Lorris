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

#include <QtCore/QtConcurrentRun>
#include <QtCore/QThreadPool>
#include <qextserialport.h>

#include "serialport.h"
#include "serialportthread.h"
#include "connectionmgr.h"

SerialPort::SerialPort() : Connection()
{
    m_type = CONNECTION_SERIAL_PORT;
    m_port = NULL;

    m_thread = NULL;

    connect(&m_watcher, SIGNAL(finished()), SLOT(openResult()));
}

SerialPort::~SerialPort()
{
    Close();
}

bool SerialPort::Open()
{
    return false;
}

void SerialPort::connectResultSer(bool opened)
{
    this->opened = opened;
    emit connectResult(this, opened);

    if(opened)
        emit connected(true);
}

void SerialPort::Close()
{
    if(m_port)
    {
        m_thread->stop();
        m_thread->wait();
        delete m_thread;
        m_thread = NULL;

        m_port->close();
        delete m_port;
        m_port = NULL;
    }
    opened = false;
}

void SerialPort::SendData(const QByteArray& data)
{
    if(opened)
        m_port->write(data);
}

void SerialPort::OpenConcurrent()
{
    m_future = QtConcurrent::run(this, &SerialPort::openPort);
    m_watcher.setFuture(m_future);
}

bool SerialPort::openPort()
{
    m_port = new QextSerialPort(m_idString, QextSerialPort::EventDriven);
    m_port->setBaudRate(m_rate);
    m_port->setParity(PAR_NONE);
    m_port->setDataBits(DATA_8);
    m_port->setStopBits(STOP_1);
    m_port->setFlowControl(FLOW_OFF);
    m_port->setTimeout(-1);

    bool res = m_port->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
    if(!res)
    {
        delete m_port;
        m_port = NULL;
    }
    else
    {
        m_thread = new SerialPortThread(m_port);
        connect(m_thread, SIGNAL(dataRead(QByteArray)), this, SIGNAL(dataRead(QByteArray)));
        m_thread->start();
    }

    return res;
}

void SerialPort::openResult()
{
    connectResultSer(m_future.result());
}
