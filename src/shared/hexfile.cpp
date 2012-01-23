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

#include <QFile>
#include <QObject>

#include "hexfile.h"
#include "common.h"
#include "chipdefs.h"

// Most of this file is ported from avr232client, file program.hpp

void HexFile::Patcher::patchPage(page &p)
{
    Q_ASSERT(!p.data.empty());

    if(m_patch_pos == 0)
        return;

    if(p.address == 0)
    {
        m_entrypt_jmp = (m_boot_reset /2 - 1) | 0xC000;
        if((m_entrypt_jmp & 0xF000) != 0xC000)
            throw QString(QObject::tr("Cannot patch the program, it does not begin with rjmp instruction."));
        p.data[0] = (quint8)m_entrypt_jmp;
        p.data[1] = (quint8)(m_entrypt_jmp >> 8);
        return;
    }

    if(p.address > m_patch_pos || p.address + p.data.size() <= m_patch_pos)
        return;

    quint32 new_patch_pos = m_patch_pos - p.address;

    if(p.data[new_patch_pos] != 0xFF || p.data[new_patch_pos + 1] != 0xFF)
        throw QString(QObject::tr("The program is incompatible with this patching algorithm."));

    quint16 entry_addr = (m_entrypt_jmp & 0x0FFF) + 1;
    quint16 patched_instr = ((entry_addr - m_patch_pos / 2 - 1) & 0xFFF) | 0xC000;
    p.data[new_patch_pos] = (quint8)patched_instr;
    p.data[new_patch_pos + 1] = (quint8)(patched_instr >> 8);
}

HexFile::HexFile()
{
}

void HexFile::LoadFromFile(const QString &path)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly))
        throw QString(QObject::tr("Can't open file \"%1\"!")).arg(path);

    clear();

    int base = 0;
    std::vector<quint8> rec_nums;
    bool ok;

    QByteArray line = file.readLine();
    for(int lineno = 0; !line.isNull(); line = file.readLine(), ++lineno)
    {
        line = line.trimmed();
        if(line.isEmpty())
            continue;

        if(line[0] != ':' || line.size()%2 != 1)
            throw QString(QObject::tr("Invalid line format (line %1)")).arg(lineno);

        rec_nums.clear();
        quint8 checksum = 0;
        for(quint8 i = 1; true;)
        {
            quint8 num = line.mid(i, 2).toInt(&ok, 16);
            if(!ok)
                throw QString(QObject::tr("Failed to parse hex num (line %1)")).arg(lineno);

            rec_nums.push_back(num);

            i += 2;

            if(i < line.length())
                checksum += num;
            else
                break;
        }
        checksum = 256 - checksum;

        if(checksum != rec_nums[rec_nums.size()-1])
            throw QString(QObject::tr("Checksums does not match (line %1)")).arg(lineno);

        int length = rec_nums[0];
        int address = rec_nums[1] * 0x100 + rec_nums[2];
        int rectype = rec_nums[3];

        if (length != (int)rec_nums.size() - 5)
            throw QString(QObject::tr("Invalid record lenght specified (line %1)")).arg(lineno);

        if (rectype == 4)
        {
            if (length != 2)
                throw QString(QObject::tr("Invalid type 4 record (line %1)")).arg(lineno);
            base = (rec_nums[4] * 0x100 + rec_nums[5]) << 16;
            continue;
        }

        if (rectype == 3)
            continue;

        if (rectype == 2)
        {
            if (length != 2)
                throw QString(QObject::tr("Invalid type 2 record (line %1)")).arg(lineno);
            base = (rec_nums[4] * 0x100 + rec_nums[5]) * 16;
            continue;
        }

        if (rectype == 1)
            break;

        if (rectype != 0)
            throw QString(QObject::tr("Invalid record type (line %1)")).arg(lineno);

        addRegion(base + address, rec_nums.data() + 4, rec_nums.data() + rec_nums.size() - 1, lineno);
    }
}

//void add_region(std::size_t pos, byte_type const * first, byte_type const * last, int lineno)
//program.hpp
void HexFile::addRegion(quint32 pos, quint8 const * first, quint8 const * last, int lineno)
{
    regionMap::iterator itr = m_data.upper_bound(pos);
    if(itr != m_data.begin())
    {
        regionMap::iterator itr2 = itr;
        --itr2;

        if(itr2->first + itr2->second.size() == pos)
        {
            itr2->second.insert(itr2->second.end(), first, last);
            return;
        }

        if(itr2->first + itr2->second.size() > pos)
            throw QString(QObject::tr("Memory location was defined twice (line %1)")).arg(lineno);
    }

    if(itr != m_data.end() && itr->first < pos + (last - first))
        throw QString(QObject::tr("Memory location was defined twice (line %1)")).arg(lineno);

    m_data[pos] = std::vector<quint8>(first, last);
}

void HexFile::SaveToFile(const QString &path)
{
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly))
        throw QString(QObject::tr("Can't open file \"%1\"!")).arg(path);

    quint32 base = 0;
    for(regionMap::iterator itr = m_data.begin(); itr != m_data.end(); ++itr)
    {
        quint32 offset = itr->first;
        quint32 address = offset;
        std::vector<quint8>& data = itr->second;

        if((base & 0xFFFF0000) != (offset & 0xFFFF0000))
        {
            writeExtAddrLine(&file, offset);
            base = offset;
        }

        quint8 write = 0;
        for(quint32 i = 0; i != data.size(); i += write)
        {
            QString line(":");
            write = (data.size() - i >= 0x10) ? 0x10 : data.size() - i;

            line += Utils::hexToString(write);       // recordn len
            line += Utils::hexToString(address >> 8); // address
            line += Utils::hexToString(address);
            line += "00";                            // record type

            quint8 checksum = write + (quint8)(address >> 8) + (quint8)address;
            for(quint8 x = 0; x < write; ++x)
            {
                line += Utils::hexToString(data[i+x]);
                checksum += data[i+x];
            }
            line += Utils::hexToString(0x100 - checksum);
            line += "\r\n";

            address += write;

            file.write(line.toAscii());
        }
    }

    static const QString endFile = ":00000001FF\r\n";
    file.write(endFile.toAscii());
    file.close();
}

void HexFile::writeExtAddrLine(QFile *file, quint32 addr)
{
    QString line = ":02" "00" "00" "04";
    line += Utils::hexToString(addr >> 24);
    line += Utils::hexToString(addr >> 16);

    quint8 checksum = 0x100 - (quint8)(0x02 + 0x04 + (quint8)(addr >> 24) + (quint8)(addr >> 16));
    line += Utils::hexToString(checksum);

    line += "\r\n";
    file->write(line.toAscii());
}

void HexFile::setData(const QByteArray &data)
{
    clear();

    quint32 base = 0;
    quint32 size = data.size();
    quint32 itr = 0;
    quint32 maxPerBase = 0xFFFF;

    std::vector<quint8> bytes;

    do
    {
        bytes.clear();

        for(;itr < size && itr < maxPerBase; ++itr)
            bytes.push_back(data[itr]);

        m_data[base] = bytes;
        base = itr;
        maxPerBase += 0xFFFF;
    }while((base & 0xFFFF0000) != (size & 0xFFFF0000));
}

QByteArray HexFile::getDataArray(quint32 len)
{
    QByteArray res(len, 0xFF);
    for(regionMap::iterator itr = m_data.begin(); itr != m_data.end(); ++itr)
    {
        quint32 offset = itr->first;
        std::vector<quint8>& data = itr->second;

        for(quint32 i = 0; i < data.size(); ++i)
        {
            if(offset + i > (quint32)res.size())
                return res;
            res[offset+i] = data[i];
        }
    }
    return res;
}

//template <typename OutputIterator>
//void make_pages(memory const & memory, std::string const & memid, chip_definition const & chip, OutputIterator out)
//program.hpp
void HexFile::makePages(std::vector<page> &pages, quint8 memId, chip_definition &chip, std::set<quint32> *skipPages)
{
    chip_definition::memorydef const * memdef = chip.getMemDef(memId);
    if(!memdef)
        throw QString(QObject::tr("This chip does not have memory type %1")).arg(memId);

    if(getTopAddress() > memdef->size)
        throw QString(QObject::tr("Program is too large."));

    if(memdef->pagesize == 0)
    {
        // The memory is unpaged.
        for(regionMap::iterator itr = m_data.begin(); itr != m_data.end(); ++itr)
        {
            page cur_page;
            cur_page.address = itr->first;
            cur_page.data = itr->second;
            pages.push_back(cur_page);
        }
    }
    else
    {
        page cur_page;
        cur_page.data.resize(memdef->pagesize);

        QString patch_pos_str = (memId == MEM_FLASH) ? chip.getOption("avr232boot_patch") : "";
        quint32 patch_pos = patch_pos_str.isEmpty() ? 0 : patch_pos_str.toInt();

        quint32 alt_entry_page = patch_pos / memdef->pagesize;
        bool add_alt_page = patch_pos != 0;

        Patcher patcher(patch_pos, memdef->size);

        for(quint32 i = 0; i < memdef->size / memdef->pagesize; ++i)
        {
            cur_page.address = i * memdef->pagesize;
            if(!intersects(cur_page.address, memdef->pagesize))
                continue;

            std::fill(cur_page.data.begin(), cur_page.data.end(), 0xFF);
            getRange(cur_page.address, memdef->pagesize, cur_page.data);

            patcher.patchPage(cur_page);
            pages.push_back(cur_page);

            if(i == alt_entry_page)
                add_alt_page = false;
        }

        if(add_alt_page)
        {
            cur_page.address = alt_entry_page * memdef->pagesize;
            std::fill(cur_page.data.begin(), cur_page.data.end(), 0xFF);
            patcher.patchPage(cur_page);
            pages.push_back(cur_page);
        }
    }

    if(skipPages)
    {
        for(quint32 i = 0; i < pages.size(); ++i)
        {
            bool skip = true;
            for(quint32 x = 0; skip && x < pages[i].data.size(); ++x)
                if(pages[i].data[x] != 0xFF)
                    skip = false;

            if(skip)
                skipPages->insert(i);
        }
    }
}

bool HexFile::intersects(quint32 address, quint32 length)
{
    regionMap::iterator itr,prior;
    itr = prior = m_data.upper_bound(address);
    --prior;

    bool res = false;

    if(itr != m_data.begin() && prior->first + prior->second.size() > address)
        res = true;
    else if(itr != m_data.end() && itr->first < address + length)
        res = true;

    return res;
}

void HexFile::getRange(quint32 address, quint32 length, std::vector<quint8> & out)
{
    regionMap::iterator itr = m_data.upper_bound(address);
    if(itr != m_data.begin())
        --itr;

    for(; itr != m_data.end() && itr->first < address + length; ++itr)
    {
        quint32 start = std::max(address, itr->first);
        quint32 stop = std::min(address + length, (quint32)(itr->first + itr->second.size()));

        if(start >= stop)
            continue;

        std::copy(itr->second.data() + (start - itr->first),
                  itr->second.data() + (stop - itr->first),
                  out.data() + (start - address));
    }
}
