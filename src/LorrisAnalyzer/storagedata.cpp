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
    if(limit == m_packet_limit)
        return;

    if(!m_data.empty())
    {
        size_t itr;
        std::vector<QByteArray> vec;
        vec.reserve((std::min)(m_data.size(), (size_t)limit));

        if(m_offset < limit)
        {
            itr = m_data.size() - (std::min)(m_data.size()-m_offset, size_t(limit - m_offset));
            vec.insert(vec.end(), m_data.begin()+itr, m_data.end());
        }

        itr = (std::max)(0, m_offset-limit);
        vec.insert(vec.end(), m_data.begin()+itr, m_data.begin()+m_offset);

        m_data.swap(vec);
    }

    m_packet_limit = limit;
    m_offset = 0;
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

