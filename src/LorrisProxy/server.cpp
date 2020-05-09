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

#include "server.h"

Server::Server(QObject *parent) : QObject(parent)
{
    m_con_counter = 0;
}

Server::~Server()
{
}

void Server::closeConnection(quint32 /*id*/)
{

}

QString Server::getAddress()
{
    if(serverAddress() == QHostAddress::Any)
    {
        QList<QHostAddress> addr = QNetworkInterface::allAddresses();

        QString name = "127.0.0.1";
        for(qint32 i = 0; i < addr.size(); ++i)
        {
            if(addr[i] != QHostAddress::LocalHost && addr[i] != QHostAddress::LocalHostIPv6)
            {
                name = addr[i].toString();
                break;
            }
        }
        return name;
    }
    else
        return serverAddress().toString();
}
