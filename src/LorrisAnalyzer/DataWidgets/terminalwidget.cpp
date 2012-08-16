/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "ScriptWidget/engines/qtscriptengine.h"
#include "terminalwidget.h"

REGISTER_DATAWIDGET(WIDGET_TERMINAL, Terminal)

TerminalWidget::TerminalWidget(QWidget *parent) : ScriptWidget(parent)
{
    setTitle(tr("Terminal"));
    setIcon(":/dataWidgetIcons/terminal.png");

    m_widgetType = WIDGET_TERMINAL;
}

void TerminalWidget::setUp(Storage *storage)
{
    ScriptWidget::setUp(storage);

    static const QString append[ENGINE_MAX] = {
        "js", // ENGINE_QTSCRIPT
        "py"  // ENGINE_PYTHON
    };

    QFile f(":/examples/terminal." + append[m_engine_type]);
    if(!f.open(QIODevice::ReadOnly))
        return;

    m_engine->setSource(QString::fromUtf8(f.readAll()));
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
