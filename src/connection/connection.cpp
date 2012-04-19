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

#include "connection.h"
#include "connectionmgr.h"
#include "WorkTab/WorkTab.h"

Connection::Connection()
    : m_type(0), m_state(st_disconnected), m_refcount(1), m_removable(true)
{
}

Connection::~Connection()
{
    // Note that m_refcount need not be 0 here. We allow connections
    // to be destroyed explicitly and clients must listen to destroyed()
    // signal.

    sConMgr.RemoveCon(m_type, this);
}

void Connection::SetState(ConnectionState state)
{
    bool oldOpen = (m_state == st_connected);
    bool newOpen = (state == st_connected);

    if(state != m_state)
    {
        m_state = state;
        if (oldOpen != newOpen)
            emit connected(newOpen);
        emit stateChanged(state);
    }
}

void Connection::SetOpen(bool open)
{
    if (open)
        this->SetState(st_connected);
    else
        this->SetState(st_disconnected);
}

void Connection::addRef()
{
    ++m_refcount;
}

void Connection::release()
{
    if (--m_refcount == 0)
        delete this;
}

//----------------------------------------------------------------------------
ConnectionBuilder::ConnectionBuilder(QWidget *parent, int moduleIdx) : QObject((QObject*)parent)
{
    m_parent = parent;
    m_module_idx = moduleIdx;
    m_tab = NULL;
}

ConnectionBuilder::~ConnectionBuilder()
{
    delete m_tab;
}

void ConnectionBuilder::addOptToTabDialog(QGridLayout */*layout*/)
{

}

void ConnectionBuilder::CreateConnection(WorkTab */*info*/)
{
}

