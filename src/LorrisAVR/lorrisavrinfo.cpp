/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "lorrisavr.h"
#include "lorrisavrinfo.h"

static const LorrisAVRInfo info;

LorrisAVRInfo::LorrisAVRInfo() : WorkTabInfo()
{

}

WorkTab *LorrisAVRInfo::GetNewTab()
{
    return new LorrisAVR();
}

QString LorrisAVRInfo::GetName()
{
    return QObject::tr("AVR emulator");
}

QString LorrisAVRInfo::GetDescription()
{
    return QObject::tr("This is an emulator of AVR ATmega 328p.");
}

QString LorrisAVRInfo::GetIdString()
{
    return "LorrisAVR";
}
