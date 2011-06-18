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
    return QString("Lorris probe");
}
