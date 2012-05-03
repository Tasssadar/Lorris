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

#ifndef PACKETPARSER_H
#define PACKETPARSER_H

#include <QObject>
#include <QByteArray>

class analyzer_data;
class analyzer_packet;
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
