/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "connectbutton.h"
#include "chooseconnectiondlg.h"

ConnectButton::ConnectButton(QToolButton * btn)
    : QObject(btn), m_btn(btn), m_conn(0), m_connTypes(pct_port)
{
    m_connectAction = m_menu.addAction(tr("Connect"));
    m_menu.setDefaultAction(m_connectAction);

    m_chooseAction = m_menu.addAction(tr("Choose connection..."));

    btn->setMenu(&m_menu);
    btn->setDefaultAction(m_connectAction);

    connect(m_connectAction, SIGNAL(triggered()), this, SLOT(connectTriggered()));
    connect(m_chooseAction, SIGNAL(triggered()), this, SLOT(choose()));
}

void ConnectButton::connectTriggered()
{
    if (!m_conn || m_conn->state() == st_removed)
    {
        this->choose();
        if (m_conn && !m_conn->isOpen())
            m_conn->OpenConcurrent();
    }
    else
    {
        if (m_conn->isOpen())
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
    switch (state)
    {
    case st_disconnected:
    case st_removed:
        m_connectAction->setText(tr("Connect"));
        m_connectAction->setEnabled(true);
        break;
    case st_connecting:
        m_connectAction->setText(tr("Connecting..."));
        m_connectAction->setEnabled(false);
        break;
    case st_connected:
        m_connectAction->setText(tr("Disconnect"));
        m_connectAction->setEnabled(true);
        break;
    }
}

void ConnectButton::connectionBeingDestroyed()
{
    disconnect(m_conn.data(), 0, this, 0);
    m_conn.take();
    this->connectionStateChanged(st_disconnected);
    emit connectionChosen(m_conn);
}
