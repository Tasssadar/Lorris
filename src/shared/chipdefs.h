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
    chip_definition(const QString& sign);

    const QString& getName() { return m_name; }
    const QString& getSign() { return m_signature; }
    void setName(const QString& name) { m_name = name; }
    void setSign(const QString& sign) { m_signature = sign; }

    QHash<QString, memorydef> &getMems() { return m_memories; }
    std::vector<fuse> &getFuses() { return m_fuses; }
    QHash<QString, QString> &getOptions() { return m_options; }

    memorydef *getMemDef(const QString& name)
    {
        QHash<QString, memorydef>::iterator itr = m_memories.find(name);
        if(itr != m_memories.end())
            return &itr.value();
        return NULL;
    }

    memorydef *getMemDef(quint8 memId);

    QString getOption(const QString& name)
    {
        QString res;
        QHash<QString, QString>::iterator itr = m_options.find(name);
        if(itr != m_options.end())
            res = itr.value();
        return res;
    }

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
