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

#include <stdarg.h>
#include <stdio.h>
#include <QEventLoop>

#include "shupito.h"
#include "lorrisshupito.h"
#include "shupitodesc.h"
#include "../connection/shupitotunnel.h"
#include "../connection/connectionmgr2.h"
#include "../connection/shupitoconn.h"

Shupito::Shupito(QObject *parent) :
    QObject(parent)
{
    m_con = NULL;
    m_desc = NULL;

    m_vdd_config = m_tunnel_config = NULL;
    m_tunnel_pipe = 0;
    m_tunnel_speed = 0;

    m_tunnel_timer.setInterval(50);

    responseTimer = NULL;
    m_wait_cmd = 0xFF;
    m_wait_type = WAIT_NONE;
}

Shupito::~Shupito()
{
}

void Shupito::init(ShupitoConnection *con, ShupitoDesc *desc)
{
    m_con = con;
    m_desc = desc;

    //chip_definition::parse_default_chipsets(m_chip_defs);

    ShupitoPacket getInfo = makeShupitoPacket(MSG_INFO, 1, 0x00);
    QByteArray data = waitForStream(getInfo, MSG_INFO);
    if(!data.isEmpty())
        m_desc->AddData(data);

    emit descRead(!data.isEmpty());
}

void Shupito::sendPacket(const ShupitoPacket& data)
{
    m_con->sendPacket(data);
}

void Shupito::readPacket(const ShupitoPacket & p)
{
    Q_ASSERT(!p.empty());
    switch(m_wait_type)
    {
        case WAIT_NONE: break;
        case WAIT_PACKET:
        {
            if(p[0] == m_wait_cmd)
            {
                m_wait_packet = p;
                m_wait_type = WAIT_NONE;
                emit packetReveived();
            }
            break;
        }
        case WAIT_STREAM:
        {
            if(p[0] != m_wait_cmd)
            {
                if(--m_wait_max_packets == 0)
                    emit packetReveived();
            }
            else
            {
                responseTimer->start(1000);
                m_wait_data.append((char const *)(p.data() + 1), p.size() - 1);
                if(p.size() < 16)
                {
                    m_wait_max_packets = 0;
                    m_wait_type = WAIT_NONE;
                    emit packetReveived();
                }
            }
            break;
        }
    }

    // FIXME: commands are offset based on the descriptor
    switch(p[0])
    {
        case MSG_VCC:
        {
            handleVccPacket(p);
            break;
        }
        case MSG_TUNNEL:
        {
            handleTunnelPacket(p);
            break;
        }
    }
}

ShupitoPacket Shupito::waitForPacket(const ShupitoPacket & data, quint8 cmd)
{
    Q_ASSERT(responseTimer == NULL);

    responseTimer = new QTimer;
    responseTimer->start(1000);
    connect(responseTimer, SIGNAL(timeout()), this, SIGNAL(packetReveived()));

    m_wait_cmd = cmd;
    m_wait_packet.clear();
    m_wait_type = WAIT_PACKET;

    QEventLoop loop;
    loop.connect(this, SIGNAL(packetReveived()), SLOT(quit()));

    m_con->sendPacket(data);

    loop.exec();

    delete responseTimer;
    responseTimer = NULL;
    m_wait_cmd = 0xFF;
    m_wait_type = WAIT_NONE;

    return m_wait_packet;
}

QByteArray Shupito::waitForStream(const ShupitoPacket& data, quint8 cmd, quint16 max_packets)
{
    Q_ASSERT(responseTimer == NULL);

    responseTimer = new QTimer;
    responseTimer->start(1000);
    connect(responseTimer, SIGNAL(timeout()), this, SIGNAL(packetReveived()));

    m_wait_cmd = cmd;
    m_wait_data.clear();
    m_wait_type = WAIT_STREAM;
    m_wait_max_packets = max_packets;

    QEventLoop loop;
    loop.connect(this, SIGNAL(packetReveived()), SLOT(quit()));

    m_con->sendPacket(data);

    loop.exec();

    delete responseTimer;
    responseTimer = NULL;
    m_wait_cmd = 0xFF;
    m_wait_type = WAIT_NONE;

    return m_wait_data;
}

void Shupito::sendTunnelData(const QByteArray &data)
{
    if(!m_tunnel_pipe)
        return;

    int sent = 0;
    const int data_len = data.length();

    quint8 cmd = getTunnelCmd();
    char pipe = getTunnelId();

    while(sent != data_len)
    {
        int chunk = data_len - sent;
        if(chunk > 14)
            chunk = 14;

        ShupitoPacket packet;
        packet.push_back(cmd);
        packet.push_back(pipe);

        quint8 const * d = (quint8 const *)data.constData();
        packet.insert(packet.end(), d + sent, d + sent + chunk);

        m_con->sendPacket(packet);

        sent += chunk;
    }
}

void Shupito::handleVccPacket(ShupitoPacket const & p)
{
    //void handle_packet(yb_packet const & p)
    //vcc value
    if(p.size() == 5 && p[1] == 0x01 && p[2] == 0x03)
    {
        if(!m_vdd_config)
            return;

        double millivolts_per_unit = 1.0;
        if (m_vdd_config->data.size() == 5 && m_vdd_config->data[0] == 1)
        {
            qint32 mpu_16_16 = m_vdd_config->data[1] | (m_vdd_config->data[2] << 8) | (m_vdd_config->data[3] << 16) | (m_vdd_config->data[4] << 24);
            millivolts_per_unit = mpu_16_16 / 65536.0;
        }

        qint32 vdd =(qint8) p.back();
        for (int i = p.size() - 1; i > 3; --i)
            vdd = (vdd << 8) | p[i-1];

        double value = vdd * (millivolts_per_unit / 1000.0);

        emit vccValueChanged(p[1] - 1, value);
    }

    // bool handle_vdd_desc(yb_packet const & p)
    else if(p.size() >= 4 && p[1] == 0 && p[2] == 0 && p[3] == 0)
    {
        m_vdd_setup.clear();
        for(quint8 i = 4; i < p.size(); ++i)
        {
            vdd_point vp;
            vp.current_drive = 0;
            quint8 len = p[i];
            if(i + 1 + len > p.size())
               return;
            vp.name = QString::fromUtf8((char const *)p.data() + i+1, len);
            i += len + 1;
            m_vdd_setup.push_back(vp);
        }
        emit vddDesc(m_vdd_setup);
    }

    //bool handle_vdd_point_desc(yb_packet const & p)
    else if(p.size() >= 3 && p[1] == 0)
    {
        if(p[2] == 0 || p[2] > m_vdd_setup.size())
            return;

        vdd_point & vp = m_vdd_setup[p[2] - 1];
        vp.drives.clear();

        for(quint8 i = 3; i < p.size();)
        {
            if(i + 4 <= p.size())
            {
                double voltage = (p[i] | (p[i+1] << 8)) / 1000.0;
                quint16 amps = p[i+2]| (p[i+3] << 8);
                if(amps == 0)
                    vp.drives.push_back("<hiz>");
                else
                {
                    char buff[50];
                    sprintf(buff, "%.2fV, %dmA", voltage, amps);
                    vp.drives.push_back(QString(buff));
                }
                i += 4;
            }
        }
        emit vddDesc(m_vdd_setup);
    }

    //bool handle_vdd_drive_state(yb_packet const & p)
    else if(p.size() == 4 && p[2] == 1)
    {
        if(p[1]== 0 || p[1] > m_vdd_setup.size())
            return;

        vdd_point & vp = m_vdd_setup[p[1] - 1];
        vp.current_drive = p[3];

        emit vddDesc(m_vdd_setup);
    }
}

void Shupito::handleTunnelPacket(ShupitoPacket const & p)
{
    if(!m_tunnel_config || p[0] != m_tunnel_config->cmd)
        return;

    if(p.size() >= 3 && p[1] == 0)
    {
        switch(p[2])
        {
            // Tunnel list
            case 0:
            {
                for(quint8 i = 3; i < p.size(); ++i)
                {
                    if (i + 1 + p[i] > p.size())
                        return Utils::ThrowException("Invalid response while enumerating pipes.");
                    i += 1 + p[i];
                }

                setTunnelState(true);
                break;
            }
            // App tunnel activated
            case 1:
            {
                if(p.size() == 4)
                {
                    m_tunnel_pipe = p[3];
                    if(!m_tunnel_pipe)
                        return;

                    SendSetComSpeed();

                    m_tunnel_conn.reset(new ShupitoTunnel());
                    m_tunnel_conn->setName("Tunnel at " + m_con->GetIDString());
                    m_tunnel_conn->setRemovable(false);
                    m_tunnel_conn->setShupito(this);
                    m_tunnel_conn->Open();
                    sConMgr2.addConnection(m_tunnel_conn.data());

                    emit tunnelStatus(true);

                    m_tunnel_data.clear();
                    connect(&m_tunnel_timer, SIGNAL(timeout()), SLOT(tunnelDataSend()));
                    m_tunnel_timer.start();
                    return;
                }
                break;
            }
            //App tunnel disabled
            case 2:
            {
                if(p.size() == 4 && p[3] == m_tunnel_pipe)
                {
                    m_tunnel_pipe = 0;

                    m_tunnel_conn.reset();
                    emit tunnelStatus(false);

                    disconnect(&m_tunnel_timer, SIGNAL(timeout()), this, SLOT(tunnelDataSend()));
                    m_tunnel_timer.stop();
                    return;
                }
                break;
            }
        }
    }
    // Tunnel incoming data
    if (m_tunnel_pipe && p[1] == m_tunnel_pipe)
    {
        m_tunnel_data.append((char const *)p.data() + 2, p.size() - 2);
    }
}

//void send_set_comm_speed()
void Shupito::SendSetComSpeed()
{
    if(!m_tunnel_speed || !m_tunnel_config || m_tunnel_pipe == 0 ||
       m_tunnel_config->data.size() != 5 || m_tunnel_config->data[0] != 1)
        return;

    quint32 base_freq = m_tunnel_config->data[1] | (m_tunnel_config->data[2] << 8) |
            (m_tunnel_config->data[3] << 16) | (m_tunnel_config->data[4] << 24);

    double bsel = ((double)base_freq / m_tunnel_speed) - 1;
    qint8 bscale = 0;

    while(bscale > -6 && bsel < 2048)
    {
        bsel *= 2;
        --bscale;
    }

    quint16 res = (quint16)(bsel + 0.5);
    res |= bscale << 12;

    ShupitoPacket pkt = makeShupitoPacket(m_tunnel_config->cmd, 5, 0, 3, m_tunnel_pipe,
                                      (quint8)res, (quint8)(res >> 8));
    m_con->sendPacket(pkt);
}

void Shupito::setTunnelSpeed(quint32 speed, bool send)
{
    m_tunnel_speed = speed;
    if(send)
        SendSetComSpeed();
}

qint16 Shupito::getTunnelCmd()
{
    if(m_tunnel_config)
        return m_tunnel_config->cmd;
    return -1;
}

void Shupito::tunnelDataSend()
{
    if(m_tunnel_data.size() > 0)
    {
        emit tunnelData(m_tunnel_data);
        m_tunnel_data.clear();
    }
    m_tunnel_timer.start();
}

void Shupito::setTunnelState(bool enable, bool wait)
{
    if(!m_tunnel_config)
        return;

    if(enable && m_tunnel_pipe == 0)
    {
        QString name = sConfig.get(CFG_STRING_SHUPITO_TUNNEL);
        QByteArray name8 = name.toUtf8();
        quint8 const * d = (quint8 const *)name8.constData();

        ShupitoPacket pkt_data = makeShupitoPacket(m_tunnel_config->cmd, 2, 0, 1);
        pkt_data.insert(pkt_data.end(), d, d + name8.size());

        if(wait)
            waitForPacket(pkt_data, m_tunnel_config->cmd);
        else
            m_con->sendPacket(pkt_data);
    }
    else if(!enable && m_tunnel_pipe != 0)
    {
        ShupitoPacket packet = makeShupitoPacket(m_tunnel_config->cmd, 3, 0, 2, m_tunnel_pipe);
        if(wait)
            waitForPacket(packet, m_tunnel_config->cmd);
        else
            m_con->sendPacket(packet);
    }
}

