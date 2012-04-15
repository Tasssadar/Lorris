/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

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
