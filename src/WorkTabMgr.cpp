#include "WorkTabMgr.h"
#include "WorkTabInfo.h"
#include "WorkTab.h"

#include "LorrisProbe/lorrisprobeinfo.h"
#include "LorrisTerminal/lorristerminalinfo.h"

WorkTabMgr::WorkTabMgr()
{
    //put ALL plugins into this vector
    m_workTabInfos.push_back(new LorrisProbeInfo);
    m_workTabInfos.push_back(new LorrisTerminalInfo);
    tabIdCounter = 0;
}

WorkTabMgr::~WorkTabMgr()
{
    for(uint8_t itr = 0; itr < m_workTabInfos.size(); ++itr)
        delete m_workTabInfos[itr];
}

std::vector<WorkTabInfo*> *WorkTabMgr::GetWorkTabInfos()
{
    return &m_workTabInfos;
}

uint16_t WorkTabMgr::AddWorkTab(WorkTab *tab)
{
    uint16_t id = tabIdCounter++;
    m_workTabs.insert(std::make_pair<uint16_t, WorkTab*>(id, tab));
    tab->setId(id);
    return id;
}
