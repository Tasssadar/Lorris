/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <algorithm>
#include <QStatusBar>

#include "WorkTabMgr.h"
#include "WorkTabInfo.h"
#include "../ui/HomeTab.h"
#include "childtab.h"

WorkTabMgr::WorkTabMgr() : QObject()
{
    tabIdCounter = 0;
    tabWidgetCounter = 0;
    windowIdCounter = 0;
    m_session_mgr = NULL;
    m_disable_window_close = false;
    m_batch_started = false;
}

WorkTabMgr::~WorkTabMgr()
{
}

void WorkTabMgr::initialize(const QStringList &openFiles, const QString& session)
{
    m_session_mgr = new SessionMgr(this);
    newWindow(openFiles);

    if(!session.isEmpty() || (openFiles.isEmpty() && sConfig.get(CFG_BOOL_LOAD_LAST_SESSION)))
        m_session_mgr->loadSession(session, true);
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
    connect(tab.data(), SIGNAL(destroyed(QObject*)), SLOT(workTabDestroyed(QObject*)));
    return tab.take();
}

void WorkTabMgr::workTabDestroyed(QObject *tab)
{
    WorkTab *w_tab = (WorkTab*)tab;
    m_workTabs.remove(w_tab->getId());
    m_children.remove(w_tab->getId());
}

void WorkTabMgr::AddWorkTab(WorkTab *tab, MainWindow *window, QString label)
{
    window->closeHomeTab();

    m_workTabs.insert(tab->getId(), tab);

    TabWidget *activeWidget = window->getTabView()->getActiveWidget();

    tab->setParent(activeWidget);
    tab->setWindowId(window->getId());
    activeWidget->addTab(tab, label, tab->getId());
    activeWidget->setTabsClosable(true);
    return;
}

void WorkTabMgr::registerTab(WorkTab *tab)
{
    m_windows[tab->getWindowId()]->closeHomeTab();
    m_workTabs.insert(tab->getId(), tab);
}

WorkTab * WorkTabMgr::AddWorkTab(WorkTabInfo * info, MainWindow *window, QString filename)
{
    QScopedPointer<WorkTab> tab(this->GetNewTab(info));
    this->AddWorkTab(tab.data(), window, info->GetName());

    WorkTab * tabp = tab.take();
    tabp->onTabShow(filename);
    return tabp;
}

WorkTab *WorkTabMgr::AddWorkTab(WorkTabInfo *info, quint32 windowId, QString filename)
{
    Q_ASSERT(m_windows.contains(windowId));
    return AddWorkTab(info, m_windows[windowId], filename);
}

void WorkTabMgr::removeTab(WorkTab *tab)
{
    ChildrenMap::iterator itr = m_children.find(tab->getId());
    if(itr != m_children.end())
    {
        for(std::set<ChildTab*>::const_iterator w_itr = (*itr).begin(); w_itr != (*itr).end(); ++w_itr)
        {
            TabWidget *tabW = getTabWidgetWithWidget(*w_itr);
            tabW->removeChildTab(*w_itr);
            tab->childClosed(*w_itr);
        }
        m_children.erase(itr);
    }
    m_workTabs.remove(tab->getId());
    delete tab;
}

bool WorkTabMgr::askChildrenToClose(quint32 parentId)
{
    ChildrenMap::iterator c_itr = m_children.find(parentId);
    if(c_itr == m_children.end())
        return true;

    for(std::set<ChildTab*>::const_iterator w_itr = (*c_itr).begin(); w_itr != (*c_itr).end(); ++w_itr)
    {
        if(!(*w_itr)->onTabClose())
            return false;
    }
    return true;
}

void WorkTabMgr::openTabWithFile(const QString &filename, MainWindow *window)
{
    if(!filename.contains("."))
        return;

    QString suffix = filename.split(".", QString::SkipEmptyParts).back();
    for(InfoList::Iterator itr = m_workTabInfos.begin(); itr != m_workTabInfos.end(); ++itr)
    {
        if(!(*itr)->GetHandledFiles().contains(suffix))
            continue;

        AddWorkTab(*itr, window, filename);
        return;
    }
}

void WorkTabMgr::addChildTab(ChildTab *child, const QString &name, quint32 workTabId)
{
    TabWidget *tabW = getTabWidgetWithTab(workTabId);
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

    tab->childClosed(child);
}

MainWindow *WorkTabMgr::newWindow(QStringList openFiles)
{
    quint32 id = generateNewWindowId();
    MainWindow *w = new MainWindow(id);
    w->show(openFiles);
    m_windows.insert(id, w);
    return w;
}

void WorkTabMgr::registerTabWidget(TabWidget *widget)
{
    m_tab_widgets.insert(widget->getId(), widget);
    connect(widget, SIGNAL(destroyed(QObject*)), SLOT(tabWidgetDestroyed(QObject*)));
}

void WorkTabMgr::tabWidgetDestroyed(QObject *widget)
{
    m_tab_widgets.remove(((TabWidget*)widget)->getId());
}

TabWidget *WorkTabMgr::getTabWidgetWithWidget(QWidget *widget)
{
    for(TabWidgetMap::iterator itr = m_tab_widgets.begin(); itr != m_tab_widgets.end(); ++itr)
        if((*itr)->indexOf(widget) != -1)
            return *itr;
    return NULL;
}

TabWidget *WorkTabMgr::getTabWidgetWithTab(quint32 tabId)
{
    for(TabWidgetMap::iterator itr = m_tab_widgets.begin(); itr != m_tab_widgets.end(); ++itr)
        if((*itr)->containsTab(tabId))
            return *itr;
    return NULL;
}

MainWindow *WorkTabMgr::getWindow(quint32 id)
{
    WindowMap::iterator itr = m_windows.find(id);
    if(itr == m_windows.end())
        return NULL;
    return *itr;
}

void WorkTabMgr::saveData(DataFileParser *file)
{
    file->writeBlockIdentifier("windowsInfo");
    file->writeVal(m_windows.size());

    for(WindowMap::iterator itr = m_windows.begin(); itr != m_windows.end(); ++itr)
        (*itr)->saveData(file);
}

void WorkTabMgr::loadData(DataFileParser *file, bool closeOther)
{
    if(!file->seekToNextBlock("windowsInfo", 0))
        return;

    m_disable_window_close = true;

    startBatchOperation();

    int count = file->readVal<int>();

    if(!closeOther)
    {
        // use current window, if it is empty
        MainWindow *activeWin = dynamic_cast<MainWindow*>(qApp->activeWindow());
        if(activeWin && activeWin->onlyHomeTab())
        {
            activeWin->loadData(file);
            --count;
        }

        // create new windows
        for(int i = 0; i < count; ++i)
        {
            MainWindow *win = newWindow();
            win->loadData(file);
        }
    }
    else
    {
        QList<quint32> keys = m_windows.keys();
        while(count != m_windows.size())
        {
            if(count < m_windows.size())
            {
                m_windows[keys.back()]->close();
                m_windows.remove(keys.takeLast());
            }
            else
                newWindow();
        }

        for(WindowMap::iterator itr = m_windows.begin(); itr != m_windows.end(); ++itr)
            (*itr)->loadData(file);
    }

    m_disable_window_close = false;

    endBatchOperation();
}

void WorkTabMgr::printToAllStatusBars(const QString &text, int timeout)
{
    for(WindowMap::iterator itr = m_windows.begin(); itr != m_windows.end(); ++itr)
        (*itr)->statusBar()->showMessage(text, timeout);
}

void WorkTabMgr::removeWindow(quint32 id)
{
    m_windows.remove(id);
}

quint32 WorkTabMgr::generateNewTabId()
{
    if(tabIdCounter >= 0x00FFFFFF)
    {
        qWarning("Tab guid overflow");
        tabIdCounter = 0;
    }
    return tabIdCounter++;
}

quint32 WorkTabMgr::generateNewChildId()
{
    if(tabIdCounter >= 0x00FFFFFF)
    {
        qWarning("Tab guid overflow");
        tabIdCounter = 0;
    }
    return (IDMASK_CHILD | tabIdCounter++);
}

void WorkTabMgr::instanceMessage(const QString &message)
{
    QStringList parts = message.split("|");
    if(parts.size() < 2)
        return;

    if(parts[0] == "newWindow")
    {
        MainWindow *w = newWindow(parts[1].split(";"));
        w->activateWindow();
        w->raise();
    }
}

void WorkTabMgr::startBatchOperation()
{
    if(!m_batch_started)
    {
        m_batch_started = true;
        m_batch_vars.clear();
    }
}

void WorkTabMgr::endBatchOperation()
{
    if(m_batch_started)
        m_batch_started = false;
}

void WorkTabMgr::setBatchVar(const QString& name, const QVariant &var)
{
    m_batch_vars[name] = var;
}

QVariant WorkTabMgr::getBatchVar(const QString& name, QVariant def)
{
    QHash<QString, QVariant>::iterator itr = m_batch_vars.find(name);
    if(itr != m_batch_vars.end())
        return itr.value();
    return def;
}
