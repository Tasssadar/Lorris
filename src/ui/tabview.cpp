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

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "tabview.h"
#include "WorkTab/WorkTabMgr.h"

TabView::TabView(QWidget *parent) :
    QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    m_layouts.insert(layout);

    m_active_widget = newTabWidget(layout);

    connect(qApp,SIGNAL(focusChanged(QWidget*,QWidget*)), SLOT(focusChanged(QWidget*,QWidget*)));
}

TabWidget *TabView::newTabWidget(QLayout *l)
{
    TabWidget *tabW = new TabWidget(sWorkTabMgr.generateNewWidgetId(), this);
    m_tab_widgets.insert(tabW->getId(), tabW);

    l->addWidget(tabW);

    return tabW;
}

void TabView::focusChanged(QWidget */*prev*/, QWidget *now)
{
    if(!now || !now->inherits("QTabWidget"))
        return;

    m_active_widget = (TabWidget*)now;
}
