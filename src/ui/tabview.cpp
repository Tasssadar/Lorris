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
#include <QFrame>
#include <QMouseEvent>

#include "tabview.h"
#include "WorkTab/WorkTabMgr.h"

#define LAYOUT_MARGIN 4
TabView::TabView(QWidget *parent) :
    QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    m_layouts.insert(layout);
    layout->setMargin(LAYOUT_MARGIN);

    m_active_widget = newTabWidget(layout);
}

TabWidget *TabView::newTabWidget(QBoxLayout *l)
{
    TabWidget *tabW = new TabWidget(sWorkTabMgr.generateNewWidgetId(), this);
    m_tab_widgets.insert(tabW->getId(), tabW);

    l->addWidget(tabW, 50);

    connect(tabW, SIGNAL(newTab()),                       SIGNAL(newTab()));
    connect(tabW, SIGNAL(openHomeTab(quint32)),           SIGNAL(openHomeTab(quint32)));
    connect(tabW, SIGNAL(changeMenu(quint32)),            SIGNAL(changeMenu(quint32)));
    connect(tabW, SIGNAL(split(bool,int)),                SLOT(split(bool,int)));
    connect(tabW, SIGNAL(removeWidget(quint32)),          SLOT(removeWidget(quint32)));
    connect(tabW, SIGNAL(changeActiveWidget(TabWidget*)), SLOT(changeActiveWidget(TabWidget*)));

    updateResizeLines((QBoxLayout*)layout());
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

    updateResizeLines((QBoxLayout*)layout());

    for(std::set<QBoxLayout*>::iterator itr = m_layouts.begin(); itr != m_layouts.end();)
    {
        QBoxLayout *l = *itr;
        if(l->count() == 0)
        {
            m_layouts.erase(itr);
            delete l;
            itr = m_layouts.begin();
        }
        else
            ++itr;
    }
}

void TabView::split(bool horizontal, int index)
{
    Q_ASSERT(sender());

    TabWidget *widget = (TabWidget*)sender();

    QBoxLayout *l = NULL;

    for(std::set<QBoxLayout*>::iterator itr = m_layouts.begin(); !l && itr != m_layouts.end(); ++itr)
        if((*itr)->indexOf(widget) != -1)
            l = *itr;

    if(!l)
        return;

    if((horizontal && !l->inherits("QVBoxLayout")) || (!horizontal && !l->inherits("QHBoxLayout")))
    {
        if(l->count() == 1)
        {
            bool setAsMain = (layout() == l);
            QBoxLayout *parentLayout = (QBoxLayout*)l->parent();
            int idx = -1;
            if(!setAsMain)
            {
                for(int i = 0; idx == -1 && i < parentLayout->count(); ++i)
                    if(parentLayout->itemAt(i)->layout() == l)
                        idx = i;
            }

            l->removeWidget(widget);
            m_layouts.erase(l);
            delete l;

            if(horizontal) l = new QVBoxLayout();
            else           l = new QHBoxLayout();

            if(setAsMain)
                setLayout(l);
            else
                parentLayout->insertLayout(idx, l, 50);

            m_layouts.insert(l);
            l->setMargin(setAsMain ? LAYOUT_MARGIN : 0);

            l->addWidget(widget, 50);
        }
        else
        {
            int pos = l->indexOf(widget);
            l->removeWidget(widget);

            QBoxLayout *newLayout = NULL;
            if(horizontal) newLayout = new QVBoxLayout();
            else           newLayout = new QHBoxLayout();

            newLayout->addWidget(widget, 50);
            l->insertLayout(pos, newLayout, 50);

            m_layouts.insert(newLayout);
            l = newLayout;
            l->setMargin(0);
        }
    }

    TabWidget *second = newTabWidget(l);
    second->pullTab(index, widget);

    updateResizeLines((QBoxLayout*)layout());
}

void TabView::updateResizeLines(QBoxLayout *l)
{
    QLayoutItem *prevItem = NULL;
    QLayoutItem *curItem = NULL;
    int count = l->count();
    for(int i = 0; i < count; ++i)
    {
        curItem = l->itemAt(i);

        if(curItem->layout())
            updateResizeLines((QBoxLayout*)curItem->layout());

        // Remove ResizeLine if there are two in a row or if it is the first or last item
        if(isResizeLine(curItem) && (!prevItem || i+1 >= count || isResizeLine(prevItem)))
        {
            ResizeLine *line = (ResizeLine*)curItem->widget();
            l->removeWidget(line);
            m_resize_lines.remove(line);
            delete line;
            goto restart_loop;
        }

        if(prevItem && !isResizeLine(curItem) && !isResizeLine(prevItem))
        {
            newResizeLine(l, i);
            goto restart_loop;
        }

        prevItem = curItem;
        continue;

restart_loop:
        i = -1;
        prevItem = curItem = NULL;
        count = l->count();
    }
}

void TabView::newResizeLine(QBoxLayout *l, int idx)
{
    ResizeLine *line = new ResizeLine(l->inherits("QHBoxLayout"), this);
    l->insertWidget(idx, line, 1);
    m_resize_lines.insert(line, l);
    line->updateStretch();
}

bool TabView::isResizeLine(QLayoutItem *item)
{
    return (item && item->widget() && item->widget()->inherits("ResizeLine"));
}

QBoxLayout *TabView::getLayoutForLine(ResizeLine *line)
{
    QHash<ResizeLine*, QBoxLayout*>::iterator itr = m_resize_lines.find(line);
    if(itr != m_resize_lines.end())
        return *itr;
    return NULL;
}

ResizeLine::ResizeLine(bool vertical, TabView *parent) : QFrame(parent)
{
    m_vertical = vertical;
    m_cur_stretch = 50;
    m_tab_view = parent;
    m_resize_layout = NULL;

    setFrameStyle((vertical ? QFrame::VLine : QFrame::HLine) | QFrame::Plain);

    if(vertical)
        setSizeIncrement(QSizePolicy::Fixed, QSizePolicy::Expanding);
    else
        setSizeIncrement(QSizePolicy::Expanding, QSizePolicy::Fixed);

    setCursor(vertical ? Qt::SizeHorCursor : Qt::SizeVerCursor);
}

void ResizeLine::updateStretch()
{
    QBoxLayout *l= m_tab_view->getLayoutForLine(this);
    Q_ASSERT(l);

    if(l)
    {
        int index = l->indexOf(this);
        if(index < 1)
        {
            Q_ASSERT(false);
            return;
        }
        m_cur_stretch = l->stretch(index-1);
    }
}

void ResizeLine::mousePressEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton)
        return QFrame::mousePressEvent(event);

    event->accept();

    m_resize_layout = m_tab_view->getLayoutForLine(this);
    Q_ASSERT(m_resize_layout);

    m_mouse_pos = event->globalPos();

    if(m_resize_layout)
    {
        int index = m_resize_layout->indexOf(this);
        if(index < 1)
        {
            Q_ASSERT(false);
            m_resize_layout = NULL;
            return;
        }

        m_resize_index = index;

        QLayoutItem *item = m_resize_layout->itemAt(index-1);
        if(item->widget()) m_resize_pos[0] = item->widget()->pos();
        else               m_resize_pos[0] = item->layout()->geometry().topLeft();

        item = m_resize_layout->itemAt(index+1);
        if(item->widget())
        {
            m_resize_pos[1] = item->widget()->pos();
            m_resize_pos[1].rx() += item->widget()->width();
            m_resize_pos[1].ry() += item->widget()->height();
        }
        else
            m_resize_pos[1] = item->layout()->geometry().bottomRight();
    }
}

void ResizeLine::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton)
        return QFrame::mouseReleaseEvent(event);

    event->accept();

    m_resize_layout = NULL;
}

void ResizeLine::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_resize_layout)
        return QFrame::mouseMoveEvent(event);

    event->accept();

    QPoint mouse = event->globalPos();

    float dist, move;
    if(m_vertical)
    {
        move = (mouse - m_mouse_pos).x();
        dist = (m_resize_pos[1] - m_resize_pos[0]).x();
    }
    else
    {
        move = (mouse - m_mouse_pos).y();
        dist = (m_resize_pos[1] - m_resize_pos[0]).y();
    }

    m_cur_stretch += move / (dist / 100.f);

    if(m_cur_stretch > 100)    m_cur_stretch = 100;
    else if(m_cur_stretch < 0) m_cur_stretch = 0;

    m_resize_layout->setStretch(m_resize_index-1, (int)m_cur_stretch);
    m_resize_layout->setStretch(m_resize_index+1, 100 - (int)m_cur_stretch);

    m_mouse_pos = event->globalPos();
}
