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

#include "../misc/singleton.h"
#include "WorkTab.h"
#include "../ui/tabview.h"
#include "../ui/mainwindow.h"

class WorkTabInfo;
class HomeTab;
class ChildTab;

#define IDMASK_CHILD 0x01000000

class WorkTabMgr : public QObject, public Singleton<WorkTabMgr>
{
    Q_OBJECT

public:
    typedef QHash<quint32, WorkTab*> WorkTabMap;
    typedef QHash<quint32, std::set<ChildTab*> > ChildrenMap;
    typedef QHash<quint32, TabWidget*> TabWidgetMap;
    typedef QList<WorkTabInfo*> InfoList;
    typedef QHash<quint32, MainWindow*> WindowMap;

    WorkTabMgr();
    ~WorkTabMgr();

    void initialize(const QStringList& openFiles);

    void RegisterTabInfo(WorkTabInfo *info);

    InfoList const & GetWorkTabInfos() const;
    void SortTabInfos();
    void AddWorkTab(WorkTab *tab, MainWindow *window, QString label);
    WorkTab *AddWorkTab(WorkTabInfo * info, MainWindow *window, QString filename = QString());
    WorkTab *AddWorkTab(WorkTabInfo * info, quint32 windowId, QString filename = QString());
    WorkTab* GetNewTab(WorkTabInfo *info);
    void registerTab(WorkTab *tab);

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

    void addChildTab(ChildTab *child, const QString& name, quint32 workTabId);
    void removeChildTab(ChildTab *child);

    quint32 generateNewWidgetId() { return tabWidgetCounter++; }
    quint32 generateNewWindowId() { return windowIdCounter++; }
    quint32 generateNewTabId();
    quint32 generateNewChildId();

    bool isFileHandled(const QString& extension) const
    {
        return m_handledTypes.contains(extension, Qt::CaseInsensitive);
    }

    void openTabWithFile(const QString& filename, MainWindow *window);

    void OpenHomeTab();
    void CloseHomeTab();

    bool onTabsClose(quint32 windowId);
    bool isAnyTabOpened() const { return !m_workTabs.isEmpty(); }

    SessionMgr *getSessionMgr() const { return m_session_mgr; }
    void registerTabWidget(TabWidget *widget);

    TabWidget *getTabWidgetWithWidget(QWidget *widget);
    TabWidget *getTabWidgetWithTab(quint32 tabId);

    MainWindow *getWindow(quint32 id);
    bool canCloseWindow() const { return !m_disable_window_close && m_windows.size() > 1; }
    void removeWindow(quint32 id);

    void saveData(DataFileParser *file);
    void loadData(DataFileParser *file);

    void printToAllStatusBars(const QString& text, int timeout = 3000);

public slots:
    MainWindow *newWindow(QStringList openFiles = QStringList());
    void instanceMessage(const QString& message);

private slots:
    void tabWidgetDestroyed(QObject *widget);
    void workTabDestroyed(QObject *tab);

private:
    InfoList m_workTabInfos;
    WorkTabMap m_workTabs;
    QStringList m_handledTypes;
    ChildrenMap m_children;
    WindowMap m_windows;
    TabWidgetMap m_tab_widgets;

    quint32 tabWidgetCounter;
    quint32 tabIdCounter;
    quint32 windowIdCounter;

    bool m_disable_window_close;
    SessionMgr *m_session_mgr;
};

#define sWorkTabMgr WorkTabMgr::GetSingleton()

#endif // WORKTABMGR_H
