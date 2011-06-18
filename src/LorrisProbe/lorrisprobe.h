#ifndef LORRISPROBE_H
#define LORRISPROBE_H

#include "WorkTab.h"

class LORRISPROBESHARED_EXPORT LorrisProbe : public WorkTab
{
    public:
        explicit LorrisProbe();
        virtual ~LorrisProbe();

        QWidget *GetTab();

    private:
};

#endif // LORRISPROBE_H
