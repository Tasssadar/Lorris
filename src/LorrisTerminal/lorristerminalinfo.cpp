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
