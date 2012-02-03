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

#include <QTypeInfo>
#include <map>
#include <vector>
#include <set>

class QFile;
class chip_definition;

enum MemoryTypes
{
    MEM_FLASH   = 1,
    MEM_EEPROM  = 2,
    MEM_FUSES   = 3,
    MEM_SDRAM   = 4,
    MEM_COUNT   = 5
};

struct page
{
    quint32 address;
    std::vector<quint8> data;
};

class HexFile
{
public:
    typedef std::map<quint32, std::vector<quint8> > regionMap;

    class Patcher
    {
    public:
        Patcher(quint32 patch_pos, quint32 boot_reset)
        {
            m_patch_pos = patch_pos;
            m_boot_reset = boot_reset;
            m_entrypt_jmp = 0;
        }

        void patchPage(page& p);

    private:
        quint16 m_entrypt_jmp;
        quint32 m_patch_pos;
        quint32 m_boot_reset;
    };

    HexFile();

    void clear()
    {
        m_data.clear();
    }

    void LoadFromFile(const QString& path);
    void SaveToFile(const QString& path);

    void addRegion(quint32 pos, quint8 const * first, quint8 const * last, int lineno);

    regionMap& getData() { return m_data; }
    void setData(const QByteArray& data);
    QByteArray getDataArray(quint32 len);

    quint32 getTopAddress()
    {
        if(m_data.empty())
            return 0;
        regionMap::iterator last = m_data.end();
        --last;
        return last->first + last->second.size();
    }

    std::vector<quint8>& operator[](quint32 i)
    {
        return m_data[i];
    }

    void makePages(std::vector<page>& pages, quint8 memId, chip_definition& chip, std::set<quint32> *skipPages);
    bool intersects(quint32 address, quint32 length);
    void getRange(quint32 address, quint32 length, std::vector<quint8> & out);

private:
    void writeExtAddrLine(QFile *file, quint32 addr);

    regionMap m_data;
};

#endif // HEXFILE_H
