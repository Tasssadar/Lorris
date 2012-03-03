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

#include <QScriptValue>

#include "scriptagent.h"
#include "common.h"

ScriptAgent::ScriptAgent(QScriptEngine *engine) :
    QScriptEngineAgent(engine)
{
    m_timer.invalidate();
    m_errors = 0;
}

void ScriptAgent::exceptionThrow(qint64 /*scriptid*/, const QScriptValue &exception, bool hasHandler)
{
    if(hasHandler)
        return;

    if((!m_timer.isValid() || m_timer.elapsed() > 1000) && m_errors < 3)
    {
        ++m_errors;
        m_timer.invalidate();

        Utils::ThrowException(QObject::tr("Uncaught exception: ") + exception.toString());

        m_timer.restart();
    }
}

void ScriptAgent::scriptLoad(qint64, const QString &, const QString &, int)
{
    m_timer.invalidate();
    m_errors = 0;
}

