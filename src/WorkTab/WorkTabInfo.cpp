/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QStringList>

#include "WorkTabInfo.h"
#include "WorkTabMgr.h"

WorkTabInfo::WorkTabInfo()
{
    sWorkTabMgr.RegisterTabInfo(this);
}

QStringList WorkTabInfo::GetHandledFiles()
{
    return QStringList();
}
