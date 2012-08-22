/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "proxytunnel.h"
#include "../LorrisProxy/tcpserver.h"

ProxyTunnel::ProxyTunnel() : PortConnection(CONNECTION_PROXY_TUNNEL)
{
    m_server = NULL;
}

ProxyTunnel::~ProxyTunnel()
{
    Close();
}

bool ProxyTunnel::Open()
{
    if(!m_server)
        return false;

    connect(m_server, SIGNAL(newData(QByteArray)), SIGNAL(dataRead(QByteArray)));

    SetOpen(true);

    return true;
}

void ProxyTunnel::OpenConcurrent()
{
    emit connectResult(this, Open());
}

void ProxyTunnel::Close()
{
    if(!isOpen())
        return;

    disconnect(m_server, SIGNAL(newData(QByteArray)), this, SIGNAL(dataRead(QByteArray)));

    SetOpen(false);
}

void ProxyTunnel::setTcpServer(TcpServer *server)
{
    if(isOpen())
        Close();

    m_server = server;
}

void ProxyTunnel::SendData(const QByteArray &data)
{
    if(isOpen())
        m_server->SendData(data);
}

QHash<QString, QVariant> ProxyTunnel::config() const
{
    QHash<QString, QVariant> cfg;
    cfg["name"] = this->name();
    return cfg;
}
