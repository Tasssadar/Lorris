/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QStringList>
#include <QScriptValueIterator>
#include <QScriptValueList>
#include <QScriptEngineAgent>
#include <QDebug>
#include <QTimer>
#include <QComboBox>

#include "../../../widgetarea.h"
#include "../../GraphWidget/graphcurve.h"
#include "../../datawidget.h"
#include "../../inputwidget.h"
#include "qtscriptengine.h"
#include "scriptagent.h"
#include "../../../../joystick/joymgr.h"
#include "../scriptwidget.h"
#include "../../../../ui/terminal.h"
#include "../../../storage.h"

/* Simple JavaScript Inheritance
 * By John Resig http://ejohn.org/
 * MIT Licensed, see MIT_LICENSE file
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

QtScriptEngine::QtScriptEngine(WidgetArea *area, quint32 w_id, ScriptWidget *parent) :
    ScriptEngine(area, w_id, parent)
{
    m_engine = NULL;
    setSource(QString());
}

QtScriptEngine::~QtScriptEngine()
{
    if(m_engine)
    {
        if(m_on_script_exit.isFunction())
            m_on_script_exit.call();

        while(!m_widgets.empty())
            m_area->removeWidget((*m_widgets.begin())->getId());

        emit stopUsingJoy(m_engine);

        delete m_engine;
    }
}

void QtScriptEngine::prepareNewContext()
{
    m_global = m_engine->currentContext()->activationObject();

    // Functions
    QScriptValue clearTerm = m_engine->newFunction(&QtScriptEngine_private::__clearTerm);
    QScriptValue appendTerm = m_engine->newFunction(&QtScriptEngine_private::__appendTerm);
    QScriptValue sendData = m_engine->newFunction(&QtScriptEngine_private::__sendData);
    QScriptValue getW = m_engine->newFunction(&QtScriptEngine_private::__getWidth);
    QScriptValue getH = m_engine->newFunction(&QtScriptEngine_private::__getHeight);
    QScriptValue throwEx = m_engine->newFunction(&QtScriptEngine_private::__throwException);
    QScriptValue getJoy = m_engine->newFunction(&QtScriptEngine_private::__getJoystick);
    QScriptValue getFirstJoy = m_engine->newFunction(&QtScriptEngine_private::__getFirstJoystick);
    QScriptValue closeJoy = m_engine->newFunction(&QtScriptEngine_private::__closeJoystick);
    QScriptValue getJoyName = m_engine->newFunction(&QtScriptEngine_private::__getJoystickNames);
    QScriptValue getJoyIds = m_engine->newFunction(&QtScriptEngine_private::__getJoystickIds);
    QScriptValue newTimer = m_engine->newFunction(&QtScriptEngine_private::__newTimer);
    QScriptValue addComboItems = m_engine->newFunction(&QtScriptEngine_private::__addComboBoxItems);
    QScriptValue moveW = m_engine->newFunction(&QtScriptEngine_private::__moveWidget);
    QScriptValue resizeW = m_engine->newFunction(&QtScriptEngine_private::__resizeWidget);
    QScriptValue getData = m_engine->newFunction(&QtScriptEngine_private::__getData);
    QScriptValue getDataCount = m_engine->newFunction(&QtScriptEngine_private::__getDataCount);
    QScriptValue playErrorSound = m_engine->newFunction(&QtScriptEngine_private::__playErrorSound);
    QScriptValue setMaxPacketNumber = m_engine->newFunction(&QtScriptEngine_private::__setMaxPacketNumber);
    QScriptValue setInterval = m_engine->newFunction(&QtScriptEngine_private::__setInterval);
    QScriptValue setTimeout = m_engine->newFunction(&QtScriptEngine_private::__setTimeout);
    QScriptValue clearTimer = m_engine->newFunction(&QtScriptEngine_private::__clearTimer);

    QScriptValue numberW = m_engine->newFunction(&QtScriptEngine_private::__newNumberWidget);
    QScriptValue barW = m_engine->newFunction(&QtScriptEngine_private::__newBarWidget);
    QScriptValue colorW = m_engine->newFunction(&QtScriptEngine_private::__newColorWidget);
    QScriptValue graphW = m_engine->newFunction(&QtScriptEngine_private::__newGraphWidget);
    QScriptValue inputW = m_engine->newFunction(&QtScriptEngine_private::__newInputWidget);
    QScriptValue circleW = m_engine->newFunction(&QtScriptEngine_private::__newCircleWidget);
    QScriptValue sliderW = m_engine->newFunction(&QtScriptEngine_private::__newSliderWidget);
    QScriptValue canvasW = m_engine->newFunction(&QtScriptEngine_private::__newCanvasWidget);
    QScriptValue newW = m_engine->newFunction(&QtScriptEngine_private::__newWidget);

    m_global.setProperty("clearTerm", clearTerm);
    m_global.setProperty("appendTerm", appendTerm);
    m_global.setProperty("print", appendTerm);
    m_global.setProperty("sendData", sendData);
    m_global.setProperty("send", sendData);
    m_global.setProperty("getWidth", getW);
    m_global.setProperty("getHeight", getH);
    m_global.setProperty("throwException", throwEx);
    m_global.setProperty("alert", throwEx);
    m_global.setProperty("getJoystick", getJoy);
    m_global.setProperty("getFirstJoystick", getFirstJoy);
    m_global.setProperty("closeJoystick", closeJoy);
    m_global.setProperty("getJoystickNames", getJoyName);
    m_global.setProperty("getJoystickIds", getJoyIds);
    m_global.setProperty("newTimer", newTimer);
    m_global.setProperty("addComboBoxItems", addComboItems);
    m_global.setProperty("moveWidget", moveW);
    m_global.setProperty("resizeWidget", resizeW);
    m_global.setProperty("getData", getData);
    m_global.setProperty("getDataCount", getDataCount);
    m_global.setProperty("playErrorSound", playErrorSound);
    m_global.setProperty("setMaxPacketNumber", setMaxPacketNumber);
    m_global.setProperty("setInterval", setInterval);
    m_global.setProperty("setTimeout", setTimeout);
    m_global.setProperty("clearInterval", clearTimer);
    m_global.setProperty("clearTimeout", clearTimer);

    m_global.setProperty("newNumberWidget", numberW);
    m_global.setProperty("newBarWidget", barW);
    m_global.setProperty("newColorWidget", colorW);
    m_global.setProperty("newGraphWidget", graphW);
    m_global.setProperty("newInputWidget", inputW);
    m_global.setProperty("newCircleWidget", circleW);
    m_global.setProperty("newSliderWidget", sliderW);
    m_global.setProperty("newCanvasWidget", canvasW);
    m_global.setProperty("newWidget", newW);

    // defines
    const QHash<QString, quint32>& enums = sWidgetFactory.getScriptEnums();
    for(QHash<QString, quint32>::const_iterator itr = enums.begin(); itr != enums.end(); ++itr)
         m_global.setProperty(itr.key(),  QScriptValue(m_engine, *itr));

    // objects
    m_global.setProperty("script", m_engine->newQObject(parent()));
    m_global.setProperty("area", m_engine->newQObject(m_area));
    m_global.setProperty("storage", m_engine->newQObject(m_storage));
    m_global.setProperty("terminal", m_engine->newQObject(scriptWidget()->getTerminal()));
    m_global.setProperty("inputLine", m_engine->newQObject(scriptWidget()->getInputEdit()));

    const WidgetArea::w_map& widgets = m_area->getWidgets();
    for(WidgetArea::w_map::const_iterator itr = widgets.begin(); itr != widgets.end(); ++itr)
    {
        QString name = sanitizeWidgetName((*itr)->getTitle());
        if(!name.isEmpty())
            m_global.setProperty(name, m_engine->newQObject(*itr));
    }

    // remove script created widgets and timer from previous script
    while(!m_widgets.empty())
        m_area->removeWidget((*m_widgets.begin())->getId());

    for(std::list<QTimer*>::iterator itr = m_timers.begin(); itr != m_timers.end(); ++itr)
        delete *itr;
    m_timers.clear();

    emit stopUsingJoy(m_engine);

    qScriptRegisterMetaType(m_engine, GraphCurveToScriptValue, GraphCurveFromScriptValue);
}

void QtScriptEngine::widgetDestroyed(QObject *widget)
{
    DataWidget *w = (DataWidget*)widget;
    m_widgets.remove(w->getId());
}

void QtScriptEngine::setSource(const QString &source)
{
    m_source = source;

    if(m_on_script_exit.isFunction())
        m_on_script_exit.call();

    delete m_engine;
    m_engine = new QtScriptEngine_private(this, parent());
    m_engine->setAgent(new ScriptAgent(this, m_engine));

    connect(this, SIGNAL(stopUsingJoy(QObject*)), m_engine, SIGNAL(stopUsingJoy(QObject*)));

    prepareNewContext();
    m_engine->evaluate(classImplement + source);

    if(m_engine->hasUncaughtException())
        emit error(tr("%1 on line %2").arg(m_engine->uncaughtException().toString()).arg(m_engine->uncaughtExceptionLineNumber()));

    m_on_data = m_global.property("onDataChanged");
    m_on_key = m_global.property("onKeyPress");
    m_on_widget_add = m_global.property("onWidgetAdd");
    m_on_widget_remove = m_global.property("onWidgetRemove");
    m_on_script_exit = m_global.property("onScriptExit");
    m_on_save = m_global.property("onSave");
    m_on_raw = m_global.property("onRawData");

    if(m_on_widget_add.isFunction())
    {
        QScriptValueList args;
        const WidgetArea::w_map& widgets = m_area->getWidgets();
        for(WidgetArea::w_map::const_iterator itr = widgets.begin(); itr != widgets.end(); ++itr)
        {
            const QString name = sanitizeWidgetName((*itr)->getTitle());
            if(name.isEmpty())
                continue;

            args.clear();
            args << m_engine->newQObject(*itr) << name;
            m_on_widget_add.call(QScriptValue(), args);
        }
    }
}

QString QtScriptEngine::dataChanged(analyzer_data *data, quint32 index)
{
    // do not execute when setting source - agent() == NULL
    if(!m_on_data.isFunction() || !m_engine->agent())
        return "";

    const QByteArray& pkt_data = data->getData();

    QScriptValue jsData = m_engine->newArray(pkt_data.size());
    for(qint32 i = 0; i < pkt_data.size(); ++i)
        jsData.setProperty(i, QScriptValue(m_engine, (quint8)pkt_data[i]));

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

void QtScriptEngine::keyPressed(const QString &key)
{
    if(!m_on_key.isFunction() || key.isEmpty())
        return;

    QScriptValueList args;
    args << key;

    m_on_key.call(QScriptValue(), args);
}

DataWidget *QtScriptEngine::addWidget(quint8 type, QScriptContext *context, quint8 removeArg)
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

QTimer *QtScriptEngine::newTimer()
{
    QTimer *t = new QTimer(this);
    m_timers.push_back(t);
    return t;
}

void QtScriptEngine::onWidgetAdd(DataWidget *w)
{
    QString name = sanitizeWidgetName(w->getTitle());
    if(!name.isEmpty())
        m_global.setProperty(name, m_engine->newQObject(w));

    connect(w, SIGNAL(titleChanged(QString)), SLOT(onTitleChange(QString)));

    if(!m_on_widget_add.isFunction())
        return;
    QScriptValueList args;
    args << m_engine->newQObject(w) << name;
    m_on_widget_add.call(QScriptValue(), args);
}

void QtScriptEngine::onWidgetRemove(DataWidget *w)
{
    QString name = sanitizeWidgetName(w->getTitle());
    if(!name.isEmpty())
        m_global.setProperty(name, m_engine->undefinedValue());
    disconnect(w, SIGNAL(titleChanged(QString)), this, SLOT(onTitleChange(QString)));

    if(!m_on_widget_remove.isFunction())
        return;

    QScriptValueList args;
    args << m_engine->newQObject(w) << name;
    m_on_widget_remove.call(QScriptValue(), args);
}

void QtScriptEngine::onTitleChange(const QString& newTitle)
{
    DataWidget *w = (DataWidget*)sender();
    QString name = sanitizeWidgetName(w->getTitle());

    if(!name.isEmpty())
        m_global.setProperty(name, m_engine->undefinedValue());

    name = sanitizeWidgetName(newTitle);
    if(!name.isEmpty())
        m_global.setProperty(name, m_engine->newQObject(w));
}

void QtScriptEngine::callEventHandler(const QString& eventId, const QVariantList& args)
{
    QScriptValue handler = m_global.property(eventId);

    if(!handler.isFunction())
        return;

    QScriptValueList script_args;
    for(int i = 0; i < args.size(); ++i)
    {
        const QVariant& val = args[i];
        QScriptValue add;
        switch(val.type())
        {
            case QVariant::Char:
                add = QScriptValue(val.toChar());
                break;
            case QVariant::Int:
            case QVariant::LongLong:
                add = QScriptValue(val.toInt());
                break;
            case QVariant::ULongLong:
            case QVariant::UInt:
                add = QScriptValue(val.toUInt());
                break;
            case QVariant::Bool:
                add = QScriptValue(val.toBool());
                break;
            case QVariant::Double:
                add = QScriptValue(val.toDouble());
                break;
            case QVariant::String:
                add = QScriptValue(val.toString());
                break;
            default:
                qWarning("Unknown QVariant type %u!", (quint32)val.type());
                Q_ASSERT(false);
                continue;
        }
        script_args << add;
    }

    handler.call(QScriptValue(), script_args);
}

void QtScriptEngine::onSave()
{
    if(m_on_save.isFunction())
        m_on_save.call();
}

void QtScriptEngine::rawData(const QByteArray &data)
{
    // do not execute when setting source - agent() == NULL
    if(!m_on_raw.isFunction() || !m_engine->agent())
        return;

    QScriptValue jsData = m_engine->newArray(data.size());
    for(qint32 i = 0; i < data.size(); ++i)
        jsData.setProperty(i, QScriptValue(m_engine, (quint8)data[i]));

    QScriptValueList args;
    args.push_back(jsData);

    m_on_raw.call(QScriptValue(), args);
}

QtScriptEngine_private::QtScriptEngine_private(QtScriptEngine *base, QObject *parent) :
    QScriptEngine(parent)
{
    m_base = base;
    setProcessEventsInterval(50);
    m_freezeDetectTimer.setSingleShot(true);
    connect(&m_freezeDetectTimer, SIGNAL(timeout()), this, SLOT(freezeDetectorTimeout()));
}

QScriptValue QtScriptEngine_private::evaluate(const QString &program, const QString &fileName, int lineNumber)
{
    startFreezeDetector();
    QScriptValue res = QScriptEngine::evaluate(program, fileName, lineNumber);
    stopFreezeDetector();
    return res;
}

void QtScriptEngine_private::startFreezeDetector()
{
    if(!m_freezeDetectTimer.isActive())
        m_freezeDetectTimer.start(sConfig.get(CFG_QUINT32_SCRIPT_FREEZE_TIMEOUT));
}

void QtScriptEngine_private::stopFreezeDetector()
{
    m_freezeDetectTimer.stop();
}

void QtScriptEngine_private::freezeDetectorTimeout()
{
    if(this->isEvaluating())
    {
        this->abortEvaluation();
        ((ScriptAgent*)this->agent())->restartEnterFuncCnt();
        emit m_base->error("FreezeDetector: killing, has been in qtscript code for too long. You can change the timeout in the global settings.");
    }
    else
    {
        QScriptContext *ctx = this->currentContext();
        if(ctx)
            ctx->throwError("FreezeDetector: killing, has been in qtscript code for too long. You can change the timeout in the global settings.");
    }
}

void QtScriptEngine_private::clearTerm()
{
    emit m_base->clearTerm();
}

void QtScriptEngine_private::appendTerm(const QString& string)
{
    emit m_base->appendTerm(string);
}

void QtScriptEngine_private::appendTermRaw(const QByteArray& data)
{
    emit m_base->appendTermRaw(data);
}

void QtScriptEngine_private::SendData(const QByteArray &data)
{
    emit m_base->SendData(data);
}

DataWidget *QtScriptEngine_private::addWidget(quint8 type, QScriptContext *context, quint8 removeArg)
{
    return m_base->addWidget(type, context, removeArg);
}

int QtScriptEngine_private::getWidth()
{
    return m_base->getWidth();
}

int QtScriptEngine_private::getHeight()
{
    return m_base->getHeight();
}

QScriptValue QtScriptEngine_private::newTimer()
{
    return newQObject(m_base->newTimer());
}

QByteArray *QtScriptEngine_private::getData(quint32 idx) const
{
    return m_base->getStorage()->get(idx);
}

quint32 QtScriptEngine_private::getDataCount() const
{
    return m_base->getStorage()->getSize();
}

QScriptValue QtScriptEngine_private::__clearTerm(QScriptContext */*context*/, QScriptEngine *engine)
{
    ((QtScriptEngine_private*)engine)->clearTerm();
    return QScriptValue();
}

QScriptValue QtScriptEngine_private::__appendTerm(QScriptContext *context, QScriptEngine *engine)
{
    QtScriptEngine_private *eng = (QtScriptEngine_private*)engine;
    QScriptValue arg = context->argument(0);

    if(!arg.isArray())
        eng->appendTerm(arg.toString());
    else
    {
        QByteArray data;

        QScriptValueIterator itr(arg);
        while(itr.hasNext())
        {
            itr.next();
            if(itr.value().isNumber() && itr.name() != "length")
                data.push_back(itr.value().toUInt16());
        }
        eng->appendTermRaw(data);
    }

    return QScriptValue();
}

QScriptValue QtScriptEngine_private::__sendData(QScriptContext *context, QScriptEngine *engine)
{
    if(context->argumentCount() == 0)
        return QScriptValue();

    QScriptValue data = context->argument(0);
    QByteArray sendData;
    if(data.isArray())
    {
        QScriptValueIterator itr(data);
        while(itr.hasNext())
        {
            itr.next();
            if(itr.value().isNumber() && itr.name() != "length")
                sendData.push_back(itr.value().toUInt16());
        }
    }
    else if(data.isString())
    {
        sendData = data.toString().toUtf8();
    }
    else
        return QScriptValue();

    if(!sendData.isEmpty())
        ((QtScriptEngine_private*)engine)->SendData(sendData);
    return QScriptValue();
}

QScriptValue QtScriptEngine_private::__newNumberWidget(QScriptContext *context, QScriptEngine *engine)
{
    DataWidget *w = ((QtScriptEngine_private*)engine)->addWidget(WIDGET_NUMBER, context);
    return engine->newQObject(w);
}

QScriptValue QtScriptEngine_private::__newBarWidget(QScriptContext *context, QScriptEngine *engine)
{
    DataWidget *w = ((QtScriptEngine_private*)engine)->addWidget(WIDGET_BAR, context);
    return engine->newQObject(w);
}

QScriptValue QtScriptEngine_private::__newColorWidget(QScriptContext *context, QScriptEngine *engine)
{
    DataWidget *w = ((QtScriptEngine_private*)engine)->addWidget(WIDGET_COLOR, context);
    return engine->newQObject(w);
}

QScriptValue QtScriptEngine_private::__newGraphWidget(QScriptContext *context, QScriptEngine *engine)
{
    DataWidget *w = ((QtScriptEngine_private*)engine)->addWidget(WIDGET_GRAPH, context);
    return engine->newQObject(w);
}

QScriptValue QtScriptEngine_private::__newInputWidget(QScriptContext *context, QScriptEngine *engine)
{
    DataWidget *w = ((QtScriptEngine_private*)engine)->addWidget(WIDGET_INPUT, context);
    return engine->newQObject(w);
}

QScriptValue QtScriptEngine_private::__newCircleWidget(QScriptContext *context, QScriptEngine *engine)
{
    DataWidget *w = ((QtScriptEngine_private*)engine)->addWidget(WIDGET_CIRCLE, context);
    return engine->newQObject(w);
}


QScriptValue QtScriptEngine_private::__newSliderWidget(QScriptContext *context, QScriptEngine *engine)
{
    DataWidget *w = ((QtScriptEngine_private*)engine)->addWidget(WIDGET_SLIDER, context);
    return engine->newQObject(w);
}

QScriptValue QtScriptEngine_private::__newCanvasWidget(QScriptContext *context, QScriptEngine *engine)
{
    DataWidget *w = ((QtScriptEngine_private*)engine)->addWidget(WIDGET_CANVAS, context);
    return engine->newQObject(w);
}

QScriptValue QtScriptEngine_private::__newWidget(QScriptContext *context, QScriptEngine *engine)
{
    if(context->argumentCount() < 2)
        return QScriptValue();

    quint16 type = context->argument(0).toUInt16();

    if(type >= WIDGET_MAX || type == WIDGET_SCRIPT)
        return QScriptValue();


    DataWidget *w = ((QtScriptEngine_private*)engine)->addWidget(type, context, 1);
    return engine->newQObject(w);
}

QScriptValue QtScriptEngine_private::__getWidth(QScriptContext */*context*/, QScriptEngine *engine)
{
    return ((QtScriptEngine_private*)engine)->getWidth();
}

QScriptValue QtScriptEngine_private::__getHeight(QScriptContext */*context*/, QScriptEngine *engine)
{
    return ((QtScriptEngine_private*)engine)->getHeight();
}

QScriptValue QtScriptEngine_private::__throwException(QScriptContext *context, QScriptEngine */*engine*/)
{
    if(context->argumentCount() != 1)
        return QScriptValue();

    Utils::showErrorBox(context->argument(0).toString());

    return QScriptValue();
}

QScriptValue QtScriptEngine_private::__getJoystick(QScriptContext *context, QScriptEngine *engine)
{
    if(context->argumentCount() != 1)
        return QScriptValue();

    Joystick *joy = sJoyMgr.getJoystick(context->argument(0).toInt32());
    if(!joy)
        return QScriptValue();

    joy->startUsing(engine);
    connect((QtScriptEngine_private*)engine, SIGNAL(stopUsingJoy(QObject*)), joy, SLOT(stopUsing(QObject*)));
    return engine->newQObject(joy);
}

QScriptValue QtScriptEngine_private::__getFirstJoystick(QScriptContext */*context*/, QScriptEngine *engine)
{
    Joystick *joy = sJoyMgr.getFirstJoystick();
    if(!joy)
        return QScriptValue();

    joy->startUsing(engine);
    connect((QtScriptEngine_private*)engine, SIGNAL(stopUsingJoy(QObject*)), joy, SLOT(stopUsing(QObject*)));
    return engine->newQObject(joy);
}

QScriptValue QtScriptEngine_private::__closeJoystick(QScriptContext *context, QScriptEngine *engine)
{
    if(context->argumentCount() != 1 || context->argument(0).isNull())
        return QScriptValue();

    Joystick* joy = (Joystick*)context->argument(0).toQObject();
    if(!joy->stopUsing(engine))
        delete joy;

    return QScriptValue();
}

QScriptValue QtScriptEngine_private::__newTimer(QScriptContext */*context*/, QScriptEngine *engine)
{
    return ((QtScriptEngine_private*)engine)->newTimer();
}

QScriptValue QtScriptEngine_private::__getJoystickNames(QScriptContext */*context*/, QScriptEngine *engine)
{
    sJoyMgr.updateJoystickNames();

    QStringList names = sJoyMgr.getNamesList();
    QScriptValue val = engine->newArray(names.size());

    for(int i = 0; i < names.size(); ++i)
        val.setProperty(i, names[i]);

    return val;
}

QScriptValue QtScriptEngine_private::__getJoystickIds(QScriptContext */*context*/, QScriptEngine *engine)
{
    QList<quint32> ids = sJoyMgr.getIdList();
    QScriptValue val = engine->newArray(ids.size());

    for(int i = 0; i < ids.size(); ++i)
        val.setProperty(i, ids[i]);
    return val;
}

QScriptValue QtScriptEngine_private::__addComboBoxItems(QScriptContext *context, QScriptEngine */*engine*/)
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

QScriptValue QtScriptEngine_private::__moveWidget(QScriptContext *context, QScriptEngine */*engine*/)
{
    if(context->argumentCount() != 3)
        return QScriptValue();

    if(!context->argument(0).isQObject() || !context->argument(0).toQObject()->inherits("QWidget"))
        return QScriptValue();

    QWidget *w = (QWidget*)context->argument(0).toQObject();
    w->move(context->argument(1).toInt32(), context->argument(2).toInt32());

    return QScriptValue();
}

QScriptValue QtScriptEngine_private::__resizeWidget(QScriptContext *context, QScriptEngine */*engine*/)
{
    if(context->argumentCount() != 3)
        return QScriptValue();

    if(!context->argument(0).isQObject() || !context->argument(0).toQObject()->inherits("QWidget"))
        return QScriptValue();

    QWidget *w = (QWidget*)context->argument(0).toQObject();
    w->resize(context->argument(1).toInt32(), context->argument(2).toInt32());

    return QScriptValue();
}

QScriptValue QtScriptEngine_private::__getData(QScriptContext *context, QScriptEngine *engine)
{
    if(context->argumentCount() != 1 || !context->argument(0).isNumber())
        return QScriptValue();

    QtScriptEngine_private *eng = (QtScriptEngine_private*)engine;

    quint32 idx = context->argument(0).toUInt32();
    quint32 count = eng->getDataCount();
    if(idx >= count)
        return QScriptValue();

    QByteArray *data = eng->getData(idx);
    QScriptValue jsData = eng->newArray(data->size());
    for(int i = 0; i < data->size(); ++i)
        jsData.setProperty(i, QScriptValue(eng, (quint8)data->at(i)));
    return jsData;
}

QScriptValue QtScriptEngine_private::__getDataCount(QScriptContext */*context*/, QScriptEngine *engine)
{
    return ((QtScriptEngine_private*)engine)->getDataCount();
}

QScriptValue QtScriptEngine_private::__playErrorSound(QScriptContext *context, QScriptEngine *engine)
{
    Utils::playErrorSound();
    return QScriptValue();
}

QScriptValue QtScriptEngine_private::__setMaxPacketNumber(QScriptContext *context, QScriptEngine *engine)
{
    QtScriptEngine_private *eng = (QtScriptEngine_private*)engine;
    if(context->argumentCount() != 1 || !context->argument(0).isNumber())
        return QScriptValue();

    int limit = context->argument(0).toInt32();
    if(limit < 0)
        limit = INT_MAX;

    eng->m_base->getStorage()->setPacketLimit(limit);
    return QScriptValue();
}

QScriptValue QtScriptEngine_private::__setInterval(QScriptContext *context, QScriptEngine *engine)
{
    QtScriptEngine_private *eng = (QtScriptEngine_private*)engine;
    if(context->argumentCount() < 2 || !context->argument(0).isFunction() || !context->argument(1).isNumber())
        return QScriptValue();

    QTimer *t = eng->m_base->newTimer();
    QtScriptTimerCallback *callback = new QtScriptTimerCallback(context->thisObject(), context->argument(0), t);
    connect(t, SIGNAL(timeout()), callback, SLOT(trigger()));
    t->setSingleShot(false);
    t->start(context->argument(1).toInt32());
    return eng->newQObject(t);
}

QScriptValue QtScriptEngine_private::__setTimeout(QScriptContext *context, QScriptEngine *engine)
{
    QtScriptEngine_private *eng = (QtScriptEngine_private*)engine;
    if(context->argumentCount() < 2 || !context->argument(0).isFunction() || !context->argument(1).isNumber())
        return QScriptValue();

    QTimer *t = eng->m_base->newTimer();
    QtScriptTimerCallback *callback = new QtScriptTimerCallback(context->thisObject(), context->argument(0), t);
    connect(t, SIGNAL(timeout()), callback, SLOT(trigger()));
    t->setSingleShot(true);
    t->start(context->argument(1).toInt32());
    return eng->newQObject(t);
}

QScriptValue QtScriptEngine_private::__clearTimer(QScriptContext *context, QScriptEngine *engine)
{
    if(context->argumentCount() < 1 || !context->argument(0).isQObject())
        return QScriptValue();

    QTimer *timer = dynamic_cast<QTimer *>(context->argument(0).toQObject());
    if(timer)
        timer->stop();
    return QScriptValue();
}

QtScriptTimerCallback::QtScriptTimerCallback(const QScriptValue &this_object, const QScriptValue &callback, QObject *parent) :
    QObject(parent), m_this_object(this_object), m_callback(callback)
{

}

QtScriptTimerCallback::~QtScriptTimerCallback() {

}

void QtScriptTimerCallback::trigger() {
    m_callback.call(m_this_object);
}
