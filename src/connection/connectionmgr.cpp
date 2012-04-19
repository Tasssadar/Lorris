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

#include <QStringList>

#include "connectionmgr.h"
#include "connection.h"
#include "../LorrisShupito/shupito.h"
#include "shupitotunnel.h"
#include "serialport.h"
#include "tcpsocket.h"
#include "fileconnection.h"

ConnectionMgr::ConnectionMgr()
{
}

void ConnectionMgr::AddCon(quint8 type, Connection *con)
{
    conMap[type][con->GetIDString()] = con;
}

void ConnectionMgr::RemoveCon(quint8 type, Connection *con)
{
    conMap[type].erase(con->GetIDString());
}

Connection* ConnectionMgr::FindConnection(quint8 type, QString idString)
{
    con_map::iterator itr = conMap[type].find(idString);
    if(itr != conMap[type].end())
        return itr->second;
    return NULL;
}

void ConnectionMgr::AddShupito(QString id, Shupito *s)
{
    shupitoMap[id] = s;

    ShupitoTunnel *tunnel = (ShupitoTunnel*)sConMgr.FindConnection(CONNECTION_SHUPITO, id);
    if(tunnel)
    {
        tunnel->setShupito(s);
        tunnel->Open();
    }
}

void ConnectionMgr::RemoveShupito(QString id)
{
    shupitoMap.erase(id);

    ShupitoTunnel *tunnel = (ShupitoTunnel*)sConMgr.FindConnection(CONNECTION_SHUPITO, id);
    if(tunnel)
        tunnel->setShupito(NULL);
}

void ConnectionMgr::RemoveShupito(Shupito *shupito)
{
    for(shupito_map::iterator itr = shupitoMap.begin(); itr != shupitoMap.end(); ++itr)
    {
        if(itr->second == shupito)
        {
            RemoveShupito(itr->first);
            return;
        }
    }
}

Shupito *ConnectionMgr::GetShupito(QString id)
{
    shupito_map::iterator itr = shupitoMap.find(id);
    if(itr != shupitoMap.end())
        return itr->second;
    return NULL;
}

void ConnectionMgr::GetShupitoIds(QStringList &list)
{
    for(shupito_map::iterator itr = shupitoMap.begin(); itr != shupitoMap.end(); ++itr)
        list.append(itr->first);
}

bool ConnectionMgr::isAnyShupito()
{
    return !shupitoMap.empty();
}

ConnectionBuilder *ConnectionMgr::getConBuilder(quint8 conType, int moduleIdx, QWidget *parent)
{
    switch(conType)
    {
        case CONNECTION_SERIAL_PORT: return new SerialPortBuilder    (parent, moduleIdx);
        case CONNECTION_SHUPITO:     return new ShupitoTunnelBuilder (parent, moduleIdx);
        case CONNECTION_FILE:        return new FileConnectionBuilder(parent, moduleIdx);
        case CONNECTION_TCP_SOCKET:  return new TcpSocketBuilder     (parent, moduleIdx);
    }
    return NULL;
}
