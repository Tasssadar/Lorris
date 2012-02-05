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

#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QSignalMapper>

#include "tcpserver.h"

TcpServer::TcpServer() : QObject()
{
    m_server = new QTcpServer(this);
    m_disconnect_map = new QSignalMapper(this);
    m_ready_map = new QSignalMapper(this);

    connect(m_server,         SIGNAL(newConnection()), SLOT(newConnection()));
    connect(m_disconnect_map, SIGNAL(mapped(int)),     SLOT(disconnected(int)));
    connect(m_ready_map,      SIGNAL(mapped(int)),     SLOT(readyRead(int)));

    m_con_counter = 0;
}

TcpServer::~TcpServer()
{
    stopListening();

    delete m_server;
    delete m_disconnect_map;
    delete m_ready_map;
}

void TcpServer::newConnection()
{
    QTcpSocket *socket = m_server->nextPendingConnection();
    m_socket_map[m_con_counter] = socket;

    m_disconnect_map->setMapping(socket, m_con_counter);
    m_ready_map->setMapping(socket, m_con_counter);
    connect(socket, SIGNAL(disconnected()), m_disconnect_map, SLOT(map()));
    connect(socket, SIGNAL(readyRead()),    m_ready_map,      SLOT(map()));

    emit newConnection(socket, m_con_counter);

    ++m_con_counter;
}

void TcpServer::SendData(const QByteArray& data)
{
    if(!m_server->isListening())
        return;

    for(socketMap::iterator itr = m_socket_map.begin(); itr != m_socket_map.end(); ++itr)
        itr->second->write(data);
}

void TcpServer::disconnected(int con)
{
    socketMap::iterator itr = m_socket_map.find((quint32)con);
    if(itr == m_socket_map.end())
        return;

    itr->second->deleteLater();

    m_socket_map.erase(itr);

    emit removeConnection(con);
}

bool TcpServer::listen(const QString& address, quint16 port)
{
    QHostAddress host_address;
    if(address == "0")
        host_address = QHostAddress::Any;
    else
        host_address = QHostAddress(address);

    return m_server->listen(host_address, port);
}

void TcpServer::stopListening()
{
    // Socket is deleted in void TcpServer::disconnected(int con)
    socketMap tmpMap = m_socket_map;
    for(socketMap::iterator itr = tmpMap.begin(); itr != tmpMap.end(); ++itr)
        itr->second->close();

    m_server->close();
}

void TcpServer::readyRead(int con)
{
    socketMap::iterator itr = m_socket_map.find((quint32)con);
    if(itr == m_socket_map.end())
        return;

    emit newData(itr->second->readAll());
}

QString TcpServer::getLastErr()
{
    return m_server->errorString();
}

bool TcpServer::isListening()
{
    return m_server->isListening();
}

QString TcpServer::getAddress()
{
    if(m_server->serverAddress() == QHostAddress::Any)
    {
        QList<QHostAddress> addr = QNetworkInterface::allAddresses();
        QString name;
        for(qint32 i = 0; name.isEmpty() && i < addr.size(); ++i)
        {
            QString cur = addr[i].toString();
            if(cur.contains("192.168"))
                name = cur;
        }
        if(name.isEmpty())
            name = "127.0.0.1";
        return name;
    }
    else
        return m_server->serverAddress().toString();
}

