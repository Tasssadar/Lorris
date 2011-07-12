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
    return QString("Terminal");
}

QString LorrisTerminalInfo::GetDescription()
{
    return "Terminal can show input data from serial port, send key strokes "
           "to serial port and flash new programs to devices with bootloader.";
}

