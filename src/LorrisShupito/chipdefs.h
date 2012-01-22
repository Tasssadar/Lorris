#ifndef CHIPDEFS_H
#define CHIPDEFS_H

#include <QString>
#include <map>
#include <vector>

class chip_definition
{
public:

    struct memorydef
    {
        quint32 size;
        quint16 pagesize;
        quint8 memid;
    };

    struct fuse
    {
        QString name;
        std::vector<int> bits;
        std::vector<int> values;
    };

    static void parse_chipdefs(QString const & strdefs, std::vector<chip_definition> & res);
    static void update_chipdef(std::vector<chip_definition> &templates, chip_definition & cd);
    static void parse_default_chipsets(std::vector<chip_definition> & res);

    template <typename Iter>
    static int get_fuse_value(Iter first, Iter last, fuse const & f);

    template <typename Iter>
    static void set_fuse_value(Iter first, Iter last, fuse const & f, int value);

    chip_definition();

    const QString& getName() { return m_name; }
    const QString& getSign() { return m_signature; }
    void setName(const QString& name) { m_name = name; }
    void setSign(const QString& sign) { m_signature = sign; }

    std::map<QString, memorydef> &getMems() { return m_memories; }
    std::vector<fuse> &getFuses() { return m_fuses; }
    std::map<QString, QString> &getOptions() { return m_options; }

    memorydef *getMemDef(const QString& name)
    {
        std::map<QString, memorydef>::iterator itr = m_memories.find(name);
        if(itr != m_memories.end())
            return &itr->second;
        return NULL;
    }

private:
    QString m_name;
    QString m_signature;

    std::map<QString, memorydef> m_memories;
    std::map<QString, QString> m_options;

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
