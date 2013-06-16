/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "storagedata.h"

StorageData::StorageData()
{
    m_packet_limit = INT_MAX;
    m_offset = 0;
}

StorageData::~StorageData()
{
    clear();
}

void StorageData::clear()
{
    m_data.clear();
    m_offset = 0;
}

void StorageData::setPacketLimit(int limit)
{
    m_packet_limit = limit;
    if(m_data.size() > (quint32)limit)
    {
        std::vector<QByteArray> vec;
        vec.insert(vec.begin(), m_data.end()-limit, m_data.end());
        m_data.swap(vec);
    }
}

QByteArray& StorageData::operator[](quint32 idx)
{
    idx += m_offset;
    if(idx >= m_data.size())
        idx -= m_data.size();
    return m_data[idx];
}

QByteArray *StorageData::push_back(const QByteArray& data)
{
    if(m_data.size() < (quint32)m_packet_limit)
    {
        m_data.push_back(data);
        return &m_data.back();
    }
    else
    {
        if((quint32)m_offset >= m_data.size())
            m_offset = 0;
        ++m_offset;
        m_data[m_offset-1] = data;
        return &m_data[m_offset-1];
    }
}

