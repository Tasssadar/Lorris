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
#include <QApplication>

#include "tabview.h"
#include "WorkTab/WorkTabMgr.h"

TabView::TabView(QWidget *parent) :
    QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    m_layouts.insert(layout);

    m_active_widget = newTabWidget(layout);
}

TabWidget *TabView::newTabWidget(QLayout *l)
{
    TabWidget *tabW = new TabWidget(sWorkTabMgr.generateNewWidgetId(), this);
    m_tab_widgets.insert(tabW->getId(), tabW);

    l->addWidget(tabW);

    connect(tabW, SIGNAL(newTab()),                       SIGNAL(newTab()));
    connect(tabW, SIGNAL(openHomeTab(quint32)),           SIGNAL(openHomeTab(quint32)));
    connect(tabW, SIGNAL(split(bool,int)),                SLOT(split(bool,int)));
    connect(tabW, SIGNAL(removeWidget(quint32)),          SLOT(removeWidget(quint32)));
    connect(tabW, SIGNAL(changeActiveWidget(TabWidget*)), SLOT(changeActiveWidget(TabWidget*)));

    return tabW;
}

void TabView::changeActiveWidget(TabWidget *widget)
{
    m_active_widget = widget;
}

void TabView::removeWidget(quint32 id)
{
    QHash<quint32, TabWidget*>::iterator itr = m_tab_widgets.find(id);
    if(itr == m_tab_widgets.end())
        return;

    if(m_active_widget == *itr)
        m_active_widget = m_tab_widgets[0];

    delete *itr;
    m_tab_widgets.erase(itr);
}

void TabView::split(bool horizontal, int index)
{
    Q_ASSERT(sender());

    TabWidget *widget = (TabWidget*)sender();

    QLayout *l = NULL;

    for(std::set<QLayout*>::iterator itr = m_layouts.begin(); !l && itr != m_layouts.end(); ++itr)
        if((*itr)->indexOf(widget) != -1)
            l = *itr;

    if(!l)
        return;

    if((horizontal && !l->inherits("QVBoxLayout")) || (!horizontal && !l->inherits("QHBoxLayout")))
    {
        if(l->count() == 1)
        {
            bool setAsMain = (layout() == l);

            l->removeWidget(widget);
            m_layouts.erase(l);
            delete l;

            if(horizontal) l = new QVBoxLayout();
            else           l = new QHBoxLayout();

            if(setAsMain)
                setLayout(l);

            m_layouts.insert(l);

            l->addWidget(widget);
        }
        else
        {
            int pos = l->indexOf(widget);
            l->removeWidget(widget);

            QBoxLayout *newLayout = NULL;
            if(horizontal) newLayout = new QVBoxLayout();
            else           newLayout = new QHBoxLayout();

            newLayout->addWidget(widget);
            ((QBoxLayout*)l)->insertLayout(pos, newLayout);

            m_layouts.insert(newLayout);
            l = newLayout;
        }
    }

    TabWidget *second = newTabWidget(l);
    second->pullTab(index, widget);
}
