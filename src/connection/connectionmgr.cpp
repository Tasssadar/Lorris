#include "connectionmgr.h"
#include "connection.h"

ConnectionMgr::ConnectionMgr()
{
}

void ConnectionMgr::RemoveCon(uint8_t type, Connection *con)
{
    QString idstring = con->GetIDString();
    qDebug() << "id: " << idstring;
    conMap[type].erase(idstring);
}


