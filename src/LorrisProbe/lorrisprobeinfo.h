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
        uint8_t GetConType() { return (CONNECTION_SOCKET | CONNECTION_SERIAL_PORT | CONNECTION_FILE); }
};

#endif // LORRISPROBEINFO_H
