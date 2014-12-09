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
    m_enterFuncCnt = 0;
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
    m_enterFuncCnt = 0;
}

void ScriptAgent::functionEntry(qint64 scriptId)
{
    if(++m_enterFuncCnt == 1)
        ((QtScriptEngine_private*)engine())->startFreezeDetector();
}

void ScriptAgent::functionExit(qint64 scriptId, const QScriptValue &returnValue)
{
    if(--m_enterFuncCnt == 0)
        ((QtScriptEngine_private*)engine())->stopFreezeDetector();
}
