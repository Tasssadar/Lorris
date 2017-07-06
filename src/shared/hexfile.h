/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

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
    MEM_NONE    = 0,

    MEM_FLASH   = 1,
    MEM_EEPROM  = 2,
    MEM_FUSES   = 3,
    MEM_SDRAM   = 4,
    MEM_JTAG    = 5,

    MEM_COUNT   = 6
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
    void LoadFromBin(const QString& path);
    void DecodeFromString(const QByteArray& hex);
    void SaveToFile(const QString& path);
    QList<QByteArray> SaveToArray();

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
    void getRange(quint32 address, quint32 length, quint8 * out);

    QString getFilePath() const { return m_filepath; }
    void setFilePath(QString path) { m_filepath = path; }

private:
    QByteArray getExtAddrLine(quint32 addr);

    regionMap m_data;
    QString m_filepath;
};

#endif // HEXFILE_H
