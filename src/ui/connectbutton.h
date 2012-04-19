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
    void setConn(Connection * conn);

public slots:
    Connection * choose();

Q_SIGNALS:
    void connectionChosen(Connection * newConnection);

private slots:
    void connectTriggered();
    void connectionStateChanged(ConnectionState state);
    void connectionDestroyed();

private:
    QMenu m_menu;
    QAction * m_connectAction;
    QAction * m_chooseAction;

    QToolButton * m_btn;
    Connection * m_conn;
};

#endif // UI_CONNECTBUTTON_H
