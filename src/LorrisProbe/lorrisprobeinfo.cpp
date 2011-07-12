#include "lorrisprobe.h"
#include "lorrisprobeinfo.h"

LorrisProbeInfo::LorrisProbeInfo()
{

}

LorrisProbeInfo::~LorrisProbeInfo()
{

}

WorkTab *LorrisProbeInfo::GetNewTab()
{
    return new LorrisProbe();
}

QString LorrisProbeInfo::GetName()
{
    return QObject::tr("Lorris probe");
}

QString LorrisProbeInfo::GetDescription()
{
    return "";
}
