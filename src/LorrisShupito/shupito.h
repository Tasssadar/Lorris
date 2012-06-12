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

#include "shupitopacket.h"
#include "shupitodesc.h"
#include "../shared/chipdefs.h"

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
    MODE_JTAG,
    MODE_CC25XX,
    MODE_COUNT
};

enum WaitTypes
{
    WAIT_NONE = 0,
    WAIT_PACKET,
    WAIT_STREAM
};

enum VerifyMode
{
    VERIFY_NONE,
    VERIFY_ONLY_NON_EMPTY,
    VERIFY_ALL_PAGES,
    VERIFY_MAX
};

class PortConnection;
class ShupitoTunnel;

// device.hpp, 122
struct vdd_point
{
    QString name;
    std::vector<QString> drives;
    quint16 current_drive;
};

typedef std::vector<vdd_point> vdd_setup;

class Shupito : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void descRead(bool correct);
    void responseReceived(char error_code);
    void vccValueChanged(quint8 id, double value);
    void vddDesc(const vdd_setup& vs);
    void tunnelData(const QByteArray& data);
    void packetReceived();
    void tunnelStatus(bool);

public:
    explicit Shupito(QObject *parent);
    ~Shupito();
    void init(PortConnection *con, ShupitoDesc *desc);

    void readData(const QByteArray& data);
    void sendPacket(ShupitoPacket packet);
    void sendPacket(const QByteArray& packet);
    ShupitoPacket waitForPacket(const QByteArray &data, quint8 cmd);
    ShupitoPacket waitForPacket(ShupitoPacket& pkt, quint8 cmd)
    {
        return waitForPacket(pkt.getData(false), cmd);
    }

    QByteArray waitForStream(const QByteArray& data, quint8 cmd, quint16 max_packets = 1024);
    QByteArray waitForStream(ShupitoPacket& pkt, quint8 cmd, quint16 max_packets = 1024)
    {
        return waitForStream(pkt.getData(false), cmd, max_packets);
    }

    void setVddConfig(ShupitoDesc::config *cfg) { m_vdd_config = cfg; }
    void setTunnelConfig(ShupitoDesc::config *cfg) { m_tunnel_config = cfg; }

    void setTunnelSpeed(quint32 speed, bool send = true);
    qint16 getTunnelCmd();
    quint8 getTunnelId() { return m_tunnel_pipe; }
    void setTunnelState(bool enable, bool wait = false);
    void setTunnelPipe(quint8 pipe) { m_tunnel_pipe = pipe; }

    ShupitoDesc *getDesc() { return m_desc; }
    void setChipId(chip_definition cd) { m_chip_def = cd; }
    const chip_definition& getChipId() { return m_chip_def; }

public slots:
    void sendTunnelData(const QString& data)
    {
        sendTunnelData(data.toUtf8());
    }

    void sendTunnelData(const QByteArray& data);

private slots:
    void tunnelDataSend();

private:
    void handlePacket();
    void handleVccPacket();
    void handleTunnelPacket();

    void SendSetComSpeed();

    PortConnection *m_con;
    QScopedPointer<ShupitoTunnel> m_tunnel_conn;

    ShupitoPacket m_packet;
    ShupitoDesc *m_desc;
    QMutex mutex;
    vdd_setup m_vdd_setup;

    ShupitoDesc::config *m_vdd_config;
    ShupitoDesc::config *m_tunnel_config;

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
