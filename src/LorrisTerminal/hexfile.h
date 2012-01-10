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

#ifndef HEXFILE_H
#define HEXFILE_H

#include <QObject>
#include <vector>

class QString;
class DeviceInfo;

struct Page
{
    quint16 address;
    std::vector<quint8> data;
};

class HexFile
{
public:
    HexFile();
    ~HexFile();

    QString load(QString file);
    QString makePages(DeviceInfo *info);

    quint16 getPagesCount() { return pages.size(); }
    Page *getNextPage()
    {
        if(m_pagesItr >= pages.size())
            return NULL;
        return pages[m_pagesItr++];
    }

    static void deleteAllPages(std::vector<Page*> pages)
    {
        for(quint16 i = 0; i < pages.size(); ++i)
            delete pages[i];
        pages.clear();
    }

private:
    bool patch_page(Page *page,  quint16 patch_pos, quint16 boot_reset, quint16 page_pos);

    std::vector<quint8> m_buffer;
    std::vector<Page*> pages;
    quint16 m_pagesItr;
};

#endif // HEXFILE_H
