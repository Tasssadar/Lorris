#ifndef SHUPITOCONN_H
#define SHUPITOCONN_H

#include "connection.h"
#include "../LorrisShupito/shupitopacket.h"

class ShupitoConnection : public Connection
{
    Q_OBJECT

public:
    ShupitoConnection(ConnectionType type) : Connection(type) {}

public slots:
    virtual void sendPacket(ShupitoPacket const & packet) = 0;

signals:
    void packetRead(ShupitoPacket const & packet);
};

class PortShupitoConnection : public ShupitoConnection
{
    Q_OBJECT

public:
    PortShupitoConnection();

    void OpenConcurrent();
    void Close();

    ConnectionPointer<PortConnection> const & port() const { return m_port; }
    void setPort(ConnectionPointer<PortConnection> const & port);

public slots:
    void sendPacket(ShupitoPacket const & packet);

protected:
    ~PortShupitoConnection();

private slots:
    void portStateChanged(ConnectionState state);
    void portDisconnecting();
    void portDestroyed();
    void portDataRead(QByteArray const & data);

private:
    ConnectionPointer<PortConnection> m_port;

    enum { pst_init0, pst_init1, pst_init2, pst_discard, pst_cmd, pst_data } m_parserState;
    size_t m_parserLen;

    ShupitoPacket m_partialPacket;
};

#endif // SHUPITOCONN_H
