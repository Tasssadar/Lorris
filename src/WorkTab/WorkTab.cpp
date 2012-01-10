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

#include "WorkTab.h"
#include <QLayout>

WorkTab::WorkTab() : QWidget(NULL)
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
