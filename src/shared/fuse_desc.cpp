/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QTextStream>
#include <QDesktopServices>
#include <QFile>

#include "fuse_desc.h"

fuse_desc::fuse_desc(const QString& name)
{
    m_name = name;
}

void fuse_desc::parse_fusedesc(QString const & desc, std::vector<fuse_desc>& res)
{
    QTextStream ss((QString*)&desc);

    for(QString line = ss.readLine(); !line.isNull(); line = ss.readLine())
    {
        if(line.isEmpty() || line.startsWith('#'))
            continue;

        QStringList tokens = line.split(" | ", QString::SkipEmptyParts);

        if(tokens.size() < 3)
            continue;

        fuse_desc desc(tokens[0]);

        QStringList chips = tokens[1].split(',', QString::SkipEmptyParts);
        if(chips.isEmpty())
            continue;

        desc.setChips(chips);

        if(tokens[2].count('"') != 2)
            continue;

        desc.setDesc(tokens[2].remove('"'));

        for(int i = 3; i < tokens.size(); ++i)
        {
            QStringList parts = tokens[i].split('=', QString::SkipEmptyParts);
            if(parts.size() != 2 || !parts[0].startsWith("0b") || parts[1].count('"') != 2)
                continue;

            desc.addOption(parts[0], parts[1].remove('"'));
        }
        res.push_back(desc);
    }
}

void fuse_desc::parse_default_fuses(std::vector<fuse_desc>& res)
{
    static const QString defFileLocations[] =
    {
        ":/definitions/fusedesc",
        "./shupito_fusedesc.txt",
        QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/shupito_fusedesc.txt",
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
        fuse_desc::parse_fusedesc(defs, res);
    }
}

fuse_desc *fuse_desc::findDesc(const QString &name, const QString &chipSign, std::vector<fuse_desc> &templates)
{
    for(std::vector<fuse_desc>::iterator itr = templates.begin(); itr != templates.end(); ++itr)
    {
        if((*itr).getName() == name && (*itr).isForChip(chipSign))
            return &(*itr);
    }
    return NULL;
}
