/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QVariant>
#include <QByteArray>
#include <QComboBox>

#include "pythonengine.h"
#include "../../../../shared/terminal.h"
#include "../../../packet.h"
#include "../scriptstorage.h"
#include "../../../widgetarea.h"
#include "../../../../joystick/joymgr.h"
#include "../../GraphWidget/graphcurve.h"

QString PythonEngine::getNewModuleName()
{
    static int i = 0;
    return QString::number(i++).repeated(3);
}

PythonEngine::PythonEngine(WidgetArea *area, quint32 w_id, Terminal *terminal, QObject *parent) :
    ScriptEngine(area, w_id, terminal, parent), m_functions(this, parent)
{
    static bool initialized = false;
    if(!initialized)
    {
        qRegisterMetaType<GraphCurve>("GraphCurve");

        PythonQt::init();
        PythonQt::self()->registerClass(&QTimer::staticMetaObject);
        PythonQt::self()->registerClass(&GraphCurve::staticMetaObject);
        initialized = true;
    }
    connect(PythonQt::self(), SIGNAL(pythonStdErr(QString)), SIGNAL(error(QString)));
    m_evaluating = false;
}

PythonEngine::~PythonEngine()
{
    if(!m_module.isNull())
        m_module.call("onScriptExit");

    while(!m_widgets.empty())
        m_area->removeWidget((*m_widgets.begin())->getId());

    emit stopUsingJoy(this);
}

void PythonEngine::setSource(const QString &source)
{
    if(!m_module.isNull())
        m_module.call("onScriptExit");

    m_evaluating = true;
    m_source = source;

    QString name = getNewModuleName();
    static const QString predefSource = "from PythonQt.Qt import *\n"
                                        "from PythonQt.QtGui import *\n"
                                        "from PythonQt import *\n";

    m_module = PythonQt::self()->createModuleFromScript(name, predefSource);

    // remove script created widgets and timer from previous script
    while(!m_widgets.empty())
        m_area->removeWidget((*m_widgets.begin())->getId());

    m_module.addObject("terminal", m_terminal);
    m_module.addObject("script", parent());
    m_module.addObject("area", m_area);
    m_module.addObject("storage", m_storage);
    m_module.addObject("lorris", &m_functions);

    // FIXMEWTF: lorris.newWidget fails on every second setSource
    // with "AttributeError: *widgetTitle*" if this is not. What?
    m_module.getVariable("lorris");

    m_module.addVariable("WIDGET_NUMBER", WIDGET_NUMBERS);
    m_module.addVariable("WIDGET_BAR",    WIDGET_BAR);
    m_module.addVariable("WIDGET_COLOR",  WIDGET_COLOR);
    m_module.addVariable("WIDGET_GRAPH",  WIDGET_GRAPH);
    m_module.addVariable("WIDGET_INPUT",  WIDGET_INPUT);
    m_module.addVariable("WIDGET_CIRCLE",  WIDGET_CIRCLE);

    const WidgetArea::w_map& widgets = m_area->getWidgets();
    for(WidgetArea::w_map::const_iterator itr = widgets.begin(); itr != widgets.end(); ++itr)
    {
        QString name = sanitizeWidgetName((*itr)->getTitle());
        if(!name.isEmpty())
            m_module.addObject(name, *itr);
    }

    for(std::list<QTimer*>::iterator itr = m_timers.begin(); itr != m_timers.end(); ++itr)
        delete *itr;
    m_timers.clear();

    emit stopUsingJoy(this);

    m_module.evalScript(source);
    m_evaluating = false;
}

void PythonEngine::widgetDestroyed(QObject *widget)
{
    DataWidget *w = (DataWidget*)widget;
    m_widgets.remove(w->getId());
}

QString PythonEngine::dataChanged(analyzer_data *data, quint32 index)
{
    if(m_evaluating)
        return QString();

    QVariantList args;
    args << data->getData();

    quint8 res = 0;
    if(data->getDeviceId(res)) args << res;
    else                       args << -1;

    if(data->getCmd(res))  args << res;
    else                   args << -1;

    args << index;

    QVariant var = m_module.call("onDataChanged", args);
    return var.toString();
}

void PythonEngine::onWidgetAdd(DataWidget *w)
{
    QString name = sanitizeWidgetName(w->getTitle());
    if(name.isEmpty())
         return;

    m_module.addObject(name, w);
    connect(w, SIGNAL(titleChanged(QString)), SLOT(onTitleChange(QString)));

    QVariantList args;
    args << m_module.getVariable(name) << name;
    m_module.call("onWidgetAdd", args);
}

void PythonEngine::onWidgetRemove(DataWidget *w)
{
    disconnect(w, SIGNAL(titleChanged(QString)), this, SLOT(onTitleChange(QString)));

    QString name = sanitizeWidgetName(w->getTitle());
    if(name.isEmpty())
         return;

    QVariantList args;
    args << m_module.getVariable(name) << name;
    m_module.call("onWidgetRemove", args);

    m_module.removeVariable(name);
}

void PythonEngine::onTitleChange(const QString& newTitle)
{
    DataWidget *w = (DataWidget*)sender();
    QString name = sanitizeWidgetName(w->getTitle());

    if(!name.isEmpty())
        m_module.removeVariable(name);

    name = sanitizeWidgetName(newTitle);
    if(!name.isEmpty())
        m_module.addObject(name, w);
}

void PythonEngine::callEventHandler(const QString& eventId)
{
    m_module.call(eventId);
}

void PythonEngine::onSave()
{
    m_module.call("onSave");
}

void PythonEngine::keyPressed(const QString &key)
{
    m_module.call("onKeyPress", (QVariantList() << key));
}


PythonFunctions::PythonFunctions(PythonEngine *engine, QObject *parent) :
    QObject(parent)
{
    m_engine = engine;
}

void PythonFunctions::sendData(const QByteArray &data)
{
    emit m_engine->SendData(data);
}

int PythonFunctions::getWidth()
{
    return m_engine->getWidth();
}

int PythonFunctions::getHeight()
{
    return m_engine->getHeight();
}

void PythonFunctions::throwException(const QString &text)
{
    Utils::ThrowException(text);
}

Joystick *PythonFunctions::getJoystick(int id)
{
    Joystick *joy = sJoyMgr.getJoystick(id);
    if(!joy)
        return NULL;

    joy->startUsing(m_engine);
    connect(m_engine, SIGNAL(stopUsingJoy(QObject*)), joy, SLOT(stopUsing(QObject*)));
    return joy;
}

void PythonFunctions::closeJoystick(Joystick *joy)
{
    disconnect(m_engine, SIGNAL(stopUsingJoy(QObject*)), joy, SLOT(stopUsing(QObject*)));
    joy->stopUsing(m_engine);
}

QStringList PythonFunctions::getJoystickNames()
{
    sJoyMgr.updateJoystickNames();
    return sJoyMgr.getNamesList();
}

QTimer *PythonFunctions::newTimer()
{
    QTimer *timer = new QTimer(this);
    m_engine->m_timers.push_back(timer);
    return timer;
}

void PythonFunctions::AddComboBoxItems(QComboBox *box, QStringList items)
{
    box->addItems(items);
}

void PythonFunctions::moveWidget(QWidget *w, int x, int y)
{
    w->move(x, y);
}

void PythonFunctions::resizeWidget(QWidget *w, int width, int height)
{
    w->resize(width, height);
}

DataWidget *PythonFunctions::newWidget(int type, QString title, int width, int height, int x, int y)
{
    DataWidget *w = m_engine->m_area->addWidget(QPoint(0,0), type);
    if(!w)
        return NULL;

    connect(w, SIGNAL(destroyed(QObject*)), m_engine, SLOT(widgetDestroyed(QObject*)));
    m_engine->m_widgets.insert(w->getId(), w);

    w->setTitle(title);

    if(width != -1 && height != -1)
        w->resize(width, height);

    if(x != -1 && y != -1)
        w->move(m_engine->m_x + x, m_engine->m_y + y);

    w->setWidgetControlled(m_engine->m_widget_id);
    w->setAcceptDrops(false);

    return w;
}
