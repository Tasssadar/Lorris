/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QSignalMapper>

#include "tcpserver.h"

TcpServer::TcpServer(QObject *parent) : Server(parent)
{
    connect(&m_server,         SIGNAL(newConnection()), SLOT(onNewConnection()));
    connect(&m_disconnect_map, SIGNAL(mapped(int)),     SLOT(disconnected(int)));
    connect(&m_ready_map,      SIGNAL(mapped(int)),     SLOT(readyRead(int)));
}

TcpServer::~TcpServer()
{
    stopListening();
}

void TcpServer::onNewConnection()
{
    QTcpSocket *socket = m_server.nextPendingConnection();
    m_socket_map[m_con_counter] = socket;

    m_disconnect_map.setMapping(socket, m_con_counter);
    m_ready_map.setMapping(socket, m_con_counter);
    connect(socket, SIGNAL(disconnected()), &m_disconnect_map, SLOT(map()));
    connect(socket, SIGNAL(readyRead()),    &m_ready_map,      SLOT(map()));

    emit newConnection(socket->peerAddress().toString(), m_con_counter);

    ++m_con_counter;
}

void TcpServer::SendData(const QByteArray& data)
{
    if(!m_server.isListening())
        return;

    for(socketMap::iterator itr = m_socket_map.begin(); itr != m_socket_map.end(); ++itr)
        (*itr)->write(data);
}

void TcpServer::disconnected(int con)
{
    socketMap::iterator itr = m_socket_map.find((quint32)con);
    if(itr == m_socket_map.end())
        return;

    (*itr)->deleteLater();

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

    return m_server.listen(host_address, port);
}

void TcpServer::stopListening()
{
    // Socket is deleted in void TcpServer::disconnected(int con)
    socketMap tmpMap = m_socket_map;
    for(socketMap::iterator itr = tmpMap.begin(); itr != tmpMap.end(); ++itr)
        (*itr)->close();

    m_server.close();
}

void TcpServer::readyRead(int con)
{
    socketMap::iterator itr = m_socket_map.find((quint32)con);
    if(itr == m_socket_map.end())
        return;

    emit newData((*itr)->readAll());
}


void TcpServer::closeConnection(quint32 id)
{
    socketMap::iterator itr = m_socket_map.find(id);
    if(itr == m_socket_map.end())
        return;

    (*itr)->close();
}
