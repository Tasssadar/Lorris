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

#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include "ScriptWidget/scriptwidget.h"

class TerminalWidget : public ScriptWidget
{
    Q_OBJECT
public:
    TerminalWidget(QWidget *parent = 0);
    ~TerminalWidget();

    void setUp(Storage *storage);
};

class TerminalWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    TerminalWidgetAddBtn(QWidget *parent = 0);

};
#endif // TERMINALWIDGET_H
