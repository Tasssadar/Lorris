#ifndef LORRISTERMINAL_H
#define LORRISTERMINAL_H

#include "WorkTab.h"

class LorrisTerminal : public WorkTab
{
public:
    explicit LorrisTerminal();
    virtual ~LorrisTerminal();

    QWidget *GetTab(QWidget *parent);

private:
    QWidget *mainWidget;
};

#endif // LORRISTERMINAL_H
