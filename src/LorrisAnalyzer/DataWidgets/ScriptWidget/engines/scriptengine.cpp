/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifdef WITH_PYTHON
    #include "pythonengine.h"
#endif

#include <QTimer>

#include "../../../widgetarea.h"
#include "../../datawidget.h"
#include "scriptengine.h"
#include "../scriptstorage.h"
#include "../scriptwidget.h"

#include "qtscriptengine.h"

ScriptEngine::ScriptEngine(WidgetArea *area, quint32 w_id, ScriptWidget *parent) :
    QObject(parent)
{
    m_x = m_y = 0;
    m_storage = new ScriptStorage(this);
    m_area = area;
    m_widget_id = w_id;
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

ScriptEngine *ScriptEngine::getEngine(int idx, WidgetArea *area, quint32 w_id, ScriptWidget *parent)
{
    switch(idx)
    {
        case ENGINE_QTSCRIPT: return new QtScriptEngine(area, w_id, parent);
#ifdef WITH_PYTHON
        case ENGINE_PYTHON: return new PythonEngine(area, w_id, parent);
#endif
    }
    return NULL;
}

QString ScriptEngine::sanitizeWidgetName(QString const & name)
{
    if (name.isEmpty())
        return QString();

    if (!name[0].isLetter() && name[0] != '_')
       return QString();

    for (int i = 1; i < name.size(); ++i)
    {
        if (!name[i].isLetterOrNumber() && name[i] != '_')
            return QString();
    }

    return name;
}
