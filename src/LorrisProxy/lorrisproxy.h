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

namespace Ui {
    class LorrisProxy;
}

class TcpServer;
class QTcpSocket;

class LorrisProxy : public PortConnWorkTab
{
    Q_OBJECT
    
public:
    explicit LorrisProxy();
    ~LorrisProxy();

    QString GetIdString();

    void setConnection(PortConnection *con);
    void onTabShow(const QString& filename);

private slots:
    void updateAddressText();
    void listenChanged();
    void addConnection(QTcpSocket *connection, quint32 id);
    void removeConnection(quint32 id);
    void connectionResult(Connection *con,bool result);

private:
    Ui::LorrisProxy *ui;
    TcpServer *m_server;
    ConnectButton * m_connectButton;
};

#endif // LORRISPROXY_H
