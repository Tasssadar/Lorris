#include <QFile>
#include <stdlib.h>

#include "hexfile.h"
#include "deviceinfo.h"

HexFile::HexFile()
{
}

HexFile::~HexFile()
{
    deleteAllPages();
}

QString HexFile::load(QString fileName)
{
    QFile *file = new QFile(fileName);
    if(!file->open(QIODevice::ReadOnly))
    {
        delete file;
        return "Can't open file!";
    }

    QByteArray line;
    uint8_t lineLenght = 0;
    std::vector<uint8_t> rec_nums;
    char *nums = new char[3];
    nums[2] = ' ';
    uint16_t length;
    uint32_t address;
    uint8_t rectype;
    int16_t base = 0;

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
            return "Wrong line format.";
        }

        rec_nums.clear();
        for(uint8_t i = 1; i < lineLenght;)
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
            return "Invalid record lenght.";
        }

        if(rectype == 2)
        {
            if(length != 2)
            {
                file->close();
                delete file;
                delete[] nums;
                return "Invalid type 2 record.";
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
            return "Invalid record type.";
        }
        for (uint16_t i = 0; i < length; ++i)
        {
            if (base + address + i >= m_buffer.size())
                m_buffer.resize(base + address + i + 1, 0xFF);

            if(m_buffer[base + address + i] != 0xFF)
            {
                file->close();
                delete file;
                delete[] nums;
                return "Memory location was defined twice!";
            }
            m_buffer[base + address + i] = rec_nums[i + 4];
        }
    }
    delete[] nums;
    file->close();
    delete file;
    return "";
}

QString HexFile::makePages(DeviceInfo *info)
{
    deleteAllPages();
    m_pagesItr = 0;

    uint16_t size = m_buffer.size();
    if (size > info->mem_size)
        for (int a = info->mem_size; a < size; ++a)
            if (m_buffer[a] != 0xff)
                return "Program is too big!";

    uint16_t alt_entry_page = info->patch_pos / info->page_size;
    bool add_alt_page = info->patch_pos != 0;

    uint16_t i = 0;
    uint16_t pageItr = 0;
    Page *cur_page = NULL;
    uint16_t page_size = info->page_size;
    uint16_t stopGenerate = info->mem_size / info->page_size;

    for (bool generate = true; generate && i < stopGenerate; ++i)
    {
        cur_page = new Page();
        cur_page->data.resize(page_size);
        cur_page->address = i * page_size;
        pageItr = 0;
        if (size <= (i + 1) * page_size)
        {
            for (uint16_t y = 0; y < page_size; ++y)
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
            for (uint16_t y = i * page_size; y < (i + 1) * page_size; ++y)
            {
                cur_page->data[pageItr] = m_buffer[y];
                ++pageItr;
            }
        }

        if (!patch_page(cur_page, info->patch_pos, info->mem_size, pageItr))
            return "Failed patching page";
        pages.push_back(cur_page);

        if (i == alt_entry_page)
            add_alt_page = false;
    }
    if (add_alt_page)
    {
        for (uint16_t y = 0; y < page_size; ++y)
            cur_page->data[y] = 0xFF;
        cur_page->address = alt_entry_page * page_size;
        patch_page(cur_page, info->patch_pos, info->mem_size, pageItr);
        pages.push_back(cur_page);
    }
    return "";
}

bool HexFile::patch_page(Page *page,  uint16_t patch_pos, uint16_t boot_reset, uint16_t page_pos)
{
    if (patch_pos == 0)
        return true;

    if (page->address == 0)
    {
        uint16_t entrypt_jmp = (boot_reset / 2 - 1) | 0xc000;
        if((entrypt_jmp & 0xf000) != 0xc000)
            return false;
        page->data[0] = uint8_t(entrypt_jmp);
        page->data[1] = uint8_t(entrypt_jmp >> 8);
        return true;
    }

    if (page->address > patch_pos || page->address + page_pos <= patch_pos)
        return true;

    uint16_t new_patch_pos = patch_pos - page->address;

    if (page->data[new_patch_pos] != 0xFF || page->data[new_patch_pos + 1] != 0xFF)
       return false;

    uint16_t entrypt_jmp2 = m_buffer[0] | (m_buffer[1] << 8);
    if ((entrypt_jmp2 & 0xf000) != 0xc000)
        return false;

    uint16_t entry_addr = (entrypt_jmp2 & 0x0fff) + 1;
    entrypt_jmp2 = ((entry_addr - patch_pos / 2 - 1) & 0xfff) | 0xc000;
    page->data[new_patch_pos] = uint8_t(entrypt_jmp2);
    page->data[new_patch_pos + 1] = uint8_t(entrypt_jmp2 >> 8);
    return true;
}
