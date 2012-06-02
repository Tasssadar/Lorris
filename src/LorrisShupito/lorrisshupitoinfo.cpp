/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "lorrisshupito.h"
#include "lorrisshupitoinfo.h"

static const LorrisShupitoInfo info;

LorrisShupitoInfo::LorrisShupitoInfo() : WorkTabInfo()
{

}

WorkTab *LorrisShupitoInfo::GetNewTab()
{
    return new LorrisShupito();
}

QString LorrisShupitoInfo::GetName()
{
    return QObject::tr("Shupito");
}

QString LorrisShupitoInfo::GetDescription()
{
    return QObject::tr("Control program for the Shupito programmer.");
}

QStringList LorrisShupitoInfo::GetHandledFiles()
{
    return QStringList("hex");
}
