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

#include <QStringList>
#include <QScriptValueIterator>
#include <QScriptValueList>
#include <QScriptEngineAgent>
#include <QDebug>
#include "scriptenv.h"

ScriptEnv::ScriptEnv(QObject *parent) :
    QScriptEngine(parent)
{
    pushContext();
    m_source =
            tr("// You can use clearTerm() and appendTerm(string) to set term content\n"
            "// You can use sendData(Array of ints) to send data to device. It expects array of uint8s\n\n"
            "// This function gets called on data received\n"
            "// it should return string, which is automatically appended to terminal\n"
            "function onDataChanged(data, dev, cmd, index) {\n"
            "    return \"\";\n"
            "}\n\n"
            "// This function is called on key press in terminal.\n"
            "// Param is string\n"
            "function onKeyPress(key) {\n"
            "    \n"
            "}\n");
}

void ScriptEnv::prepareNewContext()
{
    QScriptContext *context = pushContext();
    // Functions
    QScriptValue clearTerm = newFunction(&__clearTerm);
    QScriptValue appendTerm = newFunction(&__appendTerm);
    QScriptValue sendData = newFunction(&__sendData);
    context->activationObject().setProperty("clearTerm", clearTerm);
    context->activationObject().setProperty("appendTerm", appendTerm);
    context->activationObject().setProperty("sendData", sendData);

    m_global = context->activationObject();
}

void ScriptEnv::setSource(const QString &source)
{
    popContext();
    prepareNewContext();
    evaluate(source);

    if(hasUncaughtException())
        throw tr("%1 on line %2").arg(uncaughtException().toString()).arg(uncaughtExceptionLineNumber());

    m_on_data = m_global.property("onDataChanged");
    m_on_key = m_global.property("onKeyPress");
    m_source = source;
}

QString ScriptEnv::dataChanged(analyzer_data *data, quint32 index)
{
    if(!m_on_data.isFunction())
        return "";

    const QByteArray& pkt_data = data->getData();

    QScriptValue jsData = newArray(pkt_data.size());
    for(qint32 i = 0; i < pkt_data.size(); ++i)
        jsData.setProperty(i, QScriptValue(this, (quint8)pkt_data[i]));

    QScriptValueList args;
    args.push_back(jsData);

    quint8 res = 0;
    if(data->getDeviceId(res)) args << res;
    else                       args << -1;

    if(data->getCmd(res))  args << res;
    else                   args << -1;

    args << index;

    QScriptValue val = m_on_data.call(QScriptValue(), args);
    return val.isUndefined() ? "" : val.toString();
}

void ScriptEnv::keyPressed(const QByteArray &key)
{
    if(!m_on_key.isFunction() || key.isEmpty())
        return;

    QScriptValueList args;
    args << QString(key);

    m_on_key.call(QScriptValue(), args);
}

QScriptValue ScriptEnv::__clearTerm(QScriptContext *context, QScriptEngine *engine)
{
    emit ((ScriptEnv*)engine)->clearTerm();
    return QScriptValue();
}

QScriptValue ScriptEnv::__appendTerm(QScriptContext *context, QScriptEngine *engine)
{
    emit ((ScriptEnv*)engine)->appendTerm(context->argument(0).toString().toAscii());
    return QScriptValue();
}

QScriptValue ScriptEnv::__sendData(QScriptContext *context, QScriptEngine *engine)
{
    if(context->argumentCount() == 0)
        return QScriptValue();

    QScriptValue data = context->argument(0);
    if(!data.isArray())
        return QScriptValue();

    QByteArray sendData;

    QScriptValueIterator itr(data);
    while(itr.hasNext())
    {
        itr.next();
        if(itr.value().isNumber())
            sendData.push_back(itr.value().toUInt16());
    }

    emit ((ScriptEnv*)engine)->SendData(sendData);
    return QScriptValue();
}
