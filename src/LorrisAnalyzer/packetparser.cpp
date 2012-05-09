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

#include "storage.h"
#include "packetparser.h"
#include "packet.h"

PacketParser::PacketParser(Storage *storage, QObject *parent) :
    QObject(parent)
{
    m_storage = storage;
    m_curData = NULL;
    m_paused = false;
    m_packet = NULL;
}

PacketParser::~PacketParser()
{
    delete m_curData;
}

bool PacketParser::newData(const QByteArray &data, bool emitSig)
{
    if(m_paused || !m_packet)
        return false;

    if(!m_curData)
        m_curData = new analyzer_data(m_packet);

    if(!m_curData->getLenght())
        return false;

    char *d_start = (char*)data.data();
    char *d_itr = d_start;
    char *d_end = d_start + data.size();

    quint32 curRead = 1;

    while(d_itr != d_end)
    {
        if(m_curData->isFresh() || curRead == 0)
        {
            int index = data.indexOf(m_curData->getStaticData(), d_itr - d_start);
            if(index == -1)
                break;
            d_itr = d_start + index;
            m_curData->clear();
        }
        curRead = m_curData->addData(d_itr, d_end);
        d_itr += curRead;

        if(m_curData->isValid())
        {
            if(m_storage)
                m_storage->addData(m_curData);

            if(emitSig)
                emit packetReceived(m_curData, m_storage ? m_storage->getSize()-1 : 0);

            if(!m_storage)
                m_curData->clear();
            else
                m_curData = new analyzer_data(m_packet);
        }
    }
    return true;
}

void PacketParser::setPacket(analyzer_packet *packet)
{
    m_packet = packet;
    resetCurPacket();
}

void PacketParser::resetCurPacket()
{
    if(!m_packet)
    {
        delete m_curData;
        m_curData = NULL;
    }
    else if(m_curData)
    {
        m_curData->setPacket(m_packet);
        m_curData->clear();
    }
}
