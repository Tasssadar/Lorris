#include "WorkTabMgr.h"
#include "WorkTabInfo.h"
#include "ui/HomeTab.h"
#include "ui/tabdialog.h"

#include "LorrisProbe/lorrisprobeinfo.h"
#include "LorrisTerminal/lorristerminalinfo.h"

WorkTabMgr::WorkTabMgr()
{
    //put ALL plugins into this vector
    m_workTabInfos.push_back(new LorrisProbeInfo);
    m_workTabInfos.push_back(new LorrisTerminalInfo);
    tabIdCounter = 0;
    tabWidget = NULL;
    hometab = NULL;
}

WorkTabMgr::~WorkTabMgr()
{
    for(quint8 itr = 0; itr < m_workTabInfos.size(); ++itr)
        delete m_workTabInfos[itr];

    for(qint16 i = tabWidget->count(); tabWidget->count() && hometab == NULL;)
        removeTab(--i);

    CloseHomeTab();
    delete tabWidget;
}

std::vector<WorkTabInfo*> *WorkTabMgr::GetWorkTabInfos()
{
    return &m_workTabInfos;
}

quint16 WorkTabMgr::AddWorkTab(WorkTab *tab, QString label)
{
    quint16 id = tabIdCounter++;

    m_workTabs.insert(std::make_pair<quint16, WorkTab*>(id, tab));
    tab->setId(id);

    tab->setParent(tabWidget);
    quint16 index = tabWidget->addTab(tab, label);
    if(tabWidget->count() > 1)
    {
        tabWidget->setTabsClosable(true);
        tabWidget->setCurrentIndex(index);
    }
    CloseHomeTab();
    return id;
}

void WorkTabMgr::removeTab(WorkTab *tab)
{
    tabWidget->removeTab(tabWidget->indexOf(tab));
    m_workTabs.erase(tab->getId());
    if(tabWidget->count() == 0)
    {
        OpenHomeTab();
        tabWidget->setTabsClosable(false);
    }
    delete tab;
}

void WorkTabMgr::OpenHomeTab()
{
    hometab = new HomeTab(tabWidget);
    tabWidget->addTab(hometab, "Home");
}

void WorkTabMgr::CloseHomeTab()
{
    if(!hometab)
        return;
    tabWidget->removeTab(tabWidget->indexOf(hometab));
    delete hometab;
    hometab = NULL;
}

void WorkTabMgr::NewTabDialog()
{
    TabDialog *dialog = new TabDialog((QWidget*)tabWidget->parent());
    dialog->exec();
    delete dialog;
}
