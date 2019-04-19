/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef LORRISPROXY_H
#define LORRISPROXY_H

#include "../WorkTab/WorkTab.h"
#include "../ui/connectbutton.h"
#include "../connection/proxytunnel.h"
#include "tcpserver.h"

namespace Ui {
    class LorrisProxy;
}

class QTcpSocket;
class Server;

class LorrisProxy : public PortConnWorkTab
{
    Q_OBJECT
    
public:
    explicit LorrisProxy();
    ~LorrisProxy();

    void setPortConnection(ConnectionPointer<PortConnection> const & con);

    QString GetIdString();

    void onTabShow(const QString& filename);

    void saveData(DataFileParser *file);
    void loadData(DataFileParser *file);

private slots:
    void updateAddressText();
    void listenChanged();
    void addConnection(QString address, quint32 id);
    void removeConnection(quint32 id);
    void connectionMenu(const QPoint& pos);
    void tunnelNameEditFinished();
    void tunnelNameEdited(const QString& text);
    void tunnelToggled(bool enable);
    void protocolToggled(bool isTcp);

private:
    void createProxyTunnel(const QString& name);
    void destroyProxyTunnel();

    enum {
        PROTOCOL_TCP = 0,
        PROTOCOL_UDP = 1,
    };

    ConnectionPointer<ProxyTunnel> m_tunnel_conn;

    Ui::LorrisProxy *ui;
    Server *m_server;
    ConnectButton * m_connectButton;
    quint8 m_protocol;
};

#endif // LORRISPROXY_H
