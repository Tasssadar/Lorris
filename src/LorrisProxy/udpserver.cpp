/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "udpserver.h"

UdpServer::UdpServer(QObject *parent) : Server(parent)
{
    m_port = 0;
    m_isBound = false;
    connect(&m_socket, SIGNAL(readyRead()), SLOT(readyRead()));
}

UdpServer::~UdpServer()
{
    stopListening();
}

void UdpServer::SendData(const QByteArray& data)
{
    if(m_isBound)
        m_socket.writeDatagram(data, m_address, m_port);
}

bool UdpServer::listen(const QString& address, quint16 port)
{
    if(address == "0")
        m_address = QHostAddress::Any;
    else
        m_address = QHostAddress(address);

    m_port = port;
    m_isBound = m_socket.bind(port, QAbstractSocket::BindFlag::ShareAddress | QAbstractSocket::BindFlag::ReuseAddressHint);
    return m_isBound;
}

void UdpServer::stopListening()
{
    if(m_isBound) {
        m_isBound = false;
        m_socket.close();
    }
}

void UdpServer::readyRead()
{
    QByteArray data;
    QHostAddress addr;
    quint16 port;
    while(m_socket.hasPendingDatagrams())
    {
        data.resize(m_socket.pendingDatagramSize());
        if(m_socket.readDatagram(data.data(), data.size(), &addr, &port) > 0)
        {
            const QString addr_str = QString("%1:%2").arg(addr.toString()).arg(port);
            if(!m_clients.contains(addr_str)) {
                m_clients.insert(addr_str);
                emit newConnection(addr_str, m_con_counter++);
            }
            emit newData(data);
        }
    }
}

