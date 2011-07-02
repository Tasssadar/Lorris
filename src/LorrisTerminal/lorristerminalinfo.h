#ifndef LORRISTERMINALINFO_H
#define LORRISTERMINALINFO_H

#include "WorkTabInfo.h"

class LorrisTerminalInfo : public WorkTabInfo
{
public:
    explicit LorrisTerminalInfo();
    virtual ~LorrisTerminalInfo();

    WorkTab *GetNewTab();
    QString GetName();
    uint8_t GetConType() { return CONNECTION_SERIAL_PORT; }
};

#endif // LORRISTERMINALINFO_H
