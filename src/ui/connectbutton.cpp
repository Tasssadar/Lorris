/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "connectbutton.h"
#include "chooseconnectiondlg.h"

ConnectButton::ConnectButton(QToolButton * btn)
    : QObject(btn), m_btn(btn), m_conn(0)
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

Connection * ConnectButton::choose()
{
    ChooseConnectionDlg dialog(m_conn, m_btn);
    if (!dialog.exec())
        return 0;

    this->setConn(dialog.current());
    return dialog.current();
}

void ConnectButton::setConn(Connection *conn)
{
    Connection * oldConn = m_conn;
    Connection * newConn = conn;
    m_conn = newConn;

    if (oldConn != newConn)
    {
        if (oldConn)
        {
            disconnect(oldConn, 0, this, 0);
        }

        if (newConn)
        {
            connect(newConn, SIGNAL(destroyed()), this, SLOT(connectionDestroyed()));
            connect(newConn, SIGNAL(stateChanged(ConnectionState)), this, SLOT(connectionStateChanged(ConnectionState)));
            this->connectionStateChanged(newConn->state());
        }
        else
        {
            this->connectionStateChanged(st_disconnected);
        }

        if (PortConnection * c = dynamic_cast<PortConnection *>(newConn))
            emit connectionChosen(c);
    }
}

void ConnectButton::connectionDestroyed()
{
    this->setConn(0);
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
