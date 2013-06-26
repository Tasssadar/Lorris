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

        quint32 end = (std::min)(m_data.size(), (size_t)m_offset+limit);
        vec.insert(vec.end(), m_data.begin()+m_offset, m_data.begin()+end);
        if((quint32)m_offset+limit >= m_data.size())
        {
            end = (m_offset + limit) - m_data.size() - 1;
            vec.insert(vec.end(), m_data.begin(), m_data.begin()+end);
        }
        m_data.swap(vec);
        m_offset = 0;
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
    QByteArray *res = NULL;
    if(m_data.size() < (quint32)m_packet_limit)
    {
        m_data.push_back(data);
        res = &m_data.back();
    }
    else
    {
        if((quint32)m_offset >= m_data.size())
            m_offset = 0;

        m_data[m_offset] = data;
        res = &m_data[m_offset];
        ++m_offset;
    }

    // Data in this class will never be modified,
    // so release unused memory
    res->squeeze();
    return res;
}

