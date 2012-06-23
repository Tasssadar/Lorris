/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SHUPITOPACKET_H
#define SHUPITOPACKET_H

#include <QByteArray>

class ShupitoPacket
{
public:
    ShupitoPacket(quint8 cmd, quint8 size, ...);
    ShupitoPacket();

    void set(bool resize, quint8 cmd, quint8 size);

    QByteArray getData(bool onlyPacketData = true)
    {
        if(onlyPacketData)
            return m_data.right(m_data.length() - 2);
        else
            return m_data;
    }

    quint8 a(quint32 pos, bool onlyPacketData = true)
    {
        return (quint8)m_data[pos + (onlyPacketData ? 2 : 0)];
    }

    quint8 getOpcode() { return (quint8(m_data[1]) >> 4); }
    quint8 getLen() { return (quint8(m_data[1]) & 0x0F); }

    void Clear();
    bool isValid();
    bool isFresh() const { return itr == 0; }

    quint8 addData(char *d_itr, char *d_end);

    // Returns only data part!
    quint8 operator[](int i) const
    {
        if(i+2 >= m_data.size())
            return 0;
        return (quint8)m_data[i+2];
    }

    QByteArray &getDataRef() { return m_data; }

private:
    QByteArray m_data;
    quint8 itr;
};

#endif // SHUPITOPACKET_H
