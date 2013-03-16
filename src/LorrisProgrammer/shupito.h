/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SHUPITO_H
#define SHUPITO_H

#include <QObject>
#include <QByteArray>
#include <QMutex>
#include <QTimer>

#include "../shared/programmer.h"
#include "../shared/chipdefs.h"
#include "shupitodesc.h"
#include "../connection/shupitoconn.h"

enum Opcodes
{
    MSG_INFO       = 0x00,
    MSG_VCC        = 0x0A,
    MSG_TUNNEL     = 0x09
};

enum Modes
{
    MODE_SPI = 0,
    MODE_PDI,
    //MODE_JTAG,
    MODE_CC25XX,
    MODE_SPIFLASH,

    MODE_COUNT
};

enum WaitTypes
{
    WAIT_NONE = 0,
    WAIT_PACKET,
    WAIT_STREAM
};

class ShupitoTunnel;

class Shupito : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void descRead(bool correct);
    void responseReceived(char error_code);
    void vccValueChanged(quint8 id, double value);
    void vddDesc(const vdd_setup& vs);
    void tunnelData(const QByteArray& data);
    void packetReveived();
    void tunnelStatus(bool);

public:
    explicit Shupito(QObject *parent);
    ~Shupito();
    void init(ShupitoConnection *con, ShupitoDesc *desc);

    void readPacket(const ShupitoPacket& data);
    void sendPacket(const ShupitoPacket& data);
    ShupitoPacket waitForPacket(ShupitoPacket const & pkt, quint8 cmd);
    QByteArray waitForStream(ShupitoPacket const & pkt, quint8 cmd, quint16 max_packets = 1024);

    void setVddConfig(ShupitoDesc::config const *cfg) { m_vdd_config = cfg; }
    void setTunnelConfig(ShupitoDesc::config const *cfg);

    void setTunnelSpeed(quint32 speed, bool send = true);
    quint32 getTunnelSpeed() const { return m_tunnel_speed; }
    qint16 getTunnelCmd();
    quint8 getTunnelId() { return m_tunnel_pipe; }
    void setTunnelState(bool enable, bool wait = false);
    void setTunnelPipe(quint8 pipe) { m_tunnel_pipe = pipe; }

    ShupitoDesc *getDesc() { return m_desc; }
    void setChipId(chip_definition cd) { m_chip_def = cd; }
    const chip_definition& getChipId() { return m_chip_def; }

    size_t maxPacketSize() const { return m_con->maxPacketSize(); }

public slots:
    void sendTunnelData(const QString& data)
    {
        sendTunnelData(data.toUtf8());
    }

    void sendTunnelData(const QByteArray& data);

private slots:
    void tunnelDataSend();
    void descReceived(ShupitoDesc const & desc);

private:
    void handleVccPacket(ShupitoPacket const & p);
    void handleTunnelPacket(ShupitoPacket const & p);

    void SendSetComSpeed();

    ShupitoConnection *m_con;
    ConnectionPointer<ShupitoTunnel> m_tunnel_conn;

    ShupitoDesc *m_desc;
    QMutex mutex;
    vdd_setup m_vdd_setup;

    ShupitoDesc::config const *m_vdd_config;
    ShupitoDesc::config const *m_tunnel_config;

    quint8 m_tunnel_pipe;
    quint32 m_tunnel_speed;
    QByteArray m_tunnel_data;
    QTimer m_tunnel_timer;

    QTimer *responseTimer;
    ShupitoPacket m_wait_packet;
    quint8 m_wait_cmd;
    QByteArray m_wait_data;
    quint8 m_wait_type;
    quint16 m_wait_max_packets;

    chip_definition m_chip_def;
};

#endif // SHUPITO_H
