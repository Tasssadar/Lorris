#ifndef SHUPITOTUNNEL_H
#define SHUPITOTUNNEL_H

#include "connection.h"
#include "connectionmgr.h"

class Shupito;

class ShupitoTunnel : public Connection
{
    Q_OBJECT

public:
    ShupitoTunnel();
    ~ShupitoTunnel();

    bool Open();
    void OpenConcurrent();
    void Close();
    quint8 getType() { return CONNECTION_SERIAL_PORT; }
    void SendData(const QByteArray &data);

    void setShupito(Shupito* s);

private:
    Shupito *m_shupito;
    bool dataSigConnected;
};


#endif // SHUPITOTUNNEL_H
