/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "ScriptWidget/engines/qtscriptengine.h"
#include "terminalwidget.h"

#include "../../ui/terminal.h"

REGISTER_DATAWIDGET(WIDGET_TERMINAL, Terminal, NULL)
W_TR(QT_TRANSLATE_NOOP("DataWidget", "Terminal"))

TerminalWidget::TerminalWidget(QWidget *parent) : ScriptWidget(parent)
{

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

void TerminalWidget::newData(analyzer_data *data, quint32 index)
{
    if(this->isAssigned()) {
        QString str = data->getString(m_info.pos);
        if(!str.isEmpty())
            m_terminal->appendText(str);
    } else {
        ScriptWidget::newData(data, index);
    }
}
