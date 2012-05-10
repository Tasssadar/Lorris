/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef LORRISANALYZERINFO_H
#define LORRISANALYZERINFO_H

#include "../WorkTab/WorkTabInfo.h"

class LorrisAnalyzerInfo : public WorkTabInfo
{
public:
    explicit LorrisAnalyzerInfo();

    WorkTab *GetNewTab();
    QString GetName();
    QString GetDescription();
    QStringList GetHandledFiles();
};

#endif // LORRISANALYZERINFO_H
