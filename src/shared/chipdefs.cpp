/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QString>
#include <QTextStream>
#include <QStringList>
#include <QFile>
#include <QDesktopServices>
#include <vector>

#include "../common.h"
#include "chipdefs.h"

void chip_definition::update_chipdef(std::vector<chip_definition> & templates, chip_definition & cd)
{
    for(quint32 i = 0; i < templates.size(); ++i)
    {
        chip_definition &templ = templates[i];
        if(cd.getSign() == templ.getSign())
        {
            cd.setName(templ.getName());
            cd.getMems() = templ.getMems();

            for(quint32 x = 0; x < templ.getFuses().size(); ++x)
            {
                quint32 k;
                for(k = 0; k < cd.getFuses().size(); ++k)
                {
                    if(cd.getFuses()[k].name == templ.getFuses()[x].name)
                        break;
                }

                if(k == cd.getFuses().size())
                    cd.getFuses().push_back(templ.getFuses()[x]);
            }
        }
    }

    if(cd.getMems().find("fuses") == cd.getMems().end() && cd.getSign().left(4) == "avr:")
    {
        chip_definition::memorydef mem;
        mem.memid = 3;
        mem.size = 4;
        mem.pagesize = 0;
        cd.getMems()["fuses"] = mem;
    }
}

void chip_definition::parse_chipdefs(const QString &strdefs, std::vector<chip_definition> &res)
{
    QTextStream ss((QString*)&strdefs);
    QString line;

    for(line = ss.readLine(); !line.isNull(); line = ss.readLine())
    {
        if(line.isEmpty() || line.startsWith("#"))
            continue;

        QStringList tokens = line.split(' ', QString::SkipEmptyParts);

        if(tokens.size() < 2)
            continue;

        chip_definition def;
        def.setName(tokens[0]);
        def.setSign(tokens[1]);

        // parse memories
        if(tokens.size() > 2)
        {
            QStringList memories = tokens[2].split(',', QString::SkipEmptyParts);
            for(quint8 i = 0; i < memories.size(); ++i)
            {
                QStringList memory_tokens = memories[i].split('=', QString::SkipEmptyParts);
                if(memory_tokens.size() != 2)
                    continue;

                QStringList mem_size_tokens = memory_tokens[1].split(':', QString::SkipEmptyParts);

                chip_definition::memorydef memdef;
                memdef.memid = i + 1;
                memdef.size = mem_size_tokens[0].toInt();
                if(mem_size_tokens.size() > 1)
                    memdef.pagesize = mem_size_tokens[1].toInt();
                else
                    memdef.pagesize = 0;
                def.getMems()[memory_tokens[0]] = memdef;
            }
        }

        // parse fuses
        for(quint32 i = 3; i < (quint32)tokens.size(); ++i)
        {
            if(tokens[i][0] == '!')
            {
                int sep_pos = tokens[i].indexOf('=');
                if(sep_pos == -1)
                    return Utils::ThrowException("Invalid syntax in the chip definition file.");
                def.getOptions()[tokens[i].mid(1, sep_pos - 1)] = tokens[i].mid(sep_pos + 1);
            }
            else
            {
                QStringList token_parts = tokens[i].split(':', QString::SkipEmptyParts);
                if(token_parts.size() != 2 && token_parts.size() != 3)
                    continue;

                QStringList bit_numbers = token_parts[1].split(',', QString::SkipEmptyParts);

                chip_definition::fuse f;
                f.name = token_parts[0];
                for(quint32 j = 0; j < (quint32)bit_numbers.size(); ++j)
                    f.bits.push_back(bit_numbers[j].toInt());

                if(token_parts.size() == 3)
                {
                    bit_numbers = token_parts[2].split(',', QString::SkipEmptyParts);
                    for(quint32 j = 0; j < (quint32)bit_numbers.size(); ++j)
                        f.values.push_back(bit_numbers[j].toInt());
                }
                def.getFuses().push_back(f);
            }
        }

        // If chip with this signature is already loaded, rewrite it
        quint32 i = 0;
        for(; i < res.size() && res[i].getSign() != def.getSign(); ++i);

        if(i == res.size())
            res.push_back(def);
        else
            res[i] = def;
    }
}

void chip_definition::parse_default_chipsets(std::vector<chip_definition> & res)
{
    static const QString defFileLocations[] =
    {
        ":/definitions/chipdefs",
        "./shupito_chipdefs.txt",
        QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/shupito_chipdefs.txt",
    };

    QFile file;
    for(quint8 i = 0; i < sizeof(defFileLocations)/sizeof(QString); ++i)
    {
        file.setFileName(defFileLocations[i]);
        if(!file.open(QIODevice::ReadOnly))
            continue;

        QTextStream stream(&file);
        QString defs;

        for(QString line = stream.readLine().trimmed(); !line.isNull(); line = stream.readLine().trimmed())
            if(line.length() != 0)
                defs += line + "\n";

        file.close();
        chip_definition::parse_chipdefs(defs, res);
    }
}

chip_definition::chip_definition()
{
}

chip_definition::chip_definition(const QString &sign)
{
    m_signature = sign;
}

chip_definition::memorydef *chip_definition::getMemDef(quint8 memId)
{
    static const QString memNames[] = { "", "flash", "eeprom", "fuses", "sdram" };
    return getMemDef(memNames[memId]);
}
