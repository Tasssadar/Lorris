#include "connectionmgr.h"
#include "connection.h"

ConnectionMgr::ConnectionMgr()
{
}

void ConnectionMgr::RemoveCon(uint8_t type, Connection *con)
{
    conMap[type].erase(con->GetIDString());
}


