#ifndef LORRISSHUPITOINFO_H
#define LORRISSHUPITOINFO_H

#include "WorkTab/WorkTabInfo.h"

class LorrisShupitoInfo : public WorkTabInfo
{
public:
    explicit LorrisShupitoInfo();
    virtual ~LorrisShupitoInfo();

    WorkTab *GetNewTab();
    QString GetName();
    QString GetDescription();
    quint8 GetConType() { return CON_MSK(CONNECTION_SERIAL_PORT); }
};

#endif // LORRISSHUPITOINFO_H
