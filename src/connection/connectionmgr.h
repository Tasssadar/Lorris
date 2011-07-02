#ifndef CONNECTIONMGR_H
#define CONNECTIONMGR_H

enum ConnectionType
{
    CONNECTION_SOCKET      = 0x01,
    CONNECTION_SERIAL_PORT = 0x02,
    CONNECTION_FILE        = 0x04,
};


class ConnectionMgr
{
public:
    ConnectionMgr();
};

#endif // CONNECTIONMGR_H
