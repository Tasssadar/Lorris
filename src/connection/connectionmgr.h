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

#ifndef CONNECTIONMGR_H
#define CONNECTIONMGR_H

#include <QString>
#include <map>
#include <QDebug>

#include "singleton.h"
#include "connection.h"

enum ConnectionType
{
    CONNECTION_SOCKET      = 0,
    CONNECTION_SERIAL_PORT = 1,
    CONNECTION_FILE        = 2
};

#define MAX_CON_TYPE 3
#define CON_MSK(con) (1 << con)

class ConnectionMgr : public Singleton<ConnectionMgr>
{
public:
    ConnectionMgr();

    void AddCon(quint8 type, Connection *con)
    {
        conMap[type].insert(std::make_pair<QString, Connection*>(con->GetIDString(), con));
    }

    void RemoveCon(quint8 type, Connection *con);

    Connection* FindConnection(quint8 type, QString idString)
    {
        if(conMap[type].find(idString) != conMap[type].end())
            return conMap[type].find(idString)->second;
        else
            return NULL;
    }

private:
    std::map<QString, Connection*> conMap[MAX_CON_TYPE];
};

#define sConMgr ConnectionMgr::GetSingleton()

#endif // CONNECTIONMGR_H
