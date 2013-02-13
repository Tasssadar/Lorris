/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef LORRISPROGRAMMERINFO_H
#define LORRISPROGRAMMERINFO_H

#include "../WorkTab/WorkTabInfo.h"

class LorrisProgrammerInfo : public WorkTabInfo
{
public:
    explicit LorrisProgrammerInfo();

    WorkTab *GetNewTab();
    QString GetName();
    QString GetDescription();
    QStringList GetHandledFiles();
    QString GetIdString();
};

#endif // LORRISPROGRAMMERINFO_H
