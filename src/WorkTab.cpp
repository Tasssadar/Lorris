#include "WorkTab.h"
#include <QLayout>

WorkTab::WorkTab() : QObject(NULL)
{
    m_con = NULL;
    m_id = 0;
}

WorkTab::~WorkTab()
{
    if(m_con)
    {
        m_con->RemoveUsingTab(m_id);
        if(!m_con->IsUsedByTab())
        {
            m_con->Close();
            delete m_con;
        }
    }
}

QWidget *WorkTab::GetTab(QWidget *parent)
{
    return NULL;
}

void WorkTab::readData(QByteArray data)
{

}


void WorkTab::DeleteAllMembers(QLayout *layout)
{
    while(layout->count())
    {
        if(layout->itemAt(0)->layout())
        {
            WorkTab::DeleteAllMembers(layout->itemAt(0)->layout());
            delete layout->itemAt(0)->layout();
        }
        else
            delete layout->itemAt(0)->widget();
        layout->removeItem(layout->itemAt(0));
    }
}
