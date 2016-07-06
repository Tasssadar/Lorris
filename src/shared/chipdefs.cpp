/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QString>
#include <QTextStream>
#include <QStringList>
#include <QFile>
#include <QDesktopServices>
#include <vector>

#include "../common.h"
#include "chipdefs.h"
#include "hexfile.h"

static const QString memNames[] = { "", "flash", "eeprom", "fuses", "sdram" };

chip_definition::chip_definition()
{
}

chip_definition::chip_definition(const QString &sign)
{
    m_signature = sign;
}

quint8 chip_definition::memNameToId(const QString& name)
{
    for(size_t i = 0; i < sizeof_array(memNames); ++i)
        if(name == memNames[i])
            return i;

    return MEM_NONE;
}

const QString& chip_definition::memIdToName(quint8 id) {
    if(id < sizeof_array(memNames))
        return memNames[id];
    return memNames[0];
}

void chip_definition::copy(chip_definition &cd)
{
    m_name = cd.m_name;
    m_memories = cd.m_memories;
    m_options = cd.m_options;

    for(quint32 x = 0; x < cd.getFuses().size(); ++x)
    {
        quint32 k;
        for(k = 0; k < m_fuses.size(); ++k)
        {
            if(m_fuses[k].name == cd.getFuses()[x].name)
                break;
        }

        if(k == m_fuses.size())
            m_fuses.push_back(cd.getFuses()[x]);
    }
}

const chip_definition::memorydef *chip_definition::getMemDef(const QString& name) const
{
    QHash<QString, memorydef>::const_iterator itr = m_memories.find(name);
    if(itr != m_memories.end())
        return &itr.value();
    return NULL;
}

chip_definition::memorydef *chip_definition::getMemDef(const QString& name)
{
    QHash<QString, memorydef>::iterator itr = m_memories.find(name);
    if(itr != m_memories.end())
        return &itr.value();
    return NULL;
}

const chip_definition::memorydef *chip_definition::getMemDef(quint8 memId) const
{
    return getMemDef(memNames[memId]);
}

chip_definition::memorydef *chip_definition::getMemDef(quint8 memId)
{
    return getMemDef(memNames[memId]);
}

bool chip_definition::hasOption(const QString& name) const
{
    return m_options.contains(name);
}

QString chip_definition::getOption(const QString& name) const
{
    QHash<QString, QString>::const_iterator itr = m_options.find(name);
    if(itr == m_options.end())
        return QString();
    return itr.value();
}

quint32 chip_definition::getOptionUInt(const QString& name, bool *ok) const
{
    QHash<QString, QString>::const_iterator itr = m_options.find(name);
    if(itr == m_options.end())
    {
        if(ok)
            *ok = false;
        return 0;
    }
    return itr.value().toUInt(ok, 0);
}

qint32 chip_definition::getOptionInt(const QString& name, bool *ok) const
{
    QHash<QString, QString>::const_iterator itr = m_options.find(name);
    if(itr == m_options.end())
    {
        if(ok)
            *ok = false;
        return 0;
    }
    return itr.value().toInt(ok, 0);
}
