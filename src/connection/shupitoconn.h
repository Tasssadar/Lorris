#ifndef SHUPITOCONN_H
#define SHUPITOCONN_H

#include "connection.h"
#include "../LorrisShupito/shupitopacket.h"
#include "../LorrisShupito/shupitodesc.h"

class ShupitoConnection : public Connection
{
    Q_OBJECT

public:
    ShupitoConnection(ConnectionType type) : Connection(type) {}

    virtual void requestDesc() = 0;

public slots:
    virtual void sendPacket(ShupitoPacket const & packet) = 0;

signals:
    void packetRead(ShupitoPacket const & packet);
    void descRead(ShupitoDesc const & desc);
};

class PortShupitoConnection : public ShupitoConnection
{
    Q_OBJECT

public:
    PortShupitoConnection();

    ConnectionPointer<PortConnection> const & port() const { return m_port; }
    void setPort(ConnectionPointer<PortConnection> const & port);

    virtual void requestDesc();

public slots:
    void sendPacket(ShupitoPacket const & packet);

protected:
    ~PortShupitoConnection();
    void doOpen();
    void doClose();

private slots:
    void portStateChanged(ConnectionState state);
    void portDisconnecting();
    void portDestroyed();
    void portDataRead(QByteArray const & data);

private:
    void addPortTabRef();
    void releasePortTabRef();
    void handlePacket(ShupitoPacket const & packet);

    ConnectionPointer<PortConnection> m_port;
    bool m_holdsTabRef;

    enum { pst_init0, pst_init1, pst_init2, pst_discard, pst_cmd, pst_data } m_parserState;
    size_t m_parserLen;

    ShupitoPacket m_partialPacket;

    bool m_readDesc;
    QByteArray m_partialDesc;
};

#endif // SHUPITOCONN_H
