#ifndef HEXFILE_H
#define HEXFILE_H

#include <QObject>
#include <vector>

class QString;
class DeviceInfo;

struct Page
{
    uint16_t address;
    std::vector<uint8_t> data;
};

class HexFile
{
public:
    HexFile();
    ~HexFile();

    QString load(QString file);
    QString makePages(DeviceInfo *info);

    uint16_t getPagesCount() { return pages.size(); }
    Page *getNextPage()
    {
        if(m_pagesItr >= pages.size())
            return NULL;
        return pages[m_pagesItr++];
    }

private:
    void deleteAllPages()
    {
        for(uint16_t i = 0; i < pages.size(); ++i)
            delete pages[i];
        pages.clear();
    }

    bool patch_page(Page *page,  uint16_t patch_pos, uint16_t boot_reset, uint16_t page_pos);

    std::vector<uint8_t> m_buffer;
    std::vector<Page*> pages;
    uint16_t m_pagesItr;
};

#endif // HEXFILE_H
