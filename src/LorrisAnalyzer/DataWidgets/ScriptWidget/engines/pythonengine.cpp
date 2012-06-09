/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "pythonengine.h"
#include "../../../../shared/terminal.h"

QString PythonEngine::getNewModuleName()
{
    static int i = 0;
    return QString::number(i++);
}

PythonEngine::PythonEngine(WidgetArea *area, quint32 w_id, Terminal *terminal, QObject *parent) :
    ScriptEngine(area, w_id, terminal, parent)
{
    static bool initialized = false;
    if(!initialized)
    {
        PythonQt::init(PythonQt::IgnoreSiteModule);
        initialized = true;
    }
}

void PythonEngine::setSource(const QString &source)
{
    m_source = source;

    m_module = PythonQt::self()->createModuleFromScript(getNewModuleName());
    m_module.addObject("terminal", m_terminal);
    m_module.evalScript(source);
}

QString PythonEngine::dataChanged(analyzer_data *data, quint32 index)
{
    return QString();
}

void PythonEngine::onWidgetAdd(DataWidget *w)
{

}

void PythonEngine::onWidgetRemove(DataWidget *w)
{

}

void PythonEngine::callEventHandler(const QString& eventId)
{

}

void PythonEngine::onSave()
{

}

void PythonEngine::keyPressed(const QString &key)
{

}


