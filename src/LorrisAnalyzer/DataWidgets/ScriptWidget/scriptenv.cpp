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
#include <QTimer>
#include <QComboBox>

#include <analyzerdataarea.h>
#include "DataWidgets/GraphWidget/graphcurve.h"
#include "DataWidgets/datawidget.h"
#include "DataWidgets/inputwidget.h"
#include "scriptenv.h"
#include "scriptagent.h"
#include "joystick/joymgr.h"

/* Simple JavaScript Inheritance
 * By John Resig http://ejohn.org/
 * MIT Licensed.
 * http://ejohn.org/blog/simple-javascript-inheritance/
 */
static const QString classImplement =
        "(function(){var i=false,fnTest=/xyz/.test(function(){xyz})?/\\b_super\\b/:/.*/;this.Class=function(){};"
        "Class.extend=function(e){var f=this.prototype;i=true;var g=new this();i=false;for(var h in e){g[h]="
        "typeof e[h]==\"function\"&&typeof f[h]==\"function\"&&fnTest.test(e[h])?(function(c,d){return function()"
        "{var a=this._super;this._super=f[c];var b=d.apply(this,arguments);this._super=a;return b}})(h,e[h]):"
        "e[h]}function Class(){if(!i&&this.init)this.init.apply(this,arguments)}Class.prototype=g;Class.proto"
        "type.constructor=Class;Class.extend=arguments.callee;return Class}})();";

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

ScriptEnv::~ScriptEnv()
{
    while(!m_widgets.empty())
        m_area->removeWidget((*m_widgets.begin())->getId());

    for(std::list<QTimer*>::iterator itr = m_timers.begin(); itr != m_timers.end(); ++itr)
        delete *itr;

    emit stopUsingJoy(this);
}

void ScriptEnv::widgetDestroyed(QObject *widget)
{
    DataWidget *w = (DataWidget*)widget;
    m_widgets.remove(w->getId());
}

void ScriptEnv::prepareNewContext()
{
    QScriptContext *context = pushContext();

    m_global = context->activationObject();

    // Functions
    QScriptValue clearTerm = newFunction(&__clearTerm);
    QScriptValue appendTerm = newFunction(&__appendTerm);
    QScriptValue sendData = newFunction(&__sendData);
    QScriptValue getW = newFunction(&__getWidth);
    QScriptValue getH = newFunction(&__getHeight);
    QScriptValue throwEx = newFunction(&__throwException);
    QScriptValue getJoy = newFunction(&__getJoystick);
    QScriptValue getJoyName = newFunction(&__getJoystickNames);
    QScriptValue newTimer = newFunction(&__newTimer);
    QScriptValue addComboItems = newFunction(&__addComboBoxItems);
    QScriptValue moveW = newFunction(&__moveWidget);

    QScriptValue numberW = newFunction(&__newNumberWidget);
    QScriptValue barW = newFunction(&__newBarWidget);
    QScriptValue colorW = newFunction(&__newColorWidget);
    QScriptValue graphW = newFunction(&__newGraphWidget);
    QScriptValue inputW = newFunction(&__newInputWidget);
    QScriptValue newW = newFunction(&__newWidget);

    m_global.setProperty("clearTerm", clearTerm);
    m_global.setProperty("appendTerm", appendTerm);
    m_global.setProperty("sendData", sendData);
    m_global.setProperty("getWidth", getW);
    m_global.setProperty("getHeight", getH);
    m_global.setProperty("throwException", throwEx);
    m_global.setProperty("getJoystick", getJoy);
    m_global.setProperty("getJoystickNames", getJoyName);
    m_global.setProperty("newTimer", newTimer);
    m_global.setProperty("addComboBoxItems", addComboItems);
    m_global.setProperty("moveWidget", moveW);

    m_global.setProperty("newNumberWidget", numberW);
    m_global.setProperty("newBarWidget", barW);
    m_global.setProperty("newColorWidget", colorW);
    m_global.setProperty("newGraphWidget", graphW);
    m_global.setProperty("newInputWidget", inputW);
    m_global.setProperty("newWidget", newW);

    // defines
    m_global.setProperty("WIDGET_NUMBER", QScriptValue(this, WIDGET_NUMBERS));
    m_global.setProperty("WIDGET_BAR",    QScriptValue(this, WIDGET_BAR));
    m_global.setProperty("WIDGET_COLOR",  QScriptValue(this, WIDGET_COLOR));
    m_global.setProperty("WIDGET_GRAPH",  QScriptValue(this, WIDGET_GRAPH));
    m_global.setProperty("WIDGET_INPUT",  QScriptValue(this, WIDGET_INPUT));

    m_global.setProperty("NUM_UINT8",  QScriptValue(this, NUM_UINT8));
    m_global.setProperty("NUM_UINT16", QScriptValue(this, NUM_UINT16));
    m_global.setProperty("NUM_UINT32", QScriptValue(this, NUM_UINT32));
    m_global.setProperty("NUM_UINT64", QScriptValue(this, NUM_UINT64));
    m_global.setProperty("NUM_INT8",   QScriptValue(this, NUM_INT8));
    m_global.setProperty("NUM_INT16",  QScriptValue(this, NUM_INT16));
    m_global.setProperty("NUM_INT32",  QScriptValue(this, NUM_INT32));
    m_global.setProperty("NUM_INT64",  QScriptValue(this, NUM_INT64));
    m_global.setProperty("NUM_FLOAT",  QScriptValue(this, NUM_FLOAT));
    m_global.setProperty("NUM_DOUBLE", QScriptValue(this, NUM_DOUBLE));

    // objects
    m_global.setProperty("script", newQObject(parent()));
    m_global.setProperty("area", newQObject(m_area));

    const AnalyzerDataArea::w_map& widgets = m_area->getWidgets();
    for(AnalyzerDataArea::w_map::const_iterator itr = widgets.begin(); itr != widgets.end(); ++itr)
    {
        QString name = sanitizeWidgetName((*itr)->getTitle());
        if(!name.isEmpty())
            m_global.setProperty(name, newQObject(*itr));
    }

    // remove script created widgets and timer from previous script
    while(!m_widgets.empty())
        m_area->removeWidget((*m_widgets.begin())->getId());

    for(std::list<QTimer*>::iterator itr = m_timers.begin(); itr != m_timers.end(); ++itr)
        delete *itr;
    m_timers.clear();

    emit stopUsingJoy(this);

    qScriptRegisterMetaType(this, GraphCurveToScriptValue, GraphCurveFromScriptValue);
}

void ScriptEnv::setSource(const QString &source)
{
    m_source = source;

    popContext();
    prepareNewContext();
    setAgent(NULL);
    evaluate(classImplement + source);

    if(hasUncaughtException())
        throw tr("%1 on line %2").arg(uncaughtException().toString()).arg(uncaughtExceptionLineNumber());

    m_on_data = m_global.property("onDataChanged");
    m_on_key = m_global.property("onKeyPress");
    m_on_widget_add = m_global.property("onWidgetAdd");
    m_on_widget_remove = m_global.property("onWidgetRemove");

    setAgent(new ScriptAgent(this));
}

QString ScriptEnv::dataChanged(analyzer_data *data, quint32 index)
{
    // do not execute when setting source - agent() == NULL
    if(!m_on_data.isFunction() || !agent())
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

DataWidget *ScriptEnv::addWidget(quint8 type, QScriptContext *context, quint8 removeArg)
{
    if(context->argumentCount()-removeArg == 0)
        return NULL;

    DataWidget *w = m_area->addWidget(QPoint(0,0), type);
    if(!w)
        return NULL;

    connect(w, SIGNAL(destroyed(QObject*)), SLOT(widgetDestroyed(QObject*)));
    m_widgets.insert(w->getId(), w);

    switch(context->argumentCount()-removeArg)
    {
        case 5:
        {
            int x = m_x + context->argument(3+removeArg).toInt32();
            int y = m_y + context->argument(4+removeArg).toInt32();
            w->move(x, y);
            // Fallthrough
        }
        case 3:
        {
            int wid = context->argument(1+removeArg).toUInt32();
            int h = context->argument(2+removeArg).toUInt32();
            if(wid || h)
                w->resize(wid, h);
            // Fallthrough
        }
        case 1:
            w->setTitle(context->argument(0+removeArg).toString());
            break;
    }

    w->setWidgetControlled(m_widget_id);
    w->setAcceptDrops(false);

    return w;
}

QScriptValue ScriptEnv::newTimer()
{
    QTimer *t = new QTimer();
    m_timers.push_back(t);
    return newQObject(t);
}

QString ScriptEnv::sanitizeWidgetName(QString const & name)
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

void ScriptEnv::onWidgetAdd(DataWidget *w)
{
    QString name = sanitizeWidgetName(w->getTitle());
    if(!name.isEmpty())
        m_global.setProperty(name, newQObject(w));

    connect(w, SIGNAL(titleChanged(QString)), SLOT(onTitleChange(QString)));

    if(!m_on_widget_add.isFunction())
        return;
    QScriptValueList args;
    args << newQObject(w) << name;
    m_on_widget_add.call(QScriptValue(), args);
}

void ScriptEnv::onWidgetRemove(DataWidget *w)
{
    QString name = sanitizeWidgetName(w->getTitle());
    if(!name.isEmpty())
        m_global.setProperty(name, undefinedValue());
    disconnect(w, SIGNAL(titleChanged(QString)), this, SLOT(onTitleChange(QString)));

    if(!m_on_widget_remove.isFunction())
        return;

    QScriptValueList args;
    args << newQObject(w) << name;
    m_on_widget_remove.call(QScriptValue(), args);
}

void ScriptEnv::onTitleChange(const QString& newTitle)
{
    DataWidget *w = (DataWidget*)sender();
    QString name = sanitizeWidgetName(w->getTitle());

    if(!name.isEmpty())
        m_global.setProperty(name, undefinedValue());

    name = sanitizeWidgetName(newTitle);
    if(!name.isEmpty())
        m_global.setProperty(name, newQObject(w));
}

void ScriptEnv::callEventHandler(const QString& eventId)
{
    QScriptValue handler = m_global.property(eventId);

    if(!handler.isFunction())
        return;

    handler.call();
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

    if(sendData.size() > 1)
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

QScriptValue ScriptEnv::__newWidget(QScriptContext *context, QScriptEngine *engine)
{
    if(context->argumentCount() < 2)
        return QScriptValue();

    quint16 type = context->argument(0).toUInt16();

    if(type >= WIDGET_MAX || type == WIDGET_SCRIPT)
        return QScriptValue();


    DataWidget *w = ((ScriptEnv*)engine)->addWidget(type, context, 1);
    return engine->newQObject(w);
}

QScriptValue ScriptEnv::__getWidth(QScriptContext */*context*/, QScriptEngine *engine)
{
    return ((ScriptEnv*)engine)->getWidth();
}

QScriptValue ScriptEnv::__getHeight(QScriptContext */*context*/, QScriptEngine *engine)
{
    return ((ScriptEnv*)engine)->getHeight();
}

QScriptValue ScriptEnv::__throwException(QScriptContext *context, QScriptEngine */*engine*/)
{
    if(context->argumentCount() != 1)
        return QScriptValue();

    Utils::ThrowException(context->argument(0).toString());

    return QScriptValue();
}

QScriptValue ScriptEnv::__getJoystick(QScriptContext *context, QScriptEngine *engine)
{
    if(context->argumentCount() != 1)
        return QScriptValue();

    Joystick *joy = sJoyMgr.getJoystick(context->argument(0).toInt32());
    if(!joy)
        return QScriptValue();

    joy->startUsing(engine);

    connect((ScriptEnv*)engine, SIGNAL(stopUsingJoy(QObject*)), joy, SLOT(stopUsing(QObject*)));
    return engine->newQObject(joy);
}

QScriptValue ScriptEnv::__newTimer(QScriptContext */*context*/, QScriptEngine *engine)
{
    return ((ScriptEnv*)engine)->newTimer();
}

QScriptValue ScriptEnv::__getJoystickNames(QScriptContext */*context*/, QScriptEngine *engine)
{
    sJoyMgr.updateJoystickNames();

    const QHash<int, QString>& names = sJoyMgr.getNames();
    QScriptValue val = engine->newArray(names.size());

    for(int i = 0; i < names.size(); ++i)
        val.setProperty(i, names[i]);

    return val;
}

QScriptValue ScriptEnv::__addComboBoxItems(QScriptContext *context, QScriptEngine */*engine*/)
{
    if(context->argumentCount() != 2)
        return QScriptValue();

    if(!context->argument(0).isQObject() || !context->argument(1).isArray())
        return QScriptValue();

    if(!context->argument(0).toQObject()->inherits("QComboBox"))
        return QScriptValue();

    QComboBox *box = (QComboBox*)context->argument(0).toQObject();

    QScriptValueIterator itr(context->argument(1));
    while(itr.hasNext())
    {
        itr.next();
        if(itr.value().isString())
            box->addItem(itr.value().toString());
    }
    return QScriptValue();
}

QScriptValue ScriptEnv::__moveWidget(QScriptContext *context, QScriptEngine */*engine*/)
{
    if(context->argumentCount() != 3)
        return QScriptValue();

    if(!context->argument(0).isQObject() || !context->argument(0).toQObject()->inherits("QWidget"))
        return QScriptValue();

    QWidget *w = (QWidget*)context->argument(0).toQObject();
    w->move(context->argument(1).toInt32(), context->argument(2).toInt32());

    return QScriptValue();
}
