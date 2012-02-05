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

#include "common.h"
#include "connectionmgr.h"
#include "tcpsocket.h"

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
    m_socket->connectToHost(m_address, m_port);

    m_future = QtConcurrent::run(this, &TcpSocket::connectToHost);
    m_watcher.setFuture(m_future);
}

bool TcpSocket::connectToHost()
{
    while(m_socket->state() == QAbstractSocket::HostLookupState)
    {
        Utils::msleep(50);
    }

    while(m_socket->state() == QAbstractSocket::ConnectingState)
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
