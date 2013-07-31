/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SHUPITOSPITUNNELCONN_H
#define SHUPITOSPITUNNELCONN_H

#include "connection.h"

class Shupito;

class ShupitoSpiTunnelConn : public PortConnection
{
    Q_OBJECT

Q_SIGNALS:
    void writeSpiData(const QByteArray& data);

public:
    static QString getCompanionName();

    explicit ShupitoSpiTunnelConn();

    void setShupito(Shupito *shupito);
    bool hasShupito() const { return m_shupito; }
    void SendData(const QByteArray& data);
    void spiDataRead(const QByteArray& data);

    bool canSaveToSession() const { return m_shupito != NULL; }

public slots:
    void spiStateSwitchComplete(bool success);

protected:
    void doOpen();
    void doClose();

private:
    Shupito *m_shupito;
};

#endif // SHUPITOSPITUNNELCONN_H
