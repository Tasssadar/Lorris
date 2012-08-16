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
#include "tcpserver.h"

namespace Ui {
    class LorrisProxy;
}

class QTcpSocket;

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
    void addConnection(QTcpSocket *connection, quint32 id);
    void removeConnection(quint32 id);
    void connectionMenu(const QPoint& pos);
    void tunnelNameEditFinished();
    void tunnelNameEdited(const QString& text);
    void tunnelToggled(bool enable);

private:
    Ui::LorrisProxy *ui;
    TcpServer m_server;
    ConnectButton * m_connectButton;
};

#endif // LORRISPROXY_H
