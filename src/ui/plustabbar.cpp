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

#include <QPainter>
#include <QPaintEvent>
#include <QIcon>
#include "plustabbar.h"

PlusTabBar::PlusTabBar(QWidget *parent) :
    QTabBar(parent)
{
    m_plusRect = QRect(2, 3, 16, 16);
}

void PlusTabBar::updateRect()
{
    if(count() == 0)
        m_plusRect = QRect(2, 3, 16, 16);
    else
    {
        QRect rect = tabRect(count()-1);
        m_plusRect.moveLeft(rect.x()+rect.width()+2);
    }
}

void PlusTabBar::paintEvent(QPaintEvent *event)
{
    QTabBar::paintEvent(event);

    QPainter painter(this);

    static QPixmap map;
    if(map.isNull())
    {
        QIcon icon(":/icons/icons/list-add.png");
        map = icon.pixmap(16, 16);
    }
    updateRect();
    painter.drawPixmap(m_plusRect, map);
}

void PlusTabBar::mousePressEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton || !m_plusRect.contains(event->pos()))
        return QTabBar::mousePressEvent(event);

    event->accept();
    emit plusPressed();
}
