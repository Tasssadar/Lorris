/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "lorristerminal.h"
#include "lorristerminalinfo.h"

static const LorrisTerminalInfo info;

LorrisTerminalInfo::LorrisTerminalInfo() : WorkTabInfo()
{

}

WorkTab *LorrisTerminalInfo::GetNewTab()
{
    return new LorrisTerminal();
}

QString LorrisTerminalInfo::GetName()
{
    return QObject::tr("Terminal");
}

QString LorrisTerminalInfo::GetDescription()
{
    return QObject::tr("Terminal can show input data from connections, send key strokes "
           "and flash new programs to devices with a bootloader.");
}
