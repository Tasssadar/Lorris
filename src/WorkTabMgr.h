#ifndef WORKTABMGR_H
#define WORKTABMGR_H

#include <vector>
#include <map>
#include <QTabWidget>

#include "singleton.h"
#include "WorkTab.h"

class WorkTabInfo;
class HomeTab;

class WorkTabMgr : public Singleton<WorkTabMgr>
{
    typedef std::map<quint16, WorkTab*> WorkTabMap;

    public:
        WorkTabMgr();
        ~WorkTabMgr();

        std::vector<WorkTabInfo*> *GetWorkTabInfos();
        quint16 AddWorkTab(WorkTab *tab, QString label);

        void removeTab(quint16 index)
        {
            for(WorkTabMap::iterator itr = m_workTabs.begin(); itr != m_workTabs.end(); ++itr)
            {
                if(itr->second->GetTab() == tabWidget->widget(index))
                {
                    removeTab(itr->second);
                    return;
                }
            }
        }

        void removeTabWithId(quint16 id)
        {
            WorkTabMap::iterator itr = m_workTabs.find(id);
            if(itr != m_workTabs.end())
                removeTab(itr->second);
        }

        void removeTab(WorkTab *tab);

        QTabWidget *getWi() { return tabWidget; }
        void CreateWidget(QWidget *parent) { tabWidget = new QTabWidget(parent); }

        void OpenHomeTab();
        void CloseHomeTab();
        void NewTabDialog();

    private:
        std::vector<WorkTabInfo*> m_workTabInfos;
        WorkTabMap m_workTabs;

        quint16 tabIdCounter;
        QTabWidget *tabWidget;
        HomeTab *hometab;
};


#define sWorkTabMgr WorkTabMgr::GetSingleton()


#endif // WORKTABMGR_H
