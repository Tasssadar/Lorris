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

void SerialPort::dataReadSer(const QByteArray& data)
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
        thread->wait();
        delete thread;
        thread = NULL;
        emit connected(false);
    }
    opened = false;
}

void SerialPort::SendData(const QByteArray& data)
{
    if(opened)
        thread->Send(data);
}

void SerialPort::OpenConcurrent()
{
    if(thread)
    {
        emit stopThread();
        thread->wait();
        delete thread;
    }
    thread = new SerialPortThread(m_idString, m_rate, this);
    connect(thread, SIGNAL(dataRead(QByteArray)), this, SLOT(dataReadSer(QByteArray)));
    connect(thread, SIGNAL(connectResult(bool)), this, SLOT(connectResultSer(bool)));
    thread->start();
}
