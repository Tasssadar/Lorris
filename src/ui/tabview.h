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

#ifndef TABVIEW_H
#define TABVIEW_H

#include <QFrame>
#include <set>
#include <QHash>

#include "tabwidget.h"

class QLayoutItem;
class QBoxLayout;
class ResizeLine;

class TabView : public QWidget
{
    Q_OBJECT

Q_SIGNALS:
    void newTab();
    void openHomeTab(quint32 id);
    void changeMenu(quint32 id);

public:
    explicit TabView(QWidget *parent = 0);

    TabWidget *getActiveWidget() const
    {
        Q_ASSERT(m_active_widget);
        return m_active_widget;
    }

    TabWidget *getWidget(quint32 id)
    {
        if(m_tab_widgets.contains(id))
            return m_tab_widgets[id];
        return NULL;
    }

    QBoxLayout *getLayoutForLine(ResizeLine *line);

private slots:
    void split(bool horizontal, int index);
    void removeWidget(quint32 id);
    void changeActiveWidget(TabWidget *widget);

private:
    TabWidget *newTabWidget(QBoxLayout *l);
    void updateSize(QBoxLayout *l);
    inline bool isResizeLine(QLayoutItem *item);
    void updateResizeLines(QBoxLayout *l);
    inline void newResizeLine(QBoxLayout *l, int idx);

    QHash<quint32, TabWidget*> m_tab_widgets;
    QHash<ResizeLine*, QBoxLayout*> m_resize_lines;
    std::set<QBoxLayout*> m_layouts;

    TabWidget *m_active_widget;
};

class ResizeLine : public QFrame
{
    Q_OBJECT
public:
    ResizeLine(bool vertical, TabView *parent);

    void updateStretch();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    bool m_vertical;
    float m_cur_stretch;
    TabView *m_tab_view;
    QBoxLayout *m_resize_layout;
    QPoint m_resize_pos[2];
    QPoint m_mouse_pos;
    int m_resize_index;
};

#endif // TABVIEW_H
