/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QTimer>

#include "../../../widgetarea.h"
#include "../../datawidget.h"
#include "scriptengine.h"
#include "../scriptstorage.h"

#include "qtscriptengine.h"

#ifdef WITH_PYTHON
    #include "pythonengine.h"
#endif

ScriptEngine::ScriptEngine(WidgetArea *area, quint32 w_id, Terminal *terminal, QObject *parent) :
    QObject(parent)
{
    m_x = m_y = 0;
    m_storage = new ScriptStorage(this);
    m_area = area;
    m_widget_id = w_id;
    m_terminal = terminal;
}

ScriptEngine::~ScriptEngine()
{
    delete m_storage;
}


QStringList ScriptEngine::getEngineList()
{
#ifdef WITH_PYTHON
    static const QStringList list = (QStringList() << "QtScript" << "Python 2.7");
#else
    static const QStringList list = (QStringList() << "QtScript");
#endif
    return list;
}

ScriptEngine *ScriptEngine::getEngine(int idx, WidgetArea *area, quint32 w_id, Terminal *terminal, QObject *parent)
{
    switch(idx)
    {
        case ENGINE_QTSCRIPT: return new QtScriptEngine(area, w_id, terminal, parent);
#ifdef WITH_PYTHON
        case ENGINE_PYTHON: return new PythonEngine(area, w_id, terminal, parent);
#endif
    }
    return NULL;
}
