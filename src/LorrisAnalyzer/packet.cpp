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
    quint16 val = 0;
    getInt(val, pos);

    if(!m_packet->big_endian)
        val = Utils::swapEndian16(val);
    return val;
}

qint16 analyzer_data::getInt16(quint32 pos)
{
    qint16 val = 0;
    getInt(val, pos);

    if(!m_packet->big_endian)
        val = Utils::swapEndian16(val);
    return val;
}

quint32 analyzer_data::getUInt32(quint32 pos)
{
    quint32 val = 0;
    getInt(val, pos);

    if(!m_packet->big_endian)
        val = Utils::swapEndian32(val);
    return val;
}

qint32 analyzer_data::getInt32(quint32 pos)
{
    qint32 val = 0;
    getInt(val, pos);

    if(!m_packet->big_endian)
        val = Utils::swapEndian32(val);
    return val;
}

quint64 analyzer_data::getUInt64(quint32 pos)
{
    quint64 val = 0;
    getInt(val, pos);

    if(!m_packet->big_endian)
        val = Utils::swapEndian64(val);
    return val;
}

qint64 analyzer_data::getInt64(quint32 pos)
{
    qint64 val = 0;
    getInt(val, pos);

    if(!m_packet->big_endian)
        val = Utils::swapEndian64(val);
    return val;
}

template<class T>
void analyzer_data::getInt(T& val, quint32 pos)
{
    if(pos+sizeof(val) > (quint32)m_data.length())
        return;

    for(qint8 i = sizeof(val)-1; i >= 0; --i)
        val |= ((quint64)m_data[pos++] << (i*8));
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
