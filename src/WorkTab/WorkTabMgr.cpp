/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/


#include "WorkTabMgr.h"
#include "WorkTabInfo.h"
#include "../ui/HomeTab.h"
#include "childtab.h"
#include <algorithm>

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

static bool compareTabInfos(WorkTabInfo * lhs, WorkTabInfo * rhs)
{
    return lhs->GetName() < rhs->GetName();
}

void WorkTabMgr::SortTabInfos()
{
    std::sort(m_workTabInfos.begin(), m_workTabInfos.end(), compareTabInfos);

    // Must be done here, because RegisterTabInfo is called from
    // WorkTabInfo's constructor so virtual methods do not work
    for(InfoList::Iterator itr = m_workTabInfos.begin(); itr != m_workTabInfos.end(); ++itr)
        m_handledTypes += (*itr)->GetHandledFiles();
}

WorkTabMgr::InfoList const & WorkTabMgr::GetWorkTabInfos() const
{
    return m_workTabInfos;
}

WorkTab *WorkTabMgr::GetNewTab(WorkTabInfo *info)
{
    QScopedPointer<WorkTab> tab(info->GetNewTab());
    tab->setInfo(info);
    tab->setId(generateNewTabId());
    return tab.take();
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

void WorkTabMgr::registerTab(WorkTab *tab)
{
    CloseHomeTab();
    m_workTabs.insert(tab->getId(), tab);
}

WorkTab * WorkTabMgr::AddWorkTab(WorkTabInfo * info, QString filename)
{
    QScopedPointer<WorkTab> tab(this->GetNewTab(info));
    this->AddWorkTab(tab.data(), info->GetName());

    WorkTab * tabp = tab.take();
    tabp->onTabShow(filename);
    return tabp;
}

void WorkTabMgr::removeTab(WorkTab *tab)
{
    ChildrenMap::iterator itr = m_children.find(tab->getId());
    if(itr != m_children.end())
    {
        for(std::set<ChildTab*>::const_iterator w_itr = (*itr).begin(); w_itr != (*itr).end(); ++w_itr)
        {
            TabWidget *tabW = tabView->getWidgetWithWidget(*w_itr);
            tabW->removeChildTab(*w_itr);
        }
        m_children.erase(itr);
    }
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

TabView *WorkTabMgr::CreateWidget(QWidget *parent)
{
    tabView = new TabView(parent);

    connect(tabView, SIGNAL(openHomeTab(quint32)), SLOT(OpenHomeTab(quint32)));
    return tabView;
}

bool WorkTabMgr::onTabsClose()
{
    try {
        tabView->getSessionMgr()->saveSession();
    } catch(const QString&) { }

    for(WorkTabMap::iterator itr = m_workTabs.begin(); itr != m_workTabs.end(); ++itr)
    {
        if(!(*itr)->onTabClose())
            return false;
    }
    return true;
}

void WorkTabMgr::openTabWithFile(const QString &filename)
{
    QString suffix = filename.split(".", QString::SkipEmptyParts).back();
    for(InfoList::Iterator itr = m_workTabInfos.begin(); itr != m_workTabInfos.end(); ++itr)
    {
        if(!(*itr)->GetHandledFiles().contains(suffix))
            continue;

        AddWorkTab(*itr, filename);
        return;
    }
}

void WorkTabMgr::addChildTab(ChildTab *child, const QString &name, quint32 workTabId)
{
    TabWidget *tabW = tabView->getWidgetWithTab(workTabId);
    if(!tabW)
        return;

    child->setParentId(workTabId);
    tabW->addChildTab(child, name);
    m_children[workTabId].insert(child);
}

void WorkTabMgr::removeChildTab(ChildTab *child)
{
    quint32 tabId = 0;
    for(ChildrenMap::iterator itr = m_children.begin(); itr != m_children.end(); ++itr)
    {
        std::set<ChildTab*>::iterator w_itr = (*itr).find(child);
        if(w_itr == (*itr).end())
            continue;

        tabId = itr.key();

        (*itr).erase(w_itr);

        if((*itr).empty())
            m_children.erase(itr);
        break;
    }

    WorkTab *tab = getWorkTab(tabId);
    if(!tab)
        return;

    TabWidget *tabW = tabView->getWidgetWithWidget(child);
    tabW->removeChildTab(child);

    tab->childClosed(child);
}
