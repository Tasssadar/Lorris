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
#include "connection/connectionmgr.h"

Shupito::Shupito(QObject *parent) :
    QObject(parent)
{
    m_con = NULL;
    m_desc = NULL;
    m_packet = NULL;

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
    sConMgr.RemoveShupito(this);
}

void Shupito::init(Connection *con, ShupitoDesc *desc)
{
    m_con = con;
    m_desc = desc;

    chip_definition::parse_default_chipsets(m_chip_defs);

    delete m_packet;
    m_packet = new ShupitoPacket();

    ShupitoPacket getInfo(MSG_INFO, 1, 0x00);
    QByteArray data = waitForStream(getInfo, MSG_INFO);
    if(!data.isEmpty())
        m_desc->AddData(data);

    emit descRead(!data.isEmpty());
}

void Shupito::readData(const QByteArray &data)
{
    mutex.lock();

    static bool first = true;

    std::vector<ShupitoPacket*> packets;
    ShupitoPacket *packet = m_packet;

    char *d_start = (char*)data.data();
    char *d_itr = d_start;
    char *d_end = d_start + data.size();

    quint8 curRead = 1;
    while(d_itr != d_end)
    {
        if(first || curRead == 0)
        {
            int index = data.indexOf(char(0x80), d_itr - d_start);
            if(index == -1)
                break;
            d_itr = d_start+index;
            first = false;
        }
        curRead = packet->addData(d_itr, d_end);
        d_itr += curRead;
        if(packet->isValid())
        {
            packets.push_back(packet);
            packet = new ShupitoPacket();
        }
    }
    m_packet = packet;
    mutex.unlock();

    for(quint32 i = 0; i < packets.size(); ++i)
    {
        handlePacket(*packets[i]);
        delete packets[i];
    }
}

ShupitoPacket Shupito::waitForPacket(const QByteArray& data, quint8 cmd)
{
    Q_ASSERT(responseTimer == NULL);

    responseTimer = new QTimer;
    responseTimer->start(1000);
    connect(responseTimer, SIGNAL(timeout()), this, SIGNAL(packetReveived()));

    m_wait_cmd = cmd;
    m_wait_packet = ShupitoPacket();
    m_wait_type = WAIT_PACKET;

    QEventLoop loop;
    loop.connect(this, SIGNAL(packetReveived()), SLOT(quit()));

    m_con->SendData(data);

    loop.exec();

    delete responseTimer;
    responseTimer = NULL;
    m_wait_cmd = 0xFF;
    m_wait_type = WAIT_NONE;

    return m_wait_packet;
}

QByteArray Shupito::waitForStream(const QByteArray& data, quint8 cmd, quint16 max_packets)
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

    m_con->SendData(data);

    loop.exec();

    delete responseTimer;
    responseTimer = NULL;
    m_wait_cmd = 0xFF;
    m_wait_type = WAIT_NONE;

    return m_wait_data;
}

void Shupito::sendPacket(ShupitoPacket packet)
{
    m_con->SendData(packet.getData(false));
}

void Shupito::sendPacket(const QByteArray& packet)
{
    m_con->SendData(packet);
}

void Shupito::handlePacket(ShupitoPacket& p)
{
    switch(m_wait_type)
    {
        case WAIT_NONE: break;
        case WAIT_PACKET:
        {
            if(p.getOpcode() == m_wait_cmd)
            {
                m_wait_packet = p;
                m_wait_type = WAIT_NONE;
                emit packetReveived();
            }
            break;
        }
        case WAIT_STREAM:
        {
            if(p.getOpcode() != m_wait_cmd)
            {
                if(--m_wait_max_packets == 0)
                    emit packetReveived();
            }
            else
            {
                m_wait_max_packets = 0;
                responseTimer->start(1000);
                m_wait_data.append(p.getData());
                if(p.getLen() < 15)
                {
                    m_wait_type = WAIT_NONE;
                    emit packetReveived();
                }
            }
            break;
        }
    }

    switch(p.getOpcode())
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

void Shupito::handleVccPacket(ShupitoPacket &p)
{
    //void handle_packet(yb_packet const & p)
    //vcc value
    if(p.getLen() == 4 && p[0] == 0x01 && p[1] == 0x03)
    {
        ShupitoDesc::config *vddCfg = m_desc->getConfig("1d4738a0-fc34-4f71-aa73-57881b278cb1");
        if(!vddCfg)
            return;

        double millivolts_per_unit = 1.0;
        if (vddCfg->data.size() == 5 && vddCfg->data[0] == 1)
        {
            qint32 mpu_16_16 = vddCfg->data[1] | (vddCfg->data[2] << 8) | (vddCfg->data[3] << 16) | (vddCfg->data[4] << 24);
            millivolts_per_unit = mpu_16_16 / 65536.0;
        }

        qint32 vdd = (qint8)p[p.getLen()-1];
        for (int i = p.getLen() - 1; i > 2; --i)
            vdd = (vdd << 8) | p[i-1];

        double value = vdd * (millivolts_per_unit / 1000.0);

        emit vccValueChanged(p[0] - 1, value);
    }

    // bool handle_vdd_desc(yb_packet const & p)
    else if(p.getLen() >= 3 && p[0] == 0 && p[1] == 0 && p[2] == 0)
    {
        m_vdd_setup.clear();
        for(quint8 i = 3; i < p.getLen(); ++i)
        {
            vdd_point vp;
            vp.current_drive = 0;
            quint8 len = p[i];
            if(i + 1 + len > p.getLen())
               return;
            vp.name.append(p.getData().mid(i+1, len));
            i += len + 1;
            m_vdd_setup.push_back(vp);
        }
        emit vddDesc(m_vdd_setup);
    }

    //bool handle_vdd_point_desc(yb_packet const & p)
    else if(p.getLen() >= 2 && p[0] == 0)
    {
        if(p[1] == 0 || p[1] > m_vdd_setup.size())
            return;

        vdd_point & vp = m_vdd_setup[p[1] - 1];
        vp.drives.clear();

        for(quint8 i = 2; i < p.getLen();)
        {
            if(i + 4 <= p.getLen())
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
    else if(p.getLen() == 3 && p[1] == 1)
    {
        if(p[0]== 0 || p[0] > m_vdd_setup.size())
            return;

        vdd_point & vp = m_vdd_setup[p[1] - 1];
        vp.current_drive = p[2];

        emit vddDesc(m_vdd_setup);
    }
}

void Shupito::handleTunnelPacket(ShupitoPacket &p)
{
    if(!m_tunnel_config || p.getOpcode() != m_tunnel_config->cmd)
        return;

    if(p.getLen() >= 2 && p[0] == 0)
    {
        switch(p[1])
        {
            // Tunnel list
            case 0:
            {
                for(quint8 i = 2; i < p.getLen(); ++i)
                {
                    if (i + 1 + p[i] > p.getLen())
                        return Utils::ThrowException("Invalid response while enumerating pipes.");
                    i += 1 + p[i];
                }

                setTunnelState(true);
                break;
            }
            // App tunnel activated
            case 1:
            {
                if(p.getLen() == 3)
                {
                    m_tunnel_pipe = p[2];
                    if(!m_tunnel_pipe)
                        return;

                    SendSetComSpeed();

                    sConMgr.AddShupito(m_con->GetIDString(), this);
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
                if(p.getLen() == 3 && p[2] == m_tunnel_pipe)
                {
                    m_tunnel_pipe = 0;

                    sConMgr.RemoveShupito(this);
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
    if (m_tunnel_pipe && p[0] == m_tunnel_pipe)
    {
        QByteArray data = p.getData();
        m_tunnel_data.append(data.remove(0, 1));
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

    ShupitoPacket pkt = ShupitoPacket(m_tunnel_config->cmd, 5, 0, 3, m_tunnel_pipe,
                                      (quint8)res, (quint8)(res >> 8));
    sendPacket(pkt);
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

        QByteArray pkt_data;
        pkt_data[0] = 0x80;
        pkt_data[1] = (m_tunnel_config->cmd << 4) | (0x02 + name.length());
        pkt_data[2] = 0x00;
        pkt_data[3] = 0x01;
        pkt_data.append(name);

        if(wait)
            waitForPacket(pkt_data, m_tunnel_config->cmd);
        else
            m_con->SendData(pkt_data);
    }
    else if(!enable && m_tunnel_pipe != 0)
    {
        ShupitoPacket packet(m_tunnel_config->cmd, 3, 0, 2, m_tunnel_pipe);
        if(wait)
            waitForPacket(packet.getData(false), m_tunnel_config->cmd);
        else
            sendPacket(packet);
    }
}

