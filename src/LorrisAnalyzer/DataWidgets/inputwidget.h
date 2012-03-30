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

#ifndef INPUTWIDGET_H
#define INPUTWIDGET_H

#include <QLineEdit>
#include <QHash>

#include "datawidget.h"

class QUiLoader;

class InputWidget : public DataWidget
{
    Q_OBJECT
public:
    typedef QWidget *(*QWidgetFc)(QWidget*);

    InputWidget(QWidget *parent = 0);
    ~InputWidget();

public slots:
    QWidget *newWidget(const QString& name, int stretch = 0);

};

#endif // INPUTWIDGET_H
