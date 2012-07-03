/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef QTSCRIPTENGINE_H
#define QTSCRIPTENGINE_H

#include <QObject>
#include <QScriptEngine>
#include <QScriptProgram>
#include <QSize>

#include "../../../packet.h"
#include "../scriptstorage.h"
#include "scriptengine.h"

class WidgetArea;
class DataWidget;
class QtScriptEngine;

class QtScriptEngine_private : public QScriptEngine
{
    Q_OBJECT

    friend class QtScriptEngine;
private:
    QtScriptEngine_private(QtScriptEngine *base, QObject *parent);

    void clearTerm();
    void appendTerm(const QString& string);
    void appendTermRaw(const QByteArray& data);
    void SendData(const QByteArray& data);
    DataWidget *addWidget(quint8 type, QScriptContext *context, quint8 removeArg = 0);
    int getWidth();
    int getHeight();
    QScriptValue newTimer();

    static QScriptValue __clearTerm(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __appendTerm(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __sendData(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __getWidth(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __getHeight(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __throwException(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __getJoystick(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __closeJoystick(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __getJoystickNames(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newTimer(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __addComboBoxItems(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __moveWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __resizeWidget(QScriptContext *context, QScriptEngine *engine);

    static QScriptValue __newNumberWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newBarWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newColorWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newGraphWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newInputWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newCircleWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newWidget(QScriptContext *context, QScriptEngine *engine);

    QScriptValue  m_global;
    QtScriptEngine *m_base;
};

class QtScriptEngine : public ScriptEngine
{
    Q_OBJECT

friend class QtScriptEngine_private;

public:
    QtScriptEngine(WidgetArea *area , quint32 w_id, Terminal *terminal, QObject *parent = 0);
    ~QtScriptEngine();

    void setSource(const QString& source);
    const QString& getSource() { return m_source; }

    QString dataChanged(analyzer_data *data, quint32 index);
    DataWidget *addWidget(quint8 type, QScriptContext *context, quint8 removeArg = 0);

    QScriptValue newTimer();

    void onWidgetAdd(DataWidget *w);
    void onWidgetRemove(DataWidget *w);
    void callEventHandler(const QString& eventId);
    void onSave();

    void handleException(const QString& text)
    {
        emit error(text);
    }

public slots:
    void keyPressed(const QString &key);

private slots:
    void widgetDestroyed(QObject *widget);
    void onTitleChange(const QString& newTitle);

private:
    void prepareNewContext();

    QScriptValue *m_global;
    QScriptValue  m_on_data;
    QScriptValue  m_on_key;
    QScriptValue  m_on_widget_add;
    QScriptValue  m_on_widget_remove;
    QScriptValue  m_on_script_exit;
    QScriptValue  m_on_save;

    QtScriptEngine_private m_engine;
};

#endif // QTSCRIPTENGINE_H
