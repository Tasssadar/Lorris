#ifndef WORKTAB_H
#define WORKTAB_H

#include "common.h"
#include <QtGui/QWidget>

class LORRISPROBESHARED_EXPORT WorkTab
{
    public:
        virtual ~WorkTab();
        virtual QWidget *GetTab(QWidget *parent);

    protected:
        explicit WorkTab();

};

#endif // WORKTAB_H
