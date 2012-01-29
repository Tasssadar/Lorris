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

    void setShupito(Shupito* s, QString id);
    void SendData(const QByteArray &data);

    quint8 getType() { return CONNECTION_SERIAL_PORT; }

public slots:
    void tunnelStatus(bool opened);

private:
    Shupito *m_shupito;
    bool tunnelActive;
    bool dataSigConnected;
};


#endif // SHUPITOTUNNEL_H
