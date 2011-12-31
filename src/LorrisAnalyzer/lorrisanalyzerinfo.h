#ifndef LORRISANALYZERINFO_H
#define LORRISANALYZERINFO_H

#include "WorkTab/WorkTabInfo.h"

class LorrisAnalyzerInfo : public WorkTabInfo
{
public:
    explicit LorrisAnalyzerInfo();
    virtual ~LorrisAnalyzerInfo();

    WorkTab *GetNewTab();
    QString GetName();
    QString GetDescription();
    quint8 GetConType() { return (CON_MSK(CONNECTION_FILE) | CON_MSK(CONNECTION_SERIAL_PORT)); }
};

#endif // LORRISANALYZERINFO_H
