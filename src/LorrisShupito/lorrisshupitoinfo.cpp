#include "lorrisshupito.h"
#include "lorrisshupitoinfo.h"

LorrisShupitoInfo::LorrisShupitoInfo()
{

}

LorrisShupitoInfo::~LorrisShupitoInfo()
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
    return QObject::tr("Control program for shupito programmer.");
}

