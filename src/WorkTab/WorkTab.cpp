/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QLayout>
#include <QMessageBox>

#include "WorkTab.h"

WorkTab::WorkTab() : QWidget(NULL)
{
    m_id = 0;
    m_info = NULL;
}

WorkTab::~WorkTab()
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
            continue;
        }
        else if(item->widget())
            delete item->widget();
        delete item;
    }
}

void WorkTab::onTabShow(const QString &)
{

}

bool WorkTab::onTabClose()
{
    return true;
}

void WorkTab::addTopMenu(QMenu *menu)
{
    m_menus.push_back(menu);
}

void WorkTab::openFile(const QString &/*filename*/)
{

}

//----------------------------------------------------------------------------
PortConnWorkTab::PortConnWorkTab()
{
}

PortConnWorkTab::~PortConnWorkTab()
{
    if (m_con)
        m_con->releaseTab();
}

void PortConnWorkTab::setConnection(ConnectionPointer<Connection> const & con)
{
    this->setPortConnection(con.dynamicCast<PortConnection>());
}

void PortConnWorkTab::setPortConnection(ConnectionPointer<PortConnection> const & con)
{
    if (m_con)
    {
        disconnect(m_con.data(), 0, this, 0);
        m_con->releaseTab();
    }

    m_con = con;

    if(m_con)
    {
        connect(m_con.data(), SIGNAL(dataRead(QByteArray)), this, SLOT(readData(QByteArray)));
        connect(m_con.data(), SIGNAL(connected(bool)), this, SLOT(connectedStatus(bool)));
        m_con->addTabRef();
    }
}

void PortConnWorkTab::readData(const QByteArray& /*data*/)
{

}

void PortConnWorkTab::connectedStatus(bool /*connected*/)
{

}
