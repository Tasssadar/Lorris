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

#include <QSpacerItem>
#include <QtUiTools/QUiLoader>

#include "inputwidget.h"

InputWidget::InputWidget(QWidget *parent) :
    DataWidget(parent)
{
    m_widgetType = WIDGET_INPUT;

    setTitle(tr("Input"));
    setIcon(":/dataWidgetIcons/input.png");

    adjustSize();
    setMinimumSize(width(), width());

    layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
    layout->setContentsMargins(5, 0, 5, 5);

    m_loader = new QUiLoader(this);
}

InputWidget::~InputWidget()
{
    delete m_loader;
}

QWidget *InputWidget::newWidget(const QString &name, int stretch)
{
    QWidget *w = m_loader->createWidget(name, this);
    if(!w)
        return NULL;

    layout->addWidget(w, stretch);

    return w;
}
