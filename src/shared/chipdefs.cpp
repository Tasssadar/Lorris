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

chip_definition::memorydef *chip_definition::getMemDef(quint8 memId)
{
    return getMemDef(memNames[memId]);
}

quint8 chip_definition::memNameToId(const QString& name)
{
    for(size_t i = 0; i < sizeof_array(memNames); ++i)
        if(name == memNames[i])
            return i;

    return MEM_NONE;
}

void chip_definition::copy(chip_definition &cd)
{
    m_name = cd.getName();
    m_memories = cd.getMems();

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
