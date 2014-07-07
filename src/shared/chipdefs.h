/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef CHIPDEFS_H
#define CHIPDEFS_H

#include <QString>
#include <QHash>
#include <vector>

class chip_definition
{
public:

    struct memorydef
    {
        memorydef()
        {
            size = 0;
            pagesize = 0;
            memid = 0;
            start_addr = 0;
        }

        quint32 size;
        quint16 pagesize;
        quint8 memid;
        quint32 start_addr;
    };

    struct fuse
    {
        QString name;
        std::vector<int> bits;
        std::vector<int> values;
    };

    template <typename Iter>
    static int get_fuse_value(Iter first, Iter last, fuse const & f);

    template <typename Iter>
    static void set_fuse_value(Iter first, Iter last, fuse const & f, int value);

    static quint8 memNameToId(const QString& name);

    chip_definition();
    chip_definition(const QString& sign);

    void copy(chip_definition& cd);

    const QString& getName() { return m_name; }
    const QString& getSign() { return m_signature; }
    void setName(const QString& name) { m_name = name; }
    void setSign(const QString& sign) { m_signature = sign; }

    QHash<QString, memorydef> &getMems() { return m_memories; }
    std::vector<fuse> &getFuses() { return m_fuses; }
    QHash<QString, QString> &getOptions() { return m_options; }

    const memorydef *getMemDef(const QString& name) const;
    memorydef *getMemDef(const QString& name);
    const memorydef *getMemDef(quint8 memId) const;
    memorydef *getMemDef(quint8 memId);

    bool hasOption(const QString& name) const;
    QString getOption(const QString& name) const;
    quint32 getOptionUInt(const QString& name, bool *ok = NULL) const;
    qint32 getOptionInt(const QString& name, bool *ok = NULL) const;

private:
    QString m_name;
    QString m_signature;

    QHash<QString, memorydef> m_memories;
    QHash<QString, QString> m_options;

    std::vector<fuse> m_fuses;
};

template <typename Iter>
int chip_definition::get_fuse_value(Iter first, Iter last, fuse const & f)
{
    int fusevalue = 0;
    for (std::size_t j = f.bits.size(); j > 0; --j)
    {
        int bitno = f.bits[j-1];
        int byteno = bitno / 8;
        bitno %= 8;
        if (byteno >= last - first)
            continue;

        fusevalue = (fusevalue << 1) | !!(first[byteno] & (1<<bitno));
    }
    return fusevalue;
}

template <typename Iter>
void chip_definition::set_fuse_value(Iter first, Iter last, fuse const & f, int value)
{
    for (std::size_t j = 0; j != f.bits.size(); ++j)
    {
        int bitno = f.bits[j];
        int byteno = bitno / 8;
        bitno %= 8;

        if (byteno < last - first)
        {
            if (value & 1)
            {
                first[byteno] |= (1<<bitno);
            }
            else
            {
                first[byteno] &= ~(1<<bitno);
            }

            value >>= 1;
        }
    }
}

#endif // CHIPDEFS_H
