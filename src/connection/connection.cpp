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
{
    opened = false;
}

Connection::~Connection()
{

}

bool Connection::Open()
{
    return false;
}

void Connection::SendData(const QByteArray& /*data*/)
{

}

void Connection::OpenConcurrent()
{
}

void Connection::removeFromMgr()
{
     sConMgr.RemoveCon(m_type, this);
}

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

