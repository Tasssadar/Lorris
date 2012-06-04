/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef LORRISAVRINFO_H
#define LORRISAVRINFO_H

#include "../WorkTab/WorkTabInfo.h"

class LorrisAVRInfo : public WorkTabInfo
{
public:
    explicit LorrisAVRInfo();

    WorkTab *GetNewTab();
    QString GetName();
    QString GetDescription();
};

#endif // LORRISAVRINFO_H
