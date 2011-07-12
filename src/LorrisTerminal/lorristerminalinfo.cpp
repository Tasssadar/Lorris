#include "lorristerminal.h"
#include "lorristerminalinfo.h"

LorrisTerminalInfo::LorrisTerminalInfo()
{

}

LorrisTerminalInfo::~LorrisTerminalInfo()
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
    return QObject::tr("Terminal can show input data from serial port, send key strokes "
           "to serial port and flash new programs to devices with bootloader.");
}

