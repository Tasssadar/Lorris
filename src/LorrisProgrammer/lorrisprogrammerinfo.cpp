/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "lorrisprogrammer.h"
#include "lorrisprogrammerinfo.h"

static const LorrisProgrammerInfo info;

LorrisProgrammerInfo::LorrisProgrammerInfo() : WorkTabInfo()
{

}

WorkTab *LorrisProgrammerInfo::GetNewTab()
{
    return new LorrisProgrammer();
}

QString LorrisProgrammerInfo::GetName()
{
    return QObject::tr("Programmer");
}

QString LorrisProgrammerInfo::GetDescription()
{
    return QObject::tr("Graphical interface for various types of programmers and bootloaders.");
}

QStringList LorrisProgrammerInfo::GetHandledFiles()
{
    return QStringList("hex");
}

QString LorrisProgrammerInfo::GetIdString()
{
    // Keep as LorrisShupito because of saved sessions
    return "LorrisShupito";
}
