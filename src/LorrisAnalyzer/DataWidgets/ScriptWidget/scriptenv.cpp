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

#include <analyzerdataarea.h>
#include "DataWidgets/GraphWidget/graphcurve.h"
#include "DataWidgets/datawidget.h"
#include "scriptenv.h"

QScriptValue GraphCurveToScriptValue(QScriptEngine *engine, GraphCurve* const &in)
{ return engine->newQObject(in); }

void GraphCurveFromScriptValue(const QScriptValue &object, GraphCurve* &out)
{ out = qobject_cast<GraphCurve*>(object.toQObject()); }

ScriptEnv::ScriptEnv(AnalyzerDataArea* area, quint32 w_id, QObject *parent) :
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

    m_area = area;
    m_widget_id = w_id;
    m_x = m_y = 0;
}

void ScriptEnv::prepareNewContext()
{
    QScriptContext *context = pushContext();

    m_global = context->activationObject();

    // Functions
    QScriptValue clearTerm = newFunction(&__clearTerm);
    QScriptValue appendTerm = newFunction(&__appendTerm);
    QScriptValue sendData = newFunction(&__sendData);

    QScriptValue numberW = newFunction(&__newNumberWidget);
    QScriptValue barW = newFunction(&__newBarWidget);
    QScriptValue colorW = newFunction(&__newColorWidget);
    QScriptValue graphW = newFunction(&__newGraphWidget);
    QScriptValue inputW = newFunction(&__newInputWidget);

    m_global.setProperty("clearTerm", clearTerm);
    m_global.setProperty("appendTerm", appendTerm);
    m_global.setProperty("sendData", sendData);

    m_global.setProperty("newNumberWidget", numberW);
    m_global.setProperty("newBarWidget", barW);
    m_global.setProperty("newColorWidget", colorW);
    m_global.setProperty("newGraphWidget", graphW);
    m_global.setProperty("newInputWidget", inputW);

    for(std::list<DataWidget*>::iterator itr = m_widgets.begin(); itr != m_widgets.end(); ++itr)
        m_area->removeWidget((*itr)->getId());
    m_widgets.clear();

    qScriptRegisterMetaType(this, GraphCurveToScriptValue, GraphCurveFromScriptValue);
}

void ScriptEnv::setSource(const QString &source)
{
    m_source = source;

    popContext();
    prepareNewContext();
    evaluate(source);

    if(hasUncaughtException())
        throw tr("%1 on line %2").arg(uncaughtException().toString()).arg(uncaughtExceptionLineNumber());

    m_on_data = m_global.property("onDataChanged");
    m_on_key = m_global.property("onKeyPress");
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

DataWidget *ScriptEnv::addWidget(quint8 type, QScriptContext *context)
{
    if(context->argumentCount() == 0)
        return NULL;

    DataWidget *w = m_area->addWidget(QPoint(0,0), type);
    if(!w)
        return NULL;

    m_widgets.push_back(w);

    switch(context->argumentCount())
    {
        case 5:
        {
            int x = m_x + context->argument(3).toInt32();
            int y = m_y + context->argument(4).toInt32();
            w->move(x, y);
            // Fallthrough
        }
        case 3:
        {
            int wid = context->argument(1).toUInt32();
            int h = context->argument(2).toUInt32();
            if(wid || h)
                w->resize(wid, h);
            // Fallthrough
        }
        case 1:
            w->setTitle(context->argument(0).toString());
            break;
    }

    w->setWidgetControlled(m_widget_id);
    w->setAcceptDrops(false);

    return w;
}

QScriptValue ScriptEnv::__clearTerm(QScriptContext */*context*/, QScriptEngine *engine)
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

    sendData.chop(1); // last num is array len, wtf

    emit ((ScriptEnv*)engine)->SendData(sendData);
    return QScriptValue();
}

QScriptValue ScriptEnv::__newNumberWidget(QScriptContext *context, QScriptEngine *engine)
{
    DataWidget *w = ((ScriptEnv*)engine)->addWidget(WIDGET_NUMBERS, context);
    return engine->newQObject(w);
}

QScriptValue ScriptEnv::__newBarWidget(QScriptContext *context, QScriptEngine *engine)
{
    DataWidget *w = ((ScriptEnv*)engine)->addWidget(WIDGET_BAR, context);
    return engine->newQObject(w);
}

QScriptValue ScriptEnv::__newColorWidget(QScriptContext *context, QScriptEngine *engine)
{
    DataWidget *w = ((ScriptEnv*)engine)->addWidget(WIDGET_COLOR, context);
    return engine->newQObject(w);
}

QScriptValue ScriptEnv::__newGraphWidget(QScriptContext *context, QScriptEngine *engine)
{
    DataWidget *w = ((ScriptEnv*)engine)->addWidget(WIDGET_GRAPH, context);
    return engine->newQObject(w);
}


QScriptValue ScriptEnv::__newInputWidget(QScriptContext *context, QScriptEngine *engine)
{
    DataWidget *w = ((ScriptEnv*)engine)->addWidget(WIDGET_INPUT, context);
    return engine->newQObject(w);
}
