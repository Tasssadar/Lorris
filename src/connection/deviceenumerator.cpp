#include "deviceenumerator.h"
#include "connectionmgr2.h"

void DeviceEnumeratorBase::registerConn(Connection * conn)
{
    sConMgr2.addConnection(conn);
}
