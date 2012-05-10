/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef EEPROM_H
#define EEPROM_H

#include <QByteArray>
#include <QWidget>

#include "../shared/chipdefs.h"
#include "../shared/hexfile.h"

class EEPROM
{
public:
    EEPROM(QWidget *parent, chip_definition& chip);
    ~EEPROM();

    void AddData(QByteArray newData)
    {
        data.append(newData);
    }

    quint16 GetEEPROMSize()
    {
        chip_definition::memorydef *memdef = m_chip.getMemDef(MEM_EEPROM);
        if(!memdef)
            return 0;
        return memdef->size;
    }

    void Export();
    bool Import();

    page* getNextPage()
    {
        if(pageItr >= pages.size() || pages[pageItr].address >= GetEEPROMSize())
            return NULL;
        return &pages[pageItr++];
    }

private:
    QByteArray data;
    chip_definition m_chip;
    QWidget *m_parent;
    quint16 pageItr;
    std::vector<page> pages;
};

#endif // EEPROM_H
