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

chip_definition::chip_definition()
{
}

chip_definition::chip_definition(const QString &sign)
{
    m_signature = sign;
}

chip_definition::memorydef *chip_definition::getMemDef(quint8 memId)
{
    static const QString memNames[] = { "", "flash", "eeprom", "fuses", "sdram" };
    return getMemDef(memNames[memId]);
}
