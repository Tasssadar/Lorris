/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QScriptValue>

#include "scriptagent.h"
#include "qtscriptengine.h"
#include "../../../../common.h"

ScriptAgent::ScriptAgent(QtScriptEngine *qtengine, QScriptEngine *engine) :
    QScriptEngineAgent(engine)
{
    m_timer.invalidate();
    m_errors = 0;
    m_engine = qtengine;
}

void ScriptAgent::exceptionThrow(qint64 /*scriptid*/, const QScriptValue &exception, bool hasHandler)
{
    if(hasHandler)
        return;

    if((!m_timer.isValid() || m_timer.elapsed() > 1000) && m_errors < 3)
    {
        ++m_errors;
        m_timer.invalidate();
        m_engine->handleException(QObject::tr("Uncaught exception: ") + exception.toString());
        m_timer.restart();
    }
}

void ScriptAgent::scriptLoad(qint64, const QString &, const QString &, int)
{
    m_timer.invalidate();
    m_errors = 0;
}
