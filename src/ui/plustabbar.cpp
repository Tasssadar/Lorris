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
#include <QStyle>
#include <QStyleOption>

#include "plustabbar.h"

PlusTabBar::PlusTabBar(QWidget *parent) :
    QTabBar(parent)
{
    m_plusRect = QRect(2, 4, 16, 16);
    m_disabled = false;
    m_hover = false;

    QIcon icon(":/icons/icons/list-add.png");
    m_pixmap = icon.pixmap(16, 16);

    icon = QIcon(":/icons/icons/list-add-glow.png");
    m_hover_pixmap = icon.pixmap(16, 16);

    setMouseTracking(true);
}

void PlusTabBar::updateRect()
{
    if(count() == 0)
        m_plusRect.moveLeft(2);
    else
    {
        QRect rect = tabRect(count()-1);
        m_plusRect.moveLeft(rect.x()+rect.width()+2);

        QStyleOptionTabV3 tab;
        initStyleOption(&tab, count()-1);
        rect = style()->subElementRect(QStyle::SE_TabBarTabText, &tab, this);
        m_plusRect.moveTop(rect.y() + ((rect.height() - m_plusRect.height()) / 2));
    }
}

void PlusTabBar::tabLayoutChange()
{
    updateRect();
}

void PlusTabBar::paintEvent(QPaintEvent *event)
{
    QTabBar::paintEvent(event);

    QPainter painter(this);
    painter.drawPixmap(m_plusRect, getMap());
}

void PlusTabBar::mousePressEvent(QMouseEvent *event)
{
    if(m_disabled || event->button() != Qt::LeftButton || !m_plusRect.contains(event->pos()))
        return QTabBar::mousePressEvent(event);

    event->accept();
    emit plusPressed();
}

void PlusTabBar::mouseMoveEvent(QMouseEvent *event)
{
    if(m_disabled)
        return QTabBar::mouseMoveEvent(event);

    bool in = m_plusRect.contains(event->pos());
    if(in != m_hover)
    {
        m_hover = in;
        update();
    }
    QTabBar::mouseMoveEvent(event);
}

void PlusTabBar::leaveEvent(QEvent *event)
{
    if(m_hover)
    {
        m_hover = false;
        update();
    }
    QTabBar::leaveEvent(event);
}

void PlusTabBar::setDisablePlus(bool disable)
{
    if(disable == m_disabled)
        return;

    m_disabled = disable;

    if(disable)
        m_hover = false;

    QIcon icon(":/icons/icons/list-add.png");
    m_pixmap = icon.pixmap(16, 16, disable ? QIcon::Disabled : QIcon::Normal);
    update();
}
