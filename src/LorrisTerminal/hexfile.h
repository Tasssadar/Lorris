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

private:
    void deleteAllPages()
    {
        for(quint16 i = 0; i < pages.size(); ++i)
            delete pages[i];
        pages.clear();
    }

    bool patch_page(Page *page,  quint16 patch_pos, quint16 boot_reset, quint16 page_pos);

    std::vector<quint8> m_buffer;
    std::vector<Page*> pages;
    quint16 m_pagesItr;
};

#endif // HEXFILE_H
