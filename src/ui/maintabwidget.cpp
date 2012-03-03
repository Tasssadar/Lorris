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

#include "maintabwidget.h"

MainTabWidget::MainTabWidget(QWidget *parent) :
    QTabWidget(parent)
{
    setTabBar(new MainTabBar(this));
}

void MainTabWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() != Qt::MidButton || !checkEvent(event))
        return QTabWidget::mousePressEvent(event);
}

void MainTabWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton || !checkEvent(event))
        return QTabWidget::mouseDoubleClickEvent(event);
}

bool MainTabWidget::checkEvent(QMouseEvent *event)
{
    if(event->pos().y() > tabBar()->height())
        return false;

    emit newTab();
    return true;
}

MainTabBar::MainTabBar(QWidget *parent) :
    QTabBar(parent)
{
}

void MainTabBar::mousePressEvent(QMouseEvent *event)
{
    if(event->button() != Qt::MidButton)
        return QTabBar::mousePressEvent(event);

    int tab = tabAt(event->pos());

    if(tab != -1)
        emit tabCloseRequested(tab);
}
