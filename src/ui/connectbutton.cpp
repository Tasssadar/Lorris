/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "connectbutton.h"
#include "chooseconnectiondlg.h"
#include "../connection/shupitoconn.h"

ConnectButton::ConnectButton(QToolButton * btn)
    : QObject(btn), m_btn(btn), m_conn(0), m_connTypes(pct_port_data)
{
    m_connectAction = m_menu.addAction(tr("Connect"));
    m_menu.setDefaultAction(m_connectAction);

    m_chooseAction = m_menu.addAction(tr("Choose connection..."));

    btn->setMenu(&m_menu);
    btn->setDefaultAction(m_connectAction);

    connect(m_connectAction, SIGNAL(triggered()), this, SLOT(connectTriggered()));
    connect(m_chooseAction, SIGNAL(triggered()), this, SLOT(choose()));
}

#ifdef Q_OS_MAC
ConnectButton:: ConnectButton(QToolButton * btn, QMacToolBarItem * connectBtn, QMacToolBarItem * chooseConnectionBtn)
    : QObject(btn),m_btn(btn), m_connect(connectBtn), m_chooseConnection(chooseConnectionBtn), m_conn(0), m_connTypes(pct_port_data)
{
    m_connectAction = m_menu.addAction(tr("Connect"));
    m_menu.setDefaultAction(m_connectAction);

    m_chooseAction = m_menu.addAction(tr("Choose connection..."));

    connect(connectBtn, SIGNAL(activated()), this, SLOT(connectTriggered()));
    connect(chooseConnectionBtn, SIGNAL(activated()), this, SLOT(choose()));
}
#endif

void ConnectButton::connectTriggered()
{
    if (!m_conn || m_conn->isMissing())
    {
        this->choose();
        if (m_conn && !m_conn->isOpen())
            m_conn->OpenConcurrent();
    }
    else
    {
        if (m_conn->state() == st_connected || m_conn->state() == st_disconnecting)
            m_conn->Close();
        else
            m_conn->OpenConcurrent();
    }
}

ConnectionPointer<Connection> ConnectButton::choose()
{
    ChooseConnectionDlg dialog(m_btn);
    ConnectionPointer<Connection> port = dialog.choose(m_connTypes, m_conn);
    if (port)
        this->setConn(port);
    return port;
}

void ConnectButton::setConnectionTypes(PrimaryConnectionTypes type)
{
    m_connTypes = type;
}

void ConnectButton::setConn(ConnectionPointer<Connection> const & conn, bool emitConnChosen)
{
    if (m_conn != conn)
    {
        if (m_conn)
            disconnect(m_conn.data(), 0, this, 0);

        m_conn = conn;

        if (m_conn)
        {
            connect(m_conn.data(), SIGNAL(stateChanged(ConnectionState)), this, SLOT(connectionStateChanged(ConnectionState)));
            connect(m_conn.data(), SIGNAL(destroying()), this, SLOT(connectionBeingDestroyed()));
            this->connectionStateChanged(m_conn->state());
        }
        else
        {
            this->connectionStateChanged(st_disconnected);
        }

        if(emitConnChosen)
            emit connectionChosen(m_conn);
    }
}

void ConnectButton::connectionStateChanged(ConnectionState state)
{
    QString text;
    bool interact;
    switch (state)
    {
    case st_disconnected:
    case st_missing:
    case st_connect_pending:
        text = tr("Connect");
        interact = true;
        break;
    case st_connecting:
        text = tr("Connecting...");
        interact = false;
        break;
    case st_connected:
        text = tr("Disconnect");
        interact = true;
        break;
    case st_disconnecting:
        text = tr("Disconnecting..");
        interact = true;
        break;
    }

#ifndef Q_OS_MAC
    m_connectAction->setText(text);
    m_connectAction->setEnabled(interact);
#else
    m_connect->setText(text);
    m_connect->setSelectable(interact);
#endif
}

void ConnectButton::connectionBeingDestroyed()
{
    disconnect(m_conn.data(), 0, this, 0);
    m_conn.take();
    this->connectionStateChanged(st_disconnected);
    emit connectionChosen(m_conn);
}
