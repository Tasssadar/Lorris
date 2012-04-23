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
#include <QLabel>
#include <QComboBox>
#include <QApplication>
#include <QPushButton>
#include <QStyle>
#include <qextserialenumerator.h>

#include "serialport.h"
#include "../config.h"
#include "../WorkTab/WorkTabMgr.h"
#include "../WorkTab/WorkTab.h"
#include "../WorkTab/WorkTabInfo.h"

SerialPort::SerialPort()
    : m_rate(BAUD38400),
      m_devNameEditable(true)
{
    m_type = CONNECTION_SERIAL_PORT;
    m_port = NULL;

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
    this->SetOpen(opened);
    emit connectResult(this, opened);
}

void SerialPort::Close()
{
    {
        QMutexLocker l(&m_port_mutex);

        if(m_port)
        {
            emit disconnecting();

            m_port->close();
            delete m_port;
            m_port = NULL;
        }
    }

    this->SetOpen(false);
}

void SerialPort::SendData(const QByteArray& data)
{
    if(this->isOpen())
    {
        qint64 len = m_port->write(data);
        // FIXME: Some serial ports needs this
        if(len == -1)
        {
            Utils::msleep(1);
            m_port->write(data);
        }
    }
}

void SerialPort::OpenConcurrent()
{
    if(this->state() != st_disconnected)
        return;

    this->SetState(st_connecting);

    m_future = QtConcurrent::run(this, &SerialPort::openPort);
    m_watcher.setFuture(m_future);
}

bool SerialPort::openPort()
{
    m_port_mutex.lock();

    m_port = new QextSerialPort(this->deviceName(), QextSerialPort::EventDriven);
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
        m_port->moveToThread(QApplication::instance()->thread());
        connect(m_port, SIGNAL(readyRead()), SLOT(readyRead()));
    }

    m_port_mutex.unlock();
    return res;
}

void SerialPort::readyRead()
{
    emit dataRead(m_port->readAll());
}

void SerialPort::openResult()
{
    connectResultSer(m_future.result());
}

void SerialPort::setDeviceName(QString const & value)
{
    if (m_deviceName != value)
    {
        m_deviceName = value;
        emit changed();
    }
}

QHash<QString, QVariant> SerialPort::config() const
{
    QHash<QString, QVariant> res = this->Connection::config();
    res["device_name"] = this->deviceName();
    res["baud_rate"] = (int)this->baudRate();
    return res;
}

bool SerialPort::applyConfig(QHash<QString, QVariant> const & config)
{
    this->setDeviceName(config.value("device_name").toString());
    this->setBaudRate((BaudRateType)config.value("baud_rate", 38400).toInt());
    return this->Connection::applyConfig(config);
}
