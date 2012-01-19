#ifndef SHUPITOTUNNEL_H
#define SHUPITOTUNNEL_H

#include "connection.h"

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

public slots:
    void tunnelStatus(bool opened);

private:
    Shupito *m_shupito;
    bool tunnelActive;
};


#endif // SHUPITOTUNNEL_H
