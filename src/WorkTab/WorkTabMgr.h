/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef WORKTABMGR_H
#define WORKTABMGR_H

#include <vector>
#include <QHash>

#include "../singleton.h"
#include "WorkTab.h"
#include "../ui/tabview.h"

class WorkTabInfo;
class HomeTab;

class WorkTabMgr : public QObject, public Singleton<WorkTabMgr>
{
    Q_OBJECT

    public:
        typedef QHash<quint32, WorkTab*> WorkTabMap;
        typedef QList<WorkTabInfo*> InfoList;

        WorkTabMgr();
        ~WorkTabMgr();

        void RegisterTabInfo(WorkTabInfo *info);

        InfoList const & GetWorkTabInfos() const;
        void SortTabInfos();
        void AddWorkTab(WorkTab *tab, QString label);
        WorkTab *AddWorkTab(WorkTabInfo * info);
        WorkTab* GetNewTab(WorkTabInfo *info);

        WorkTab* getWorkTab(quint32 id)
        {
            WorkTabMap::iterator itr = m_workTabs.find(id);
            if(itr != m_workTabs.end())
                return *itr;
            return NULL;
        }

        void removeTab(quint32 id)
        {
            WorkTabMap::iterator itr = m_workTabs.find(id);
            if(itr != m_workTabs.end())
                removeTab(*itr);
        }

        void removeTab(WorkTab *tab);

        TabView *getWi() { return tabView; }
        TabView *CreateWidget(QWidget *parent);

        quint32 generateNewWidgetId()
        {
            return tabWidgetCounter++;
        }

        quint32 generateNewTabId()
        {
            return tabIdCounter++;
        }

        bool isFileHandled(const QString& extension) const
        {
            return m_handledTypes.contains(extension, Qt::CaseInsensitive);
        }

        void openTabWithFile(const QString& filename);

        void OpenHomeTab();
        void CloseHomeTab();

        bool onTabsClose();

    private slots:
        void OpenHomeTab(quint32 id);

    private:
        InfoList m_workTabInfos;
        WorkTabMap m_workTabs;
        QStringList m_handledTypes;

        quint32 tabWidgetCounter;
        quint32 tabIdCounter;
        TabView *tabView;
        HomeTab *hometab;
};

#define sWorkTabMgr WorkTabMgr::GetSingleton()

#endif // WORKTABMGR_H
