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

#ifndef EEPROM_H
#define EEPROM_H

#include <QByteArray>
#include <QWidget>

#include "deviceinfo.h"
#include "hexfile.h"

class EEPROM
{
public:
    EEPROM(QWidget *parent, DeviceInfo *info);
    ~EEPROM();

    void AddData(QByteArray newData)
    {
        data.append(newData);
    }

    quint16 GetEEPROMSize()
    {
        return m_deviceInfo->eeprom_size;
    }

    void Export();
    bool Import();
    Page *getNextPage()
    {
        if(pageItr >= pages.size() || pages[pageItr]->address >= m_deviceInfo->eeprom_size)
            return NULL;
        return pages[pageItr++];
    }

private:
    QByteArray data;
    DeviceInfo *m_deviceInfo;
    QWidget *m_parent;
    quint16 pageItr;
    std::vector<Page*> pages;
};

#endif // EEPROM_H
