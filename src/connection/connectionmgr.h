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
    CONNECTION_FILE        = 2,
    CONNECTION_SHUPITO     = 3, // Do not use in WorkTabs, when connected to shupito, it identifies
                                // as serial port from WorkTab's point of view
    MAX_CON_TYPE           = 4
};

#define CON_MSK(con) (1 << con)

class Shupito;

class ConnectionMgr : public Singleton<ConnectionMgr>
{
    typedef std::map<QString, Connection*> con_map;
    typedef std::map<QString, Shupito*> shupito_map;

public:
    ConnectionMgr();

    void AddCon(quint8 type, Connection *con);
    void RemoveCon(quint8 type, Connection *con);
    Connection* FindConnection(quint8 type, QString idString);

    void AddShupito(QString id, Shupito *s);
    void RemoveShupito(QString id);
    void RemoveShupito(Shupito *shupito);
    Shupito *GetShupito(QString id);
    void GetShupitoIds(QStringList& list);
    bool isAnyShupito();

private:
    con_map conMap[MAX_CON_TYPE];
    shupito_map shupitoMap;
};

#define sConMgr ConnectionMgr::GetSingleton()

#endif // CONNECTIONMGR_H
