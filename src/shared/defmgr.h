/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef DEFMGR_H
#define DEFMGR_H

#include <QObject>
#include <vector>
#include <QTextStream>
#include <QMultiHash>

#include "../singleton.h"
#include "chipdefs.h"
#include "fuse_desc.h"

class DefMgr : public QObject, public Singleton<DefMgr>
{
    Q_OBJECT
public:
    explicit DefMgr(QObject *parent = 0);

    void updateChipdef(chip_definition & cd);
    fuse_desc *findFuse_desc(const QString& name, const QString& chipSign);
    
private:
    void loadChipdefs();
    void loadFusedesc();

    void parseChipdefs(QTextStream &ss);
    void parseFusedesc(QTextStream& ss);

    std::vector<chip_definition> m_chipdefs;
    QMultiHash<QString, fuse_desc> m_fusedesc;
};

#define sDefMgr DefMgr::GetSingleton()

#endif // DEFMGR_H
