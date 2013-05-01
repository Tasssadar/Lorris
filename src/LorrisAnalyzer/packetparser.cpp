/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "storage.h"
#include "packetparser.h"
#include "packet.h"

PacketParser::PacketParser(Storage *storage, QObject *parent) :
    QObject(parent), m_curData(&m_curByteArray), m_emitSigData(NULL)
{
    m_storage = storage;
    m_paused = false;
    m_packet = NULL;
    m_packetItr = 0;
}

PacketParser::~PacketParser()
{
    m_import.close();
}

bool PacketParser::newData(const QByteArray &data, bool emitSig)
{
    if(m_paused || !m_packet)
        return false;

    if(!m_curData.getLenght())
        return false;

    char *d_start = (char*)data.data();
    char *d_itr = d_start;
    char *d_end = d_start + data.size();

    quint32 curRead = 1;

    while(d_itr != d_end)
    {
        if(m_packetItr == 0 || curRead == 0)
        {
            int index = data.indexOf(m_packet->getStaticData(), d_itr - d_start);
            if(index == -1)
                break;
            d_itr = d_start + index;
            m_curData.clear();
            m_packetItr = 0;
        }
        curRead = m_curData.addData(d_itr, d_end, m_packetItr);
        d_itr += curRead;

        if(m_curData.isValid(m_packetItr))
        {
            if(m_storage)
            {
                QByteArray *b = m_storage->addData(m_curData.getData());
                m_emitSigData.setData(b);
            }
            else
                m_emitSigData.setData(m_curData.getDataPtr());

            if(emitSig)
                emit packetReceived(&m_emitSigData, m_storage ? m_storage->getSize()-1 : 0);

            m_curData.clear();
            m_packetItr = 0;
        }
    }
    return true;
}

void PacketParser::setPacket(analyzer_packet *packet)
{
    m_packet = packet;
    m_curData.setPacket(packet);
    m_emitSigData.setPacket(packet);
    resetCurPacket();
}

void PacketParser::resetCurPacket()
{
    if(m_packet)
    {
        m_curData.clear();
        m_packetItr = 0;
        tryImport();
    }
}

void PacketParser::setImport(const QString& filename)
{
    m_import.close();
    m_import.setFileName(filename);
    m_import.open(QIODevice::ReadOnly);
}

void PacketParser::tryImport()
{
    if(!m_packet || !m_import.isOpen())
        return;

    m_import.seek(0);

    bool fromheader = false;
    int len = m_curData.getLenght(&fromheader);

    newData(m_import.read(len));

    if(m_packet->header->hasLen() && !fromheader && m_import.size() >= len)
    {
        int total = m_curData.getLenght(&fromheader);
        if(fromheader)
            newData(m_import.read(total - len));
    }
}
