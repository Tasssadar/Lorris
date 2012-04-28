/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

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
