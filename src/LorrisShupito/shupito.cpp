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

#include "shupito.h"
#include "lorrisshupito.h"
#include "shupitodesc.h"

ShupitoPacket::ShupitoPacket(quint8 cmd, quint8 size, ...)
{
    itr = 0;

    m_data = QByteArray(size + 2, 0);
    m_data[itr++] = 0x80;
    m_data[itr++] = size | (cmd << 4);

    va_list args;
    va_start(args, size);
    for (quint8 i = 0; i < size; ++i)
        m_data[itr++] = (quint8)va_arg(args, int);
    va_end(args);
}

ShupitoPacket::ShupitoPacket()
{
    Clear();
}

void ShupitoPacket::Clear()
{
    m_data = QByteArray(2, 0);
    itr = 0;
}

bool ShupitoPacket::isValid()
{
    if((quint8)m_data.at(0) != 0x80 || itr < 2)
        return false;

    quint8 size = getLen();
    if((itr - 2) != size)
        return false;
    return true;
}

quint8 ShupitoPacket::addData(const QByteArray& data)
{
    quint8 read = 0;

    if(itr == 0)
    {
        if(quint8(data[read++]) == 0x80)
            m_data[itr++] = 0x80;
        else
            return 0;
    }
    if(itr == 1 && data.length() > 1)
        m_data[itr++] = data[read++];

    quint8 size = getLen();
    for(quint8 i = itr - 2; i < size && read < data.length(); ++i)
        m_data[itr++] = (char)data[read++];
    return read;
}

Shupito::Shupito(QObject *parent) :
    QObject(parent)
{
    m_con = NULL;
    m_desc = NULL;
    m_packet = ShupitoPacket();
}

void Shupito::init(Connection *con, ShupitoDesc *desc)
{
    m_con = con;
    m_desc = desc;
    ShupitoPacket getInfo(MSG_INFO, 1, 0x00);
    sendPacket(getInfo);
}

void Shupito::readData(const QByteArray &data)
{
    mutex.lock();
    QByteArray dta = data;

    static bool first = true;

    std::vector<ShupitoPacket> packets;
    ShupitoPacket packet = m_packet;

    quint8 curRead = 1;
    while(dta.length() > 0)
    {
        if(first || curRead == 0)
        {
            int index = dta.indexOf(char(0x80));
            if(index == -1)
                break;
            dta.remove(0, index);
            first = false;
        }
        curRead = packet.addData(dta);
        dta.remove(0, curRead);
        if(packet.isValid())
        {
            packets.push_back(packet);
            packet = ShupitoPacket();
        }
    }
    m_packet = packet;
    mutex.unlock();

    for(quint32 i = 0; i < packets.size(); ++i)
        handlePacket(packets[i]);

}

void Shupito::sendPacket(ShupitoPacket& packet)
{
    QByteArray data = packet.getData(false);
    m_con->SendData(packet.getData(false));
}

void Shupito::handlePacket(ShupitoPacket& p)
{
    switch(p.getOpcode())
    {
        case MSG_INFO:
        {
            if(p.getLen() == 1)
            {
                emit responseReceived(p.getData()[0]);
                break;
            }
            m_desc->AddData(p.getData(), p.getLen() < 15);
            if(p.getLen() < 15)
                emit descRead();
            break;
        }
        case MSG_VCC:
        {
            handleVccPacket(p);
            break;
        }
    }
}

void Shupito::handleVccPacket(ShupitoPacket &p)
{
    //void handle_packet(yb_packet const & p)
    //vcc value
    if(p.getLen() == 4 && p.a(0) == 0x01 && p.a(1) == 0x03)
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

        qint32 vdd = (qint8)p.a(p.getLen()-1);
        for (int i = p.getLen() - 1; i > 2; --i)
            vdd = (vdd << 8) | p.a(i-1);

        double value = vdd * (millivolts_per_unit / 1000.0);

        emit vccValueChanged(p.a(0) - 1, value);
    }

    // bool handle_vdd_desc(yb_packet const & p)
    else if(p.getLen() >= 3 && p.a(0) == 0 && p.a(1) == 0 && p.a(2) == 0)
    {
        m_vdd_setup.clear();
        for(quint8 i = 3; i < p.getLen(); ++i)
        {
            vdd_point vp;
            vp.current_drive = 0;
            quint8 len = p.a(i);
            if(i + 1 + len > p.getLen())
               return;
            vp.name.append(p.getData().mid(i+1, len));
            i += len + 1;
            m_vdd_setup.push_back(vp);
        }
        emit vddDesc(m_vdd_setup);
    }

    //bool handle_vdd_point_desc(yb_packet const & p)
    else if(p.getLen() >= 2 && p.a(0) == 0)
    {
        if(p.a(1) == 0 || p.a(1) > m_vdd_setup.size())
            return;

        vdd_point & vp = m_vdd_setup[p.a(1) - 1];
        vp.drives.clear();

        for(quint8 i = 2; i < p.getLen();)
        {
            if(i + 4 <= p.getLen())
            {
                double voltage = (p.a(i) | (p.a(i+1) << 8)) / 1000.0;
                quint16 amps = p.a(i+2) | (p.a(i+3) << 8);
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
    else if(p.getLen() == 3 && p.a(1) == 1)
    {
        if(p.a(0) == 0 || p.a(0) > m_vdd_setup.size())
            return;

        vdd_point & vp = m_vdd_setup[p.a(1) - 1];
        vp.current_drive = p.a(2);

        emit vddDesc(m_vdd_setup);
    }
}

