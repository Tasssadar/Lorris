#ifndef LORRISPROBE_H
#define LORRISPROBE_H

#include "WorkTab.h"

class LorrisProbe : public WorkTab
{
    Q_OBJECT
    public:
        explicit LorrisProbe();
        virtual ~LorrisProbe();

        QWidget *GetTab(QWidget *parent = NULL);

    private:
        void readData(QByteArray data);
};

#endif // LORRISPROBE_H
