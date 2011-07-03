#ifndef WORKTABMGR_H
#define WORKTABMGR_H

#include <vector>

#include "singleton.h"

class WorkTabInfo;

class WorkTabMgr : public Singleton<WorkTabMgr>
{
    public:
        WorkTabMgr();
        ~WorkTabMgr();

        std::vector<WorkTabInfo*> *GetWorkTabInfos();

    private:
        std::vector<WorkTabInfo*> m_workTabInfos;

};

//template <class WorkTabMgr> WorkTabMgr *Singleton<WorkTabMgr>::msSingleton = 0;
#define sWorkTabMgr WorkTabMgr::GetSingleton()


#endif // WORKTABMGR_H
