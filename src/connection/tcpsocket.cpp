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

#include <QtNetwork/QTcpSocket>
#include <QtCore/QtConcurrentRun>
#include <QtCore/QThreadPool>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>

#include "common.h"
#include "connectionmgr.h"
#include "tcpsocket.h"
#include "WorkTab/WorkTabInfo.h"
#include "WorkTab/WorkTab.h"

static const int CONNECT_TIMEOUT = 10000 / 50; // 10s

TcpSocket::TcpSocket() : Connection()
{
    m_type = CONNECTION_TCP_SOCKET;

    m_socket = new QTcpSocket(this);

    connect(m_socket,   SIGNAL(readyRead()),                                SLOT(readyRead()));
    connect(m_socket,   SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(stateChanged()));
    connect(&m_watcher, SIGNAL(finished()),                                 SLOT(tcpConnectResult()));
}

TcpSocket::~TcpSocket()
{
    Close();
    delete m_socket;
}

bool TcpSocket::Open()
{
    return false;
}

void TcpSocket::Close()
{
    if(m_future.isRunning())
    {
        m_future.cancel();
        m_future.waitForFinished();
    }

    m_socket->close();

    emit connected(false);
    opened = false;
}

void TcpSocket::connectResultSer(bool opened)
{
    this->opened = opened;
    emit connectResult(this, opened);

    if(opened)
        emit connected(true);
}

void TcpSocket::OpenConcurrent()
{
    if(opened)
        return;

    m_socket->connectToHost(m_address, m_port);

    m_future = QtConcurrent::run(this, &TcpSocket::connectToHost);
    m_watcher.setFuture(m_future);
}

bool TcpSocket::connectToHost()
{
    int time = 0;
    for(; !m_future.isCanceled() && time < CONNECT_TIMEOUT && m_socket->state() == QAbstractSocket::HostLookupState; ++time)
    {
        Utils::msleep(50);
    }

    for(; !m_future.isCanceled() && time < CONNECT_TIMEOUT && m_socket->state() == QAbstractSocket::ConnectingState; ++time)
    {
        Utils::msleep(50);
    }

    return (m_socket->state() == QAbstractSocket::ConnectedState);
}

void TcpSocket::tcpConnectResult()
{
    connectResultSer(m_future.result());
}

void TcpSocket::SendData(const QByteArray &data)
{
    m_socket->write(data);
}

void TcpSocket::readyRead()
{
    QByteArray data = m_socket->readAll();
    emit dataRead(data);
}

void TcpSocket::stateChanged()
{
    if(opened && m_socket->state() != QAbstractSocket::ConnectedState)
        Close();
}

void TcpSocketBuilder::addOptToTabDialog(QGridLayout *layout)
{
    QLabel    *addressLabel = new QLabel(tr("Address:"), m_parent);
    m_address               = new QLineEdit(m_parent);
    QLabel    *portLabel    = new QLabel(tr("Port:"), m_parent);
    m_port                  = new QSpinBox(m_parent);

    m_port->setMaximum(0xFFFF);
    m_port->setValue(sConfig.get(CFG_QUINT32_TCP_PORT));

    m_address->setText(sConfig.get(CFG_STRING_TCP_ADDR));

    layout->addWidget(addressLabel, 1, 0);
    layout->addWidget(m_address, 1, 1);
    layout->addWidget(portLabel, 1, 2);
    layout->addWidget(m_port, 1, 3);
}

void TcpSocketBuilder::CreateConnection(WorkTab *tab)
{
    QString address = m_address->text();
    quint16 port = m_port->value();

    sConfig.set(CFG_QUINT32_TCP_PORT, port);
    sConfig.set(CFG_STRING_TCP_ADDR, address);

    TcpSocket *socket =
            (TcpSocket*)sConMgr.FindConnection(CONNECTION_TCP_SOCKET, address + ":" + QString::number(port));
    if(!socket || !socket->isOpen())
    {
        emit setCreateBtnStatus(true);

        m_tab = tab;

        if(!socket)
        {
            socket = new TcpSocket();
            socket->setAddress(address, port);
        }
        m_tab->setConnection(socket);

        connect(socket, SIGNAL(connectResult(Connection*,bool)), SLOT(conResult(Connection*,bool)));
        socket->OpenConcurrent();
    }
    else
    {
        tab->setConnection(socket);
        emit connectionSucces(socket, tab->getInfo()->GetName() + " - " + socket->GetIDString(), tab);
    }
}

void TcpSocketBuilder::conResult(Connection *con, bool open)
{
    if(open)
    {
        emit connectionSucces(con, m_tab->getInfo()->GetName() + " - " + con->GetIDString(), m_tab);
        m_tab = NULL;
    }
    else
    {
        // Connection is deleted in WorkTab::~WorkTab()
        delete m_tab;
        m_tab = NULL;

        emit setCreateBtnStatus(false);
        emit connectionFailed(tr("Error opening TCP socket!"));
    }
}
