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
#include <QStringBuilder>

#include "../common.h"
#include "tcpsocket.h"
#include "../WorkTab/WorkTabInfo.h"
#include "../WorkTab/WorkTab.h"

static const int CONNECT_TIMEOUT = 10000 / 50; // 10s

TcpSocket::TcpSocket()
    : PortConnection(CONNECTION_TCP_SOCKET)
{
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

QString TcpSocket::details() const
{
    return this->host() % ":" % QString::number(this->port());
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

    this->SetOpen(false);
}

void TcpSocket::connectResultSer(bool opened)
{
    this->SetOpen(opened);
    emit connectResult(this, opened);
}

void TcpSocket::OpenConcurrent()
{
    if(this->isOpen())
        return;

    this->SetState(st_connecting);
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
    if(this->isOpen() && m_socket->state() != QAbstractSocket::ConnectedState)
        Close();
}

void TcpSocket::setHost(QString const & value)
{
    if (value != m_address)
    {
        m_address = value;
        emit changed();
    }
}

void TcpSocket::setPort(quint16 value)
{
    if (value != m_port)
    {
        m_port = value;
        emit changed();
    }
}

QHash<QString, QVariant> TcpSocket::config() const
{
    QHash<QString, QVariant> res = this->Connection::config();
    res["host"] = this->host();
    res["port"] = this->port();
    return res;
}

bool TcpSocket::applyConfig(QHash<QString, QVariant> const & config)
{
    this->setHost(config.value("host").toString());
    this->setPort(config.value("port", 80).toInt());
    return this->Connection::applyConfig(config);
}
