/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef PROXYTUNNEL_H
#define PROXYTUNNEL_H

#include "connection.h"

class TcpServer;

class ProxyTunnel : public PortConnection
{
    Q_OBJECT
public:
    ProxyTunnel();

    void SendData(const QByteArray &data);

    void setTcpServer(TcpServer *server);

    QHash<QString, QVariant> config() const;
    bool canSaveToSession() const { return true; }

protected:
    ~ProxyTunnel();
    void doOpen();
    void doClose();

private:
    TcpServer *m_server;
};

#endif // PROXYTUNNEL_H
