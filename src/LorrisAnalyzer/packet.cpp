/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "packet.h"
#include "../common.h"

analyzer_data::analyzer_data(analyzer_packet *packet)
{
    m_packet = packet;
    clear();
}

void analyzer_data::clear()
{
    m_data.clear();
    m_static_data = QByteArray((char*)m_packet->static_data.data(), m_packet->header->static_len);
    itr = 0;
    m_forceValid = false;
}

quint32 analyzer_data::addData(char *d_itr, char *d_end)
{
    quint32 read = 0;
    for(; itr < (quint32)m_static_data.length() && d_itr+read != d_end;)
    {
        if(*(d_itr+read) != m_static_data[itr])
            return read;
        m_data[itr++] = *(d_itr+read++);
    }

    bool readFromHeader = false;
    quint32 len = getLenght(&readFromHeader);
    quint32 var_len = 0;

    quint8 lenRead = (m_packet->header->data_mask & DATA_LEN);

    for(quint32 i = itr; i < len && d_itr+read != d_end; ++i)
    {
        if(!readFromHeader && lenRead == DATA_LEN && getLenFromHeader(var_len))
        {
            len += var_len;
            lenRead = 0;
        }

        m_data[itr++] = *(d_itr+read++);
    }
    return read;
}

quint32 analyzer_data::getLenght(bool *readFromHeader)
{
    if(m_packet->header->data_mask & DATA_LEN)
    {
        quint32 res = 0;
        if(getLenFromHeader(res))
        {
            if(readFromHeader)
                *readFromHeader = true;
            return res + m_packet->header->length;
        }
        else
        {
            if(readFromHeader)
                *readFromHeader = false;
            return m_packet->header->length;
        }
    }
    else
        return m_packet->header->packet_length;
}

bool analyzer_data::isValid()
{
    if(m_forceValid)
        return true;

    if(m_data.isEmpty() || itr < (quint32)m_static_data.length())
        return false;

    for(quint8 i = 0; i < m_static_data.length(); ++i)
    {
        if(m_data[i] != m_static_data[i])
            return false;
    }

    if(itr != getLenght())
        return false;
    return true;
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

bool analyzer_data::getLenFromHeader(quint32& len)
{
    if(!(m_packet->header->data_mask & DATA_LEN))
        return false;

    try
    {
        quint32 pos = getHeaderDataPos(DATA_LEN);
        switch(m_packet->header->len_fmt)
        {
            case 0: len = getUInt8(pos);  break;
            case 1: len = getUInt16(pos); break;
            case 2: len = getUInt32(pos); break;
            default: return false;
        }
        len += m_packet->header->len_offset;
    }
    catch(const char*)
    {
        return false;
    }

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
