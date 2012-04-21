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

#include "shupitopacket.h"

ShupitoPacket::ShupitoPacket(quint8 cmd, quint8 size, ...)
{
    itr = 0;

    m_data = QByteArray(size + 2, 0);
    m_data[itr++] = 0x80;
    m_data[itr++] = size | (cmd << 4);

    va_list args;
    va_start(args, size);
    for (quint8 i = 0; i < size; ++i)
        m_data[itr++] = (quint8)va_arg(args, int);
    va_end(args);
}

ShupitoPacket::ShupitoPacket()
{
    Clear();
}

void ShupitoPacket::set(bool resize, quint8 cmd, quint8 size)
{
    itr = 0;

    int target_size = resize ? size + 2 : 2;
    m_data = QByteArray(target_size, 0);

    m_data[itr++] = 0x80;
    m_data[itr++] = size | (cmd << 4);
}

void ShupitoPacket::Clear()
{
    m_data.clear();
    itr = 0;
}

bool ShupitoPacket::isValid()
{
    if(itr < 2 || (quint8)m_data.at(0) != 0x80)
        return false;

    quint8 size = getLen();
    if((itr - 2) != size)
        return false;
    return true;
}

quint8 ShupitoPacket::addData(char *d_first, char *d_end)
{
    char * d_cur = d_first;

    if(itr == 0)
    {
        if(d_cur != d_end && quint8(*d_cur++) == 0x80)
            m_data[itr++] = 0x80;
        else
            return 0;
    }

    Q_ASSERT(itr >= 1);
    while (d_cur != d_end
        && (itr == 1 || itr < getLen() + 2))
    {
        m_data[itr++] = *d_cur++;
    }

    return d_cur - d_first;
}
