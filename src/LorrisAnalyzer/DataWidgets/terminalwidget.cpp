/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "ScriptWidget/scriptenv.h"
#include "terminalwidget.h"

TerminalWidget::TerminalWidget(QWidget *parent) : ScriptWidget(parent)
{
    setTitle(tr("Terminal"));
    setIcon(":/dataWidgetIcons/terminal.png");

    m_widgetType = WIDGET_TERMINAL;
}

void TerminalWidget::setUp(Storage *storage)
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
