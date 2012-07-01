/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SHUPITODESC_H
#define SHUPITODESC_H

#include <QString>
#include <vector>
#include <map>
#include <QHash>

#include "shupitopacket.h"

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
