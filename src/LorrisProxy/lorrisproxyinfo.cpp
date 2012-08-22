/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "lorrisproxy.h"
#include "lorrisproxyinfo.h"

static const LorrisProxyInfo info;

LorrisProxyInfo::LorrisProxyInfo() : WorkTabInfo()
{

}

WorkTab *LorrisProxyInfo::GetNewTab()
{
    return new LorrisProxy();
}

QString LorrisProxyInfo::GetName()
{
    return QObject::tr("Proxy");
}

QString LorrisProxyInfo::GetDescription()
{
    return QObject::tr("This module acts as proxy between a connection and TCP server socket.");
}

QString LorrisProxyInfo::GetIdString()
{
    return "LorrisProxy";
}

