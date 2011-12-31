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

    quint16 pos = 0;
    for(quint8 i = 0; i < 4; ++i)
    {
        switch(m_packet->header->order[i])
        {
            case DATA_STATIC:
                pos += m_packet->header->static_len;
                break;
            case DATA_LEN:
                pos += (1 << m_packet->header->len_fmt);
                break;
            case DATA_OPCODE:
                ++pos;
                break;
            case DATA_DEVICE_ID:
                id = (quint8)m_data[pos];
                return true;
        }
    }
    return false;
}

bool analyzer_data::getCmd(quint8 &cmd)
{
    if(!(m_packet->header->data_mask & DATA_OPCODE))
        return false;

    quint16 pos = 0;
    for(quint8 i = 0; i < 4; ++i)
    {
        switch(m_packet->header->order[i])
        {
            case DATA_STATIC:
                pos += m_packet->header->static_len;
                break;
            case DATA_LEN:
                pos += (1 << m_packet->header->len_fmt);
                break;
            case DATA_DEVICE_ID:
                ++pos;
                break;
            case DATA_OPCODE:
                cmd = (quint8)m_data[pos];
                return true;
        }
    }
    return false;
}

quint8 analyzer_data::getUInt8(quint32 pos)
{
    if(pos >= (quint32)m_data.length())
        return 0;
    return (quint8)m_data[pos];
}

qint8 analyzer_data::getInt8(quint32 pos)
{
    if(pos >= (quint32)m_data.length())
        return 0;
    return (qint8)m_data[pos];
}

quint16 analyzer_data::getUInt16(quint32 pos)
{
    if(pos+2 > (quint32)m_data.length())
        return 0;
    quint16 val = ((quint8)m_data[pos] << 8) | (quint8)m_data[pos+1];
    if(!m_packet->big_endian)
        val = Utils::swapEndian16(val);
    return val;
}

qint16 analyzer_data::getInt16(quint32 pos)
{
    if(pos+2 > (quint32)m_data.length())
        return 0;
    qint16 val = ((quint8)m_data[pos] << 8) | (quint8)m_data[pos+1];
    if(!m_packet->big_endian)
        val = Utils::swapEndian16(val);
    return val;
}

quint32 analyzer_data::getUInt32(quint32 pos)
{
    if(pos+4 > (quint32)m_data.length())
        return 0;
    quint32 val = ((quint8)m_data[pos++] << 24) | ((quint8)m_data[pos++] << 16) |
                  ((quint8)m_data[pos++] << 8)  | (quint8)m_data[pos];
    if(!m_packet->big_endian)
        val = Utils::swapEndian32(val);
    return val;
}

qint32 analyzer_data::getInt32(quint32 pos)
{
    if(pos+4 > (quint32)m_data.length())
        return 0;
    qint32 val = ((quint8)m_data[pos++] << 24) | ((quint8)m_data[pos++] << 16) |
                  ((quint8)m_data[pos++] << 8)  | (quint8)m_data[pos];
    if(!m_packet->big_endian)
        val = Utils::swapEndian32(val);
    return val;
}

quint64 analyzer_data::getUInt64(quint32 pos)
{
    if(pos+8 > (quint32)m_data.length())
        return 0;
    quint64 val = ((quint64)m_data[pos++] << 56) | ((quint64)m_data[pos++] << 48) |
                  ((quint64)m_data[pos++] << 40) | ((quint64)m_data[pos++] << 32) |
                  ((quint32)m_data[pos++] << 24) | ((quint32)m_data[pos++] << 16) |
                  ((quint16)m_data[pos++] << 8)  | (quint8)m_data[pos];
    if(!m_packet->big_endian)
        val = Utils::swapEndian64(val);
    return val;
}

qint64 analyzer_data::getInt64(quint32 pos)
{
    if(pos+8 > (quint32)m_data.length())
        return 0;
    qint64 val = ((quint64)m_data[pos++] << 56) | ((quint64)m_data[pos++] << 48) |
                 ((quint64)m_data[pos++] << 40) | ((quint64)m_data[pos++] << 32) |
                 ((quint32)m_data[pos++] << 24) | ((quint32)m_data[pos++] << 16) |
                 ((quint16)m_data[pos++] << 8)  | (quint8)m_data[pos];
    if(!m_packet->big_endian)
        val = Utils::swapEndian64(val);
    return val;
}

QString analyzer_data::getString(quint32 pos)
{
    QString str = "";
    if(pos >= (quint32)m_data.length())
        return str;
    for(; pos < (quint32)m_data.length() && QChar(m_data[pos]) != 0; ++pos)
        str.append(QChar(m_data[pos]));
    return str;
}
