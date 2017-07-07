/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "packet.h"
#include "../common.h"

analyzer_data::analyzer_data(QByteArray *data, analyzer_packet *packet)
{
    m_packet = packet;
    m_data = data;
}

void analyzer_data::clear()
{
    if(m_data)
        m_data->clear();
}

void analyzer_data::copy(analyzer_data *other)
{
    m_data = other->m_data;
    m_packet = other->m_packet;
}

quint32 analyzer_data::addData(char *d_itr, char *d_end, quint32 &itr)
{
    if(!m_packet)
        return 0;

    QByteArray static_data = m_packet->getStaticData();
    const quint32 staticOffset = m_packet->getStaticDataOffset();
    const quint32 staticOffsetEnd = staticOffset + static_data.length();
    quint32 staticItr = 0;
    bool readFromHeader = false;
    quint32 len = getLenght(&readFromHeader);
    quint32 var_len = 0;

    quint8 lenRead = m_packet->header->hasLen();

    quint32 read = 0;
    for(quint32 i = itr; i < len && d_itr+read != d_end; ++i)
    {
        if(i >= staticOffset && i < staticOffsetEnd)
        {
            if(static_data[staticItr] != *(d_itr+read))
                return read;
            ++staticItr;
        }
        else if(!readFromHeader && lenRead == DATA_LEN && getLenFromHeader(var_len))
        {
            len += var_len;
            lenRead = 0;
        }

        (*m_data)[itr++] = *(d_itr+read++);
    }
    return read;
}

quint32 analyzer_data::getLenght(bool *readFromHeader)
{
    if(m_packet->header->hasLen())
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

bool analyzer_data::isValid(quint32 itr)
{
    if(!m_packet)
        return false;

    QByteArray static_data = m_packet->getStaticData();

    if(m_data->isEmpty() || itr < (quint32)static_data.length())
        return false;

    quint32 static_pos = m_packet->header->findDataPos(DATA_STATIC);
    for(quint8 i = 0; i < static_data.length(); ++i)
    {
        if((*m_data)[static_pos++] != static_data[i])
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

    id = (quint8)m_data->at(m_packet->header->findDataPos(DATA_DEVICE_ID));
    return true;
}

bool analyzer_data::getCmd(quint8 &cmd)
{
    if(!m_packet->header->hasOpcode())
        return false;

    if(m_packet->header->data_mask & DATA_OPCODE)
    {
        int pos = m_packet->header->findDataPos(DATA_OPCODE);
        if(pos < 0 || pos >= m_data->size())
            return false;
        cmd = (quint8)m_data->at(pos);
    }
    else
    {
        int pos = m_packet->header->findDataPos(DATA_AVAKAR);
        if(pos < 0 || pos >= m_data->size())
            return false;
        cmd = quint8(m_data->at(pos)) >> 4;
    }
    return true;
}

bool analyzer_data::getLenFromHeader(quint32& len)
{
    if(m_packet->header->data_mask & DATA_LEN)
    {
        try
        {
            quint32 pos = m_packet->header->findDataPos(DATA_LEN);
            if(!m_data || pos >= m_data->size())
                return false;

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
    }
    else if(m_packet->header->data_mask & DATA_AVAKAR)
    {
        try
        {
            quint32 pos = m_packet->header->findDataPos(DATA_AVAKAR);
            if(m_data->size() <= pos)
                return false;
            len = (getUInt8(pos) & 0xF) + m_packet->header->len_offset;
        }
        catch(const char*)
        {
            return false;
        }
    }
    else
        return false;

    return true;
}

QString analyzer_data::getString(quint32 pos)
{
    QString str;
    if(pos >= (quint32)m_data->length())
        return str;
    for(; pos < (quint32)m_data->length() && m_data->at(pos) != '\0'; ++pos)
        str.append(QChar(m_data->at(pos)));
    return str;
}
