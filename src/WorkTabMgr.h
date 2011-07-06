#ifndef WORKTABMGR_H
#define WORKTABMGR_H

#include <vector>
#include <map>
#include <QtCore/QVariant>

#include "singleton.h"

class WorkTabInfo;
class WorkTab;

class WorkTabMgr : public Singleton<WorkTabMgr>
{
    public:
        WorkTabMgr();
        ~WorkTabMgr();

        std::vector<WorkTabInfo*> *GetWorkTabInfos();
        uint16_t AddWorkTab(WorkTab *tab);

    private:
        std::vector<WorkTabInfo*> m_workTabInfos;
        std::map<uint16_t, WorkTab*> m_workTabs;

        uint16_t tabIdCounter;

};

//template <class WorkTabMgr> WorkTabMgr *Singleton<WorkTabMgr>::msSingleton = 0;
#define sWorkTabMgr WorkTabMgr::GetSingleton()


#endif // WORKTABMGR_H
