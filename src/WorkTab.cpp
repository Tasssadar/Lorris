#include "WorkTab.h"

WorkTab::WorkTab() : QObject(NULL)
{
    m_con = NULL;
}

WorkTab::~WorkTab()
{
    if(m_con)
    {
        m_con->Close();
        delete m_con;
    }
}

QWidget *WorkTab::GetTab(QWidget *parent)
{
    return NULL;
}

void WorkTab::readData(QByteArray data)
{
    data.contains("a");
}
