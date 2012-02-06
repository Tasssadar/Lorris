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
#include <qextserialenumerator.h>

#include "serialport.h"
#include "serialportthread.h"
#include "connectionmgr.h"
#include "config.h"
#include "WorkTab/WorkTabMgr.h"
#include "WorkTab/WorkTabInfo.h"

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

        emit connected(false);
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

void SerialPortBuilder::addOptToTabDialog(QGridLayout *layout)
{
    QLabel *portLabel = new QLabel(tr("Port: "), NULL, Qt::WindowFlags(0));
    m_portBox = new QComboBox(m_parent);
    m_portBox->setEditable(true);

    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    QStringList portNames;
    for (int i = 0; i < ports.size(); i++)
    {
#ifdef Q_OS_WIN
        QString name = ports.at(i).portName;
        name.replace(QRegExp("[^\\w]"), "");
        portNames.push_back(name);
#else
        portNames.push_back(ports.at(i).physName);
#endif
    }
    portNames.sort();
    m_portBox->addItems(portNames);

    layout->addWidget(portLabel, 1, 0);
    layout->addWidget(m_portBox, 1, 1);

    QLabel *rateLabel = new QLabel(tr("Baud Rate: "), m_parent);
    m_rateBox = new QComboBox(m_parent);

    m_rateBox->addItem("38400",   BAUD38400);
    m_rateBox->addItem("2400",    BAUD2400);
    m_rateBox->addItem("4800",    BAUD4800);
    m_rateBox->addItem("19200",   BAUD19200);
    m_rateBox->addItem("57600",   BAUD57600);
    m_rateBox->addItem("115200",  BAUD115200);

    layout->addWidget(rateLabel, 1, 2);
    layout->addWidget(m_rateBox, 1, 3);

    int baud = sConfig.get(CFG_QUINT32_SERIAL_BAUD);
    for(quint8 i = 0; i < m_rateBox->count(); ++i)
    {
        if(baud == m_rateBox->itemData(i).toInt())
        {
            m_rateBox->setCurrentIndex(i);
            break;
        }
    }

    QString port = sConfig.get(CFG_STRING_SERIAL_PORT);
    if(port.length() != 0)
        m_portBox->setEditText(port);

    port = sConfig.get(CFG_STRING_SHUPITO_PORT);
    if(m_module_idx >= 0 && m_module_idx < (int)sWorkTabMgr.GetWorkTabInfos()->size() && port.length() != 0)
    {
        WorkTabInfo *info = sWorkTabMgr.GetWorkTabInfos()->at(m_module_idx);
        if(info->GetName().contains("Shupito", Qt::CaseInsensitive))
            m_portBox->setEditText(port);
    }
}

void SerialPortBuilder::CreateConnection(WorkTabInfo *info)
{
    QString portName = m_portBox->currentText();
    BaudRateType rate = BaudRateType(m_rateBox->itemData(m_rateBox->currentIndex()).toInt());

    sConfig.set(CFG_STRING_SERIAL_PORT, portName);
    sConfig.set(CFG_QUINT32_SERIAL_BAUD, (quint32)rate);

    SerialPort *port = (SerialPort*)sConMgr.FindConnection(CONNECTION_SERIAL_PORT, portName);
    if(port && port->isOpen())
    {
        emit connectionSucces(port, info->GetName() + " - " + port->GetIDString(), info);
    }
    else
    {
        emit setCreateBtnStatus(true);

        m_tab_info = info;

        if(!port)
        {
            port = new SerialPort();
            port->SetNameAndRate(portName, rate);
        }

        connect(port, SIGNAL(connectResult(Connection*,bool)), SLOT(conResult(Connection*,bool)));
        port->OpenConcurrent();
    }
}

void SerialPortBuilder::conResult(Connection *con, bool open)
{
    if(open)
    {
        emit connectionSucces(con, m_tab_info->GetName() + " - " + con->GetIDString(), m_tab_info);
    }
    else
    {
        emit setCreateBtnStatus(false);
        emit connectionFailed(tr("Failed to open serial port!"));
        delete con;
    }
}
