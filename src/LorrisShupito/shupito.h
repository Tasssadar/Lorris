/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef SHUPITO_H
#define SHUPITO_H

#include <QObject>
#include <QByteArray>
#include <QMutex>

#include "shupitodesc.h"

enum Opcodes
{
    MSG_INFO       = 0x00,
    MSG_VCC        = 0x0A,
    MSG_TUNNEL     = 0x09
};

class Connection;

// device.hpp, 122
struct vdd_point
{
    QString name;
    std::vector<QString> drives;
    quint16 current_drive;
};

typedef std::vector<vdd_point> vdd_setup;

class ShupitoPacket
{
public:
    ShupitoPacket(quint8 cmd, quint8 size, ...);
    ShupitoPacket();

    QByteArray getData(bool onlyPacketData = true)
    {
        if(onlyPacketData)
            return m_data.right(m_data.length() - 2);
        else
            return m_data;
    }

    quint8 a(quint32 pos, bool onlyPacketData = true)
    {
        return (quint8)m_data[pos + (onlyPacketData ? 2 : 0)];
    }

    quint8 getOpcode() { return (quint8(m_data[1]) >> 4); }
    quint8 getLen() { return (quint8(m_data[1]) & 0x0F); }

    void Clear();
    bool isValid();

    quint8 addData(const QByteArray& data);

private:
    QByteArray m_data;
    quint8 itr;
};

class Shupito : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void descRead();
    void responseReceived(char error_code);
    void vccValueChanged(quint8 id, double value);
    void vddDesc(const vdd_setup& vs);

public:
    explicit Shupito(QObject *parent);
    void init(Connection *con, ShupitoDesc *desc);

    void readData(const QByteArray& data);
    void sendPacket(ShupitoPacket& packet);

    void setVddConfig(ShupitoDesc::config *cfg) { m_vdd_config = cfg; }
    void setTunnelConfig(ShupitoDesc::config *cfg) { m_tunnel_config = cfg; }

    void setTunnelSpeed(quint32 speed, bool send = true);

private:
    void handlePacket(ShupitoPacket& p);
    void handleVccPacket(ShupitoPacket& p);
    void handleTunnelPacket(ShupitoPacket& p);

    void SendSetComSpeed();

    Connection *m_con;
    ShupitoPacket m_packet;
    ShupitoDesc *m_desc;
    QMutex mutex;
    vdd_setup m_vdd_setup;

    ShupitoDesc::config *m_vdd_config;
    ShupitoDesc::config *m_tunnel_config;

    quint8 m_tunnel_pipe;
    quint32 m_tunnel_speed;
};

#endif // SHUPITO_H
