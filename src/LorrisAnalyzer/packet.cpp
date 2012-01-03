#include "packet.h"
#include "common.h"

analyzer_data::analyzer_data(analyzer_packet *packet)
{
    m_packet = packet;
}

bool analyzer_data::getDeviceId(quint8& id)
{
    if(!(m_packet->header->data_mask & DATA_DEVICE_ID))
        return false;

    id = (quint8)m_data[getHeaderDataPos(DATA_DEVICE_ID)];
    return true;
}

bool analyzer_data::getCmd(quint8 &cmd)
{
    if(!(m_packet->header->data_mask & DATA_OPCODE))
        return false;

    cmd = (quint8)m_data[getHeaderDataPos(DATA_OPCODE)];
    return true;
}

quint16 analyzer_data::getHeaderDataPos(quint8 type)
{
    quint16 pos = 0;
    for(quint8 i = 0; i < 4; ++i)
    {
        if(m_packet->header->order[i] == type)
            return pos;

        switch(m_packet->header->order[i])
        {
            case DATA_STATIC:
                pos += m_packet->header->static_len;
                break;
            case DATA_LEN:
                pos += (1 << m_packet->header->len_fmt);
                break;
            case DATA_DEVICE_ID:
            case DATA_OPCODE:
                ++pos;
                break;
        }
    }
    return pos;
}

QString analyzer_data::getString(quint32 pos)
{
    QString str = "";
    if(pos >= (quint32)m_data.length())
        return str;
    for(; pos < (quint32)m_data.length() && m_data[pos] != '\0'; ++pos)
        str.append(QChar(m_data[pos]));
    return str;
}
