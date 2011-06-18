#ifndef LORRISPROBEINFO_H
#define LORRISPROBEINFO_H

#include "WorkTabInfo.h"

class LorrisProbeInfo : public WorkTabInfo
{
    public:
        explicit LorrisProbeInfo();
        virtual ~LorrisProbeInfo();

        WorkTab *GetNewTab();
        QString GetName();
};

#endif // LORRISPROBEINFO_H
