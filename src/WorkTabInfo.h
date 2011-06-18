#ifndef WORKTABINFO_H
#define WORKTABINFO_H

#include <QString>

class WorkTab;

class WorkTabInfo
{
    public:
        virtual ~WorkTabInfo();

        virtual WorkTab *GetNewTab();
        virtual QString GetName();

    protected:
        explicit WorkTabInfo();

};

#endif // WORKTABINFO_H
