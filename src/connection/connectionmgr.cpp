#include "connectionmgr.h"
#include "connection.h"

ConnectionMgr::ConnectionMgr()
{
}

void ConnectionMgr::RemoveCon(quint8 type, Connection *con)
{
    conMap[type].erase(con->GetIDString());
}


