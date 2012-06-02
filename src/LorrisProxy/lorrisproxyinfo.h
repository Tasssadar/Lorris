/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef LORRISPROXYINFO_H
#define LORRISPROXYINFO_H

#include "../WorkTab/WorkTabInfo.h"

class LorrisProxyInfo : public WorkTabInfo
{
public:
    explicit LorrisProxyInfo();

    WorkTab *GetNewTab();
    QString GetName();
    QString GetDescription();
};

#endif // LORRISPROXYINFO_H
