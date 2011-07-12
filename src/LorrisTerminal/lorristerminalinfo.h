#ifndef LORRISTERMINALINFO_H
#define LORRISTERMINALINFO_H

#include "WorkTab/WorkTabInfo.h"

class LorrisTerminalInfo : public WorkTabInfo
{
public:
    explicit LorrisTerminalInfo();
    virtual ~LorrisTerminalInfo();

    WorkTab *GetNewTab();
    QString GetName();
    QString GetDescription();
    quint8 GetConType() { return CON_MSK(CONNECTION_SERIAL_PORT); }
};

#endif // LORRISTERMINALINFO_H
