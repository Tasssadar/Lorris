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

#include "shupito.h"
#include "shupitodesc.h"
#include <exception>
#include <stdexcept>

ShupitoDesc::ShupitoDesc()
{
    Clear();
}

void ShupitoDesc::Clear()
{
    m_guid = "";
    m_data.clear();
    m_interface_map.clear();
}

void ShupitoDesc::AddData(const QByteArray& data, bool finish)
{
    m_data.append(data);
    if(finish)
    {
        quint8 *first = (quint8*)m_data.data();
        quint8 *last = (quint8*)(m_data.data() + m_data.size());

        if((last - first) < 17 || *first++ != 1)
            throw std::runtime_error("Invalid descriptor.");

        m_guid = makeGuid(first);
        first += 16;

        quint8 base_cmd;
        std::vector<quint8> act_seq;
        parseGroupConfig(first, last, base_cmd, act_seq);
    }
}

// Device guid: 093d7f32-cdc6-4928-955d-513d17a85358 for Shupito 2.0
QString ShupitoDesc::makeGuid(quint8 *data)
{
    QString guid = "";
    for(quint8 i = 0; i < 16; ++i)
    {
        static char const digits[] = "0123456789abcdef";
        guid.append(digits[quint8(data[i]) >> 4]);
        guid.append(digits[quint8(data[i]) & 0x0F]);
    }
    guid.insert(20, "-");
    guid.insert(16, "-");
    guid.insert(12, "-");
    guid.insert(8, "-");
    return guid;
}

void ShupitoDesc::parseGroupConfig(quint8 *& first, quint8 *& last, quint8& base_cmd, std::vector<quint8>& actseq)
{
    if(first == last)
        throw std::runtime_error("Invalid descriptor.");

    if(*first == 0)
    {
        ++first;
        return parseConfig(first, last, base_cmd, actseq);
    }

    quint8 count = *first & 0x7F;

    // Or
    if(*first & 0x80)
    {
        ++first;
        quint8 next_base_cmd = base_cmd;
        for(quint8 i = 0; i < count; ++i)
        {
            quint8 or_base_cmd = base_cmd;
            actseq.push_back(i);
            parseGroupConfig(first, last, or_base_cmd, actseq);
            actseq.pop_back();
            next_base_cmd = std::max(next_base_cmd, or_base_cmd);
        }
        base_cmd = next_base_cmd;
    }
    // And
    else
    {
        ++first;
        for(quint8 i = 0; i < count; ++i)
        {
            actseq.push_back(i);
            parseGroupConfig(first, last, base_cmd, actseq);
            actseq.pop_back();
        }
    }
}

void ShupitoDesc::parseConfig(quint8 *& first, quint8 *& last, quint8& base_cmd, std::vector<quint8>& actseq)
{
    if ((last - first) < 19)
        throw std::runtime_error("Invalid descriptor.");

    config cfg;
    cfg.flags = *first++;
    cfg.guid = makeGuid(first);
    first += 16;

    cfg.cmd = *first++;
    cfg.cmd_count = *first++;
    base_cmd += cfg.cmd_count;

    cfg.actseq = actseq;

    quint8 data_len = *first++;
    if (last - first < data_len)
        throw std::runtime_error("Invalid descriptor.");

    cfg.data.assign(first, first + data_len);
    first += data_len;

    m_interface_map[cfg.guid] = cfg;
}

ShupitoPacket ShupitoDesc::config::getStateChangeCmd(bool activate)
{
    ShupitoPacket packet = ShupitoPacket();
    QByteArray data;
    data[0] = 0x80;
    data[1] = actseq.size() + 1;
    data[2] = activate ? 0x01 : 0x02;
    for(quint8 i = 0; i < actseq.size(); ++i)
        data[i+3] = actseq[i];
    packet.addData(data);
    return packet;
}
