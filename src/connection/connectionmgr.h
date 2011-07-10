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
