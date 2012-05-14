/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef PACKETPARSER_H
#define PACKETPARSER_H

#include <QObject>
#include <QByteArray>

struct analyzer_packet;
class analyzer_data;
class Storage;

class PacketParser : public QObject
{
    Q_OBJECT
Q_SIGNALS:
    void packetReceived(analyzer_data *data, quint32 index);

public:
    explicit PacketParser(Storage *storage, QObject *parent = 0);
    ~PacketParser();

    void setPaused(bool pause)
    {
        m_paused = pause;
    }

    void setPacket(analyzer_packet *packet);
    
public slots:
    bool newData(const QByteArray& data, bool emitSig = true);
    void resetCurPacket();

private:
    bool m_paused;
    analyzer_data *m_curData;
    analyzer_packet *m_packet;
    Storage *m_storage;
};

#endif // PACKETPARSER_H
