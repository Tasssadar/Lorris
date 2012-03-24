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

#include <QMouseEvent>
#include <QInputDialog>
#include <QMenu>

#include "tabwidget.h"

TabWidget::TabWidget(quint32 id, QWidget *parent) :
    QTabWidget(parent)
{
    m_id = id;

    setTabBar(new TabBar(this));
    setMovable(true);

    QPushButton* newTabBtn = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogNewFolder), "", this);
    connect(newTabBtn, SIGNAL(clicked()), SIGNAL(newTab()));

    setCornerWidget(newTabBtn);
}

void TabWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() != Qt::MidButton || !checkEvent(event))
        return QTabWidget::mousePressEvent(event);
}

void TabWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton || !checkEvent(event))
        return QTabWidget::mouseDoubleClickEvent(event);
}

bool TabWidget::checkEvent(QMouseEvent *event)
{
    if(event->pos().y() > tabBar()->height())
        return false;

    emit newTab();
    return true;
}

TabBar::TabBar(QWidget *parent) :
    QTabBar(parent)
{
}

void TabBar::mousePressEvent(QMouseEvent *event)
{
    switch(event->button())
    {
        case Qt::MidButton:
        {
            int tab = tabAt(event->pos());

            if(tab != -1)
                emit tabCloseRequested(tab);
            break;
        }
        case Qt::RightButton:
        {
            int tab = tabAt(event->pos());
            if(tab == -1 || tabText(tab) == "Home")
                break;

            QMenu menu(this);
            menu.addAction(tr("Rename..."));
            QAction *act = menu.exec(event->globalPos());
            if(act)
            {
                QString name = QInputDialog::getText(this, tr("Rename tab"), tr("New name:"),
                                                     QLineEdit::Normal, tabText(tab));
                if(!name.isEmpty())
                    setTabText(tab, name);
            }
            break;
        }
        default:
            QTabBar::mousePressEvent(event);
            break;
    }
}
