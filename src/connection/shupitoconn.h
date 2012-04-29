#ifndef SHUPITOCONN_H
#define SHUPITOCONN_H

#include "connection.h"
#include "../LorrisShupito/shupitopacket.h"

class ShupitoConnection : public Connection
{
    Q_OBJECT

public:
    ShupitoConnection();

    void OpenConcurrent();
    void Close();

    ConnectionPointer<PortConnection> const & port() const { return m_port; }
    void setPort(ConnectionPointer<PortConnection> const & port);

Q_SIGNALS:
    void packetRead(ShupitoPacket const & packet);

public slots:
    void sendPacket(ShupitoPacket const & packet);

protected:
    ~ShupitoConnection();

private slots:
    void portStateChanged(ConnectionState state);
    void portDisconnecting();
    void portDestroyed();
    void portDataRead(QByteArray const & data);

private:
    ConnectionPointer<PortConnection> m_port;

    enum { pst_discard, pst_cmd, pst_data } m_parserState;
    size_t m_parserLen;

    ShupitoPacket m_partialPacket;
};

#endif // SHUPITOCONN_H
