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
