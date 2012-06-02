/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef LORRISTERMINALINFO_H
#define LORRISTERMINALINFO_H

#include "../WorkTab/WorkTabInfo.h"

class LorrisTerminalInfo : public WorkTabInfo
{
public:
    explicit LorrisTerminalInfo();

    WorkTab *GetNewTab();
    QString GetName();
    QString GetDescription();
};

#endif // LORRISTERMINALINFO_H
