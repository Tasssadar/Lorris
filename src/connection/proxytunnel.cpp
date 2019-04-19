/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "proxytunnel.h"
#include "../LorrisProxy/server.h"

ProxyTunnel::ProxyTunnel() : PortConnection(CONNECTION_PROXY_TUNNEL)
{
    m_server = NULL;
}

ProxyTunnel::~ProxyTunnel()
{
    Close();
}

void ProxyTunnel::doOpen()
{
    if(m_server)
    {
        connect(m_server, SIGNAL(newData(QByteArray)), SIGNAL(dataRead(QByteArray)));
        this->SetState(st_connected);
    }
}

void ProxyTunnel::doClose()
{
    disconnect(m_server, SIGNAL(newData(QByteArray)), this, SIGNAL(dataRead(QByteArray)));
    this->SetState(st_disconnected);
}

void ProxyTunnel::setServer(Server *server)
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
