#ifndef WORKTABMGR_H
#define WORKTABMGR_H

#include <vector>

class WorkTabInfo;

class WorkTabMgr
{
    public:
        WorkTabMgr();
        ~WorkTabMgr();

        std::vector<WorkTabInfo*> *GetWorkTabInfos();

    private:
        std::vector<WorkTabInfo*> m_workTabInfos;

};

#endif // WORKTABMGR_H
