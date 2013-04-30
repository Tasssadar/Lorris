/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef UI_CONNECTBUTTON_H
#define UI_CONNECTBUTTON_H

#include <QObject>
#include <QMenu>
#include <QAction>
#include <QToolButton>
#include "../connection/connection.h"

class ConnectButton : public QObject
{
    Q_OBJECT

public:
    ConnectButton(QToolButton * btn);
    void setConn(ConnectionPointer<Connection> const & conn, bool emitConnChosen = true);
    void setConnectionTypes(PrimaryConnectionTypes type);

    QToolButton *btn() const { return m_btn; }

public slots:
    ConnectionPointer<Connection> choose();

Q_SIGNALS:
    void connectionChosen(ConnectionPointer<Connection> const & newConnection);

private slots:
    void connectTriggered();
    void connectionStateChanged(ConnectionState state);
    void connectionBeingDestroyed();

private:
    QMenu m_menu;
    QAction * m_connectAction;
    QAction * m_chooseAction;

    QToolButton * m_btn;
    ConnectionPointer<Connection> m_conn;
    PrimaryConnectionTypes m_connTypes;
};

#endif // UI_CONNECTBUTTON_H
