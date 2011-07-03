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
        uint8_t GetConType() { return (CON_MSK(CONNECTION_SOCKET) | CON_MSK(CONNECTION_SERIAL_PORT) | CON_MSK(CONNECTION_FILE)); }
};

#endif // LORRISPROBEINFO_H
