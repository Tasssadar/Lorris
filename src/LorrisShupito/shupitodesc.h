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

#ifndef SHUPITODESC_H
#define SHUPITODESC_H

#include <QString>
#include <vector>
#include <map>
#include <QHash>

class ShupitoPacket;

class ShupitoDesc
{
public:
    struct config
    {
        QString guid;
        quint8 flags;
        quint8 cmd;
        quint8 cmd_count;
        std::vector<quint8> actseq;
        std::vector<quint8> data;

        bool always_active() const { return (flags & 1) != 0; }
        bool default_active() const { return (flags & 2) != 0; }

        ShupitoPacket getStateChangeCmd(bool activate);
    };

    typedef QHash<QString, config> intf_map;

    ShupitoDesc();

    void Clear();

    void AddData(const QByteArray &data);
    QString makeGuid(quint8 *data);
    void parseGroupConfig(quint8 *& first, quint8 *& last, quint8& base_cmd, std::vector<quint8>& actseq);
    void parseConfig(quint8 *& first, quint8 *& last, quint8& base_cmd, std::vector<quint8>& actseq);

    const QString& getGuid() { return m_guid; }
    config *getConfig(const QString& guid)
    {
        intf_map::iterator itr = m_interface_map.find(guid);
        if(itr == m_interface_map.end())
            return NULL;
        return &(*itr);
    }
    const intf_map& getInterfaceMap()
    {
        return m_interface_map;
    }

private:
    QString m_guid;
    QByteArray m_data;

    intf_map m_interface_map;
};

#endif // SHUPITODESC_H
