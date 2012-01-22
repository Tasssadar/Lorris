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
#include <stdlib.h>

#include "hexfileterminal.h"
#include "deviceinfo.h"

HexFileTerminal::HexFileTerminal()
{
}

HexFileTerminal::~HexFileTerminal()
{
    HexFileTerminal::deleteAllPages(pages);
}

QString HexFileTerminal::load(QString fileName)
{
    QFile *file = new QFile(fileName);
    if(!file->open(QIODevice::ReadOnly))
    {
        delete file;
        return QObject::tr("Can't open file!");
    }

    QByteArray line;
    quint8 lineLenght = 0;
    std::vector<quint8> rec_nums;
    char *nums = new char[3];
    nums[2] = ' ';
    quint16 length;
    quint32 address;
    quint8 rectype;
    qint16 base = 0;

    m_buffer.clear();

    while(true)
    {
        line = file->readLine();

        lineLenght = line.size();
        if(!lineLenght)
            break;

        // remove \r\n
        lineLenght -= 2;
        line = line.left(lineLenght);

        if (line[0] != ':' || lineLenght % 2 != 1)
        {
            file->close();
            delete file;
            delete[] nums;
            return QObject::tr("Wrong line format.");
        }

        rec_nums.clear();
        for(quint8 i = 1; i < lineLenght;)
        {
            nums[0] = line[i++];
            nums[1] = line[i++];
            rec_nums.push_back(strtol(nums, NULL, 16));
        }

        length = rec_nums[0];
        address = rec_nums[1] * 0x100 + rec_nums[2];
        rectype = rec_nums[3];

        if (length != rec_nums.size() - 5)
        {
            file->close();
            delete file;
            delete[] nums;
            return QObject::tr("Invalid record lenght.");
        }

        if(rectype == 2)
        {
            if(length != 2)
            {
                file->close();
                delete file;
                delete[] nums;
                return QObject::tr("Invalid type 2 record.");
                base = (rec_nums[4] * 0x100 + rec_nums[5]) * 16;
                continue;
            }
        }

        if(rectype == 1)
            break;

        if (rectype != 0)
        {
            file->close();
            delete file;
            delete[] nums;
            return QObject::tr("Invalid record type.");
        }

        for (quint16 i = 0; i < length; ++i)
        {
            if (base + address + i >= m_buffer.size())
                m_buffer.resize(base + address + i + 1, 0xFF);

            if(m_buffer[base + address + i] != 0xFF)
            {
                file->close();
                delete file;
                delete[] nums;
                return QObject::tr("Memory location was defined twice!");
            }
            m_buffer[base + address + i] = rec_nums[i + 4];
        }
    }
    delete[] nums;
    file->close();
    delete file;
    return "";
}

QString HexFileTerminal::makePages(DeviceInfo *info)
{
    HexFileTerminal::deleteAllPages(pages);
    m_pagesItr = 0;

    quint16 size = m_buffer.size();
    if (size > info->mem_size)
        for (int a = info->mem_size; a < size; ++a)
            if (m_buffer[a] != 0xff)
                return QObject::tr("Program is too big!");

    quint16 alt_entry_page = info->patch_pos / info->page_size;
    bool add_alt_page = info->patch_pos != 0;

    quint16 i = 0;
    quint16 pageItr = 0;
    Page *cur_page = NULL;
    quint16 page_size = info->page_size;
    quint16 stopGenerate = info->mem_size / info->page_size;

    for (bool generate = true; generate && i < stopGenerate; ++i)
    {
        cur_page = new Page();
        cur_page->data.resize(page_size);
        cur_page->address = i * page_size;
        pageItr = 0;
        if (size <= (i + 1) * page_size)
        {
            for (quint16 y = 0; y < page_size; ++y)
            {
                if (i * page_size + y < size)
                    cur_page->data[pageItr] = m_buffer[i * page_size + y];
                else
                    cur_page->data[pageItr] = 0xFF;
                ++pageItr;
            }
            generate = false;
        }
        else
        {
            for (quint16 y = i * page_size; y < (i + 1) * page_size; ++y)
            {
                cur_page->data[pageItr] = m_buffer[y];
                ++pageItr;
            }
        }

        if (!patch_page(cur_page, info->patch_pos, info->mem_size, pageItr))
            return QObject::tr("Failed patching page");
        pages.push_back(cur_page);

        if (i == alt_entry_page)
            add_alt_page = false;
    }
    if (add_alt_page)
    {
        for (quint16 y = 0; y < page_size; ++y)
            cur_page->data[y] = 0xFF;
        cur_page->address = alt_entry_page * page_size;
        patch_page(cur_page, info->patch_pos, info->mem_size, pageItr);
        pages.push_back(cur_page);
    }
    return "";
}

bool HexFileTerminal::patch_page(Page *page,  quint16 patch_pos, quint16 boot_reset, quint16 page_pos)
{
    if (patch_pos == 0)
        return true;

    if (page->address == 0)
    {
        quint16 entrypt_jmp = (boot_reset / 2 - 1) | 0xc000;
        if((entrypt_jmp & 0xf000) != 0xc000)
            return false;
        page->data[0] = quint8(entrypt_jmp);
        page->data[1] = quint8(entrypt_jmp >> 8);
        return true;
    }

    if (page->address > patch_pos || page->address + page_pos <= patch_pos)
        return true;

    quint16 new_patch_pos = patch_pos - page->address;

    if (page->data[new_patch_pos] != 0xFF || page->data[new_patch_pos + 1] != 0xFF)
       return false;

    quint16 entrypt_jmp2 = m_buffer[0] | (m_buffer[1] << 8);
    if ((entrypt_jmp2 & 0xf000) != 0xc000)
        return false;

    quint16 entry_addr = (entrypt_jmp2 & 0x0fff) + 1;
    entrypt_jmp2 = ((entry_addr - patch_pos / 2 - 1) & 0xfff) | 0xc000;
    page->data[new_patch_pos] = quint8(entrypt_jmp2);
    page->data[new_patch_pos + 1] = quint8(entrypt_jmp2 >> 8);
    return true;
}
