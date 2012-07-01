/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "connectbutton.h"
#include "chooseconnectiondlg.h"

ConnectButton::ConnectButton(QToolButton * btn)
    : QObject(btn), m_btn(btn), m_conn(0), m_connType(pct_port)
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
    if (!m_conn)
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

    ConnectionPointer<Connection> port;
    switch (m_connType)
    {
    case pct_port:
         port = dialog.choosePort(m_conn);
         break;
    case pct_shupito:
         port = dialog.chooseShupito(m_conn);
         break;
    }

    if (port)
        this->setConn(port);
    return port;
}

void ConnectButton::setConnectionType(PrimaryConnectionType type)
{
    m_connType = type;
}

void ConnectButton::setConn(ConnectionPointer<Connection> const & conn)
{
    if (m_conn != conn)
    {
        if (m_conn)
            disconnect(m_conn.data(), 0, this, 0);

        m_conn = conn;

        if (m_conn)
        {
            connect(m_conn.data(), SIGNAL(stateChanged(ConnectionState)), this, SLOT(connectionStateChanged(ConnectionState)));
            this->connectionStateChanged(m_conn->state());
        }
        else
        {
            this->connectionStateChanged(st_disconnected);
        }

        emit connectionChosen(m_conn);
    }
}

void ConnectButton::connectionStateChanged(ConnectionState state)
{
    switch (state)
    {
    case st_disconnected:
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
