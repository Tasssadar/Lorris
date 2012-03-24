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

#ifndef MAINTABWIDGET_H
#define MAINTABWIDGET_H

#include <QTabWidget>
#include <QTabBar>

class TabWidget : public QTabWidget
{
    Q_OBJECT

Q_SIGNALS:
    void newTab();

public:
    explicit TabWidget(quint32 id, QWidget *parent = 0);

    quint32 getId() const { return m_id; }

protected:
    void mousePressEvent(QMouseEvent *ev);
    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    bool checkEvent(QMouseEvent *event);

    quint32 m_id;
};

class TabBar : public QTabBar
{
    Q_OBJECT

public:
    explicit TabBar(QWidget * parent = 0);

protected:
    void mousePressEvent(QMouseEvent * event);
};

#endif // MAINTABWIDGET_H
