/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef LORRISSHUPITOINFO_H
#define LORRISSHUPITOINFO_H

#include "../WorkTab/WorkTabInfo.h"

class LorrisShupitoInfo : public WorkTabInfo
{
public:
    explicit LorrisShupitoInfo();

    WorkTab *GetNewTab();
    QString GetName();
    QString GetDescription();
    QStringList GetHandledFiles();
    QString GetIdString();
};

#endif // LORRISSHUPITOINFO_H
