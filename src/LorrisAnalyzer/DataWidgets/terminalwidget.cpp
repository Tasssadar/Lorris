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

#include "ScriptWidget/scriptenv.h"
#include "terminalwidget.h"

TerminalWidget::TerminalWidget(QWidget *parent) : ScriptWidget(parent)
{
    setTitle(tr("Terminal"));
    setIcon(":/dataWidgetIcons/terminal.png");

    m_widgetType = WIDGET_TERMINAL;
}

void TerminalWidget::setUp(AnalyzerDataStorage *storage)
{
    ScriptWidget::setUp(storage);

    m_env->setSource("function onDataChanged(data, dev, cmd, index) {\n"
                     "    appendTerm(data);\n"
                     "}\n"
                     "\n"
                     "function onKeyPress(key) {\n"
                     "    sendData(new Array(key.charCodeAt(0)));\n"
                     "}\n");
}

TerminalWidget::~TerminalWidget()
{

}

TerminalWidgetAddBtn::TerminalWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Terminal"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/terminal.png"));

    m_widgetType = WIDGET_TERMINAL;
}
