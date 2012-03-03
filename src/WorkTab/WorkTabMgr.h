/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef WORKTABMGR_H
#define WORKTABMGR_H

#include <vector>
#include <map>

#include "singleton.h"
#include "WorkTab.h"
#include "ui/maintabwidget.h"

class WorkTabInfo;
class HomeTab;

class WorkTabMgr : public QObject, public Singleton<WorkTabMgr>
{
    Q_OBJECT

    public:
        typedef std::map<quint16, WorkTab*> WorkTabMap;
        typedef QList<WorkTabInfo*> InfoList;

        WorkTabMgr();
        ~WorkTabMgr();

        void RegisterTabInfo(WorkTabInfo *info);

        InfoList *GetWorkTabInfos();
        void SortTabInfos();
        quint16 AddWorkTab(WorkTab *tab, QString label);

        WorkTab* getWorkTab(int index)
        {
            if(index == 0 && hometab)
                return NULL;
            else
                return (WorkTab*)tabWidget->widget(index);
        }

        void removeTab(quint16 index)
        {
            removeTab((WorkTab*)tabWidget->widget(index));
        }

        void removeTabWithId(quint16 id)
        {
            WorkTabMap::iterator itr = m_workTabs.find(id);
            if(itr != m_workTabs.end())
                removeTab(itr->second);
        }

        void removeTab(WorkTab *tab);

        QTabWidget *getWi() { return tabWidget; }
        QTabWidget *CreateWidget(QWidget *parent);

        void OpenHomeTab();
        void CloseHomeTab();

    public slots:
        void NewTabDialog();

    private:
        InfoList m_workTabInfos;
        WorkTabMap m_workTabs;

        quint16 tabIdCounter;
        MainTabWidget *tabWidget;
        HomeTab *hometab;
};


#define sWorkTabMgr WorkTabMgr::GetSingleton()


#endif // WORKTABMGR_H
