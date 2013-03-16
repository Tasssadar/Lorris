#ifndef SHUPITOCONN_H
#define SHUPITOCONN_H

#include "connection.h"
#include "../LorrisProgrammer/shupitopacket.h"
#include "../LorrisProgrammer/shupitodesc.h"

#include <QString>
#include <QDateTime>
#include <stdint.h>

struct ShupitoFirmwareDetails
{
    uint8_t hw_major;
    uint8_t hw_minor;
    QDateTime fw_timestamp;
    int16_t fw_zone_offset;
    QString fw_revision;

    bool empty() const;
    QString firmwareFilename() const;
};

class ShupitoConnection : public Connection
{
    Q_OBJECT

public:
    ShupitoConnection(ConnectionType type);

    bool isNamePersistable() const;
    void persistName();

    virtual void requestDesc() = 0;
    virtual size_t maxPacketSize() const = 0;

    bool getFirmwareDetails(ShupitoFirmwareDetails & details) const;

public slots:
    virtual void sendPacket(ShupitoPacket const & packet) = 0;

signals:
    void packetRead(ShupitoPacket const & packet);
    void descRead(ShupitoDesc const & desc);

private slots:
    void connectionStateChanged(ConnectionState state);
    void descriptorChanged(ShupitoDesc const & desc);

private:
    void doPersist();

    ShupitoDesc::config const * m_renameConfig;
    ShupitoFirmwareDetails m_fwDetails;
    bool m_persistScheduled;
};

class PortShupitoConnection : public ShupitoConnection
{
    Q_OBJECT

public:
    PortShupitoConnection();

    ConnectionPointer<PortConnection> const & port() const { return m_port; }
    void setPort(ConnectionPointer<PortConnection> const & port);

    virtual void requestDesc();
    virtual size_t maxPacketSize() const { return 15; }

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
