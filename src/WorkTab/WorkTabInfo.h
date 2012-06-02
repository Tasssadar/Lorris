/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef WORKTABINFO_H
#define WORKTABINFO_H

#include <QString>
#include <QStringList>

class WorkTab;

class WorkTabInfo
{
public:
    virtual WorkTab *GetNewTab() = 0;
    virtual QString GetName() = 0;
    virtual QString GetDescription() = 0;
    virtual QStringList GetHandledFiles();

protected:
    explicit WorkTabInfo();
};

#endif // WORKTABINFO_H
