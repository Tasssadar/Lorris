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


#include "WorkTabMgr.h"
#include "WorkTabInfo.h"
#include "ui/HomeTab.h"
#include "ui/tabdialog.h"

WorkTabMgr::WorkTabMgr() : QObject()
{
    tabIdCounter = 0;
    tabWidgetCounter = 0;
    tabView = NULL;
    hometab = NULL;
}

WorkTabMgr::~WorkTabMgr()
{
}

void WorkTabMgr::RegisterTabInfo(WorkTabInfo *info)
{
    m_workTabInfos.push_back(info);
}

void WorkTabMgr::SortTabInfos()
{
    QMap<QString, WorkTabInfo*> map;

    for(InfoList::iterator itr = m_workTabInfos.begin(); itr != m_workTabInfos.end(); ++itr)
        map.insert((*itr)->GetName(), *itr);

    m_workTabInfos = map.values();
}

WorkTabMgr::InfoList const & WorkTabMgr::GetWorkTabInfos() const
{
    return m_workTabInfos;
}

WorkTab *WorkTabMgr::GetNewTab(WorkTabInfo *info)
{
    WorkTab *tab = info->GetNewTab();
    tab->setInfo(info);
    tab->setId(generateNewTabId());
    return tab;
}

void WorkTabMgr::AddWorkTab(WorkTab *tab, QString label)
{
    CloseHomeTab();

    m_workTabs.insert(tab->getId(), tab);

    TabWidget *activeWidget = tabView->getActiveWidget();

    tab->setParent(activeWidget);
    activeWidget->addTab(tab, label, tab->getId());
    activeWidget->setTabsClosable(true);
    return;
}

void WorkTabMgr::removeTab(WorkTab *tab)
{
    m_workTabs.remove(tab->getId());
    delete tab;
}

void WorkTabMgr::OpenHomeTab(quint32 id)
{
    Q_ASSERT(!hometab);

    TabWidget *tabWidget = tabView->getWidget(id);
    if(!tabWidget)
        return;

    hometab = new HomeTab(tabWidget);
    tabWidget->addTab(hometab, QObject::tr("Home"));
}

void WorkTabMgr::OpenHomeTab()
{
    TabWidget *activeWidget = tabView->getActiveWidget();

    hometab = new HomeTab(activeWidget);
    activeWidget->addTab(hometab, QObject::tr("Home"));
}

void WorkTabMgr::CloseHomeTab()
{
    if(!hometab)
        return;

    TabWidget *activeWidget = tabView->getActiveWidget();
    if(!activeWidget)
    {
        Q_ASSERT(false);
        return;
    }
    activeWidget->removeTab(activeWidget->indexOf(hometab));
    delete hometab;
    hometab = NULL;
}

void WorkTabMgr::NewTabDialog()
{
    TabDialog *dialog = new TabDialog;
    dialog->exec();
    delete dialog;
}

TabView *WorkTabMgr::CreateWidget(QWidget *parent)
{
    tabView = new TabView(parent);

    connect(tabView, SIGNAL(newTab()), SLOT(NewTabDialog()));
    connect(tabView, SIGNAL(openHomeTab(quint32)), SLOT(OpenHomeTab(quint32)));
    return tabView;
}

bool WorkTabMgr::onTabsClose()
{
    for(WorkTabMap::iterator itr = m_workTabs.begin(); itr != m_workTabs.end(); ++itr)
    {
        if(!(*itr)->onTabClose())
            return false;
    }
    return true;
}
