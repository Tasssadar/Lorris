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

QWidget *WorkTab::GetTab(QWidget */*parent*/)
{
    return NULL;
}

void WorkTab::readData(QByteArray /*data*/)
{

}

void WorkTab::connectedStatus(bool /*connected*/)
{

}

void WorkTab::DeleteAllMembers(QLayout *layout)
{
    while(layout->count())
    {
        QLayoutItem *item = layout->itemAt(0);
        layout->removeItem(item);
        if(item->layout())
        {
            WorkTab::DeleteAllMembers(item->layout());
            delete item->layout();
        }
        else if(item->widget())
        {
            delete item->widget();
            delete item;
        }
    }
}
