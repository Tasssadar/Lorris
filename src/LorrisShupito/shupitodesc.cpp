/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "../common.h"
#include "shupito.h"
#include "shupitodesc.h"

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

void ShupitoDesc::AddData(const QByteArray& data)
{
    m_data.append(data);

    quint8 *first = (quint8*)m_data.data();
    quint8 *last = (quint8*)(first + m_data.size());

    if((last - first) < 17 || *first++ != 1)
        return Utils::showErrorBox("Invalid descriptor.");

    m_guid = makeGuid(first);
    first += 16;

    quint8 base_cmd = 1;
    std::vector<quint8> act_seq;
    parseGroupConfig(first, last, base_cmd, act_seq);
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
        return Utils::showErrorBox("Invalid descriptor.");

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
        return Utils::showErrorBox("Invalid descriptor.");

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
        return Utils::showErrorBox("Invalid descriptor.");

    cfg.data.assign(first, first + data_len);
    first += data_len;

    m_interface_map[cfg.guid] = cfg;
}

ShupitoPacket ShupitoDesc::config::getStateChangeCmd(bool activate)
{
    ShupitoPacket pkt;
    pkt.push_back(0);
    pkt.push_back(activate ? 0x01 : 0x02);
    for(quint8 i = 0; i < actseq.size(); ++i)
        pkt.push_back(actseq[i]);
    return pkt;
}
