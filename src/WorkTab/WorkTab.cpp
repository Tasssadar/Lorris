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

void WorkTab::onTabShow()
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
