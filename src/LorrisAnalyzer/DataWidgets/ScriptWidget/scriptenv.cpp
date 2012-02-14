#include <QStringList>
#include <QScriptValueIterator>
#include <QScriptValueList>
#include <QScriptEngineAgent>
#include <QDebug>
#include "scriptenv.h"

ScriptEnv::ScriptEnv(QObject *parent) :
    QScriptEngine(parent)
{
    m_global = globalObject();
    m_source =
            "// You can use clearTerm() and appendTerm(string) to set term content\n\n"
            "// This function gets called on data received\n"
            "// it should return string, which is automatically appended to terminal\n"
            "function onDataChanged(data, dev, cmd) {\n"
            "    return \"\";\n"
            "}\n\n"
            "// This function is called on key press in terminal.\n"
            "// Param is string\n"
            "function onKeyPress(key) {\n"
            "    \n"
            "}\n";

    QScriptValue clearTerm = newFunction(&__clearTerm);
    QScriptValue appendTerm = newFunction(&__appendTerm);
    m_global.setProperty("clearTerm", clearTerm);
    m_global.setProperty("appendTerm", appendTerm);
}

void ScriptEnv::setSource(const QString &source)
{
    evaluate(source);

    if(hasUncaughtException())
        throw tr("%1 on line %2").arg(uncaughtException().toString()).arg(uncaughtExceptionLineNumber());

    m_on_data = m_global.property("onDataChanged");
    m_on_key = m_global.property("onKeyPress");
    m_source = source;
}

QString ScriptEnv::dataChanged(analyzer_data *data)
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
