/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef STORAGEDATA_H
#define STORAGEDATA_H

#include <vector>
#include <QByteArray>

class StorageData
{
public:
    StorageData();
    ~StorageData();

    void clear();
    inline bool empty() const { return m_data.empty(); }
    inline quint32 size() const { return m_data.size(); }

    int getPacketLimit() const { return m_packet_limit; }
    void setPacketLimit(int limit);

    QByteArray& operator [](quint32 idx);
    QByteArray *push_back(const QByteArray& data);

private:
    std::vector<QByteArray> m_data;
    int m_packet_limit;
    int m_offset;
};

#endif // STORAGEDATA_H
