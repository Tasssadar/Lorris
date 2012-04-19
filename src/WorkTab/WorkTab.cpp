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
    m_con = NULL;
    m_id = 0;
    m_info = NULL;
}

WorkTab::~WorkTab()
{
    if(m_con)
        m_con->release();
}

void WorkTab::setConnection(Connection *con)
{
    if (m_con)
    {
        disconnect(m_con, 0, this, 0);
        m_con->release();
    }

    m_con = con;

    if(!con)
        return;

    connect(m_con, SIGNAL(dataRead(QByteArray)), this, SLOT(readData(QByteArray)));
    connect(m_con, SIGNAL(connected(bool)), this, SLOT(connectedStatus(bool)));
    connect(m_con, SIGNAL(destroyed()), this, SLOT(connectionDestroyed()));
    m_con->addRef();
}

void WorkTab::connectionDestroyed()
{
    m_con = 0;
}

void WorkTab::readData(const QByteArray& /*data*/)
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
