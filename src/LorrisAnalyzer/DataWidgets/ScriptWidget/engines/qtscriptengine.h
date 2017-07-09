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
#include <QTimer>

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
Q_SIGNALS:
    void stopUsingJoy(QObject *object);

public:
    QScriptValue evaluate(const QString &program, const QString &fileName = QString(), int lineNumber = 1);
    void startFreezeDetector();
    void stopFreezeDetector();

private slots:
    void freezeDetectorTimeout();

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
    quint32 getDataCount() const;
    QByteArray *getData(quint32 idx) const;

    static QScriptValue __clearTerm(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __appendTerm(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __sendData(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __getWidth(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __getHeight(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __throwException(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __getJoystick(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __getFirstJoystick(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __closeJoystick(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __getJoystickNames(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __getJoystickIds(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newTimer(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __addComboBoxItems(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __moveWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __resizeWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __getData(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __getDataCount(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __playErrorSound(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __setMaxPacketNumber(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __setInterval(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __setTimeout(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __clearTimer(QScriptContext *context, QScriptEngine *engine);

    static QScriptValue __newNumberWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newBarWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newColorWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newGraphWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newInputWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newCircleWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newSliderWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newCanvasWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newWidget(QScriptContext *context, QScriptEngine *engine);

    QScriptValue  m_global;
    QtScriptEngine *m_base;
    QTimer m_freezeDetectTimer;
};

class QtScriptEngine : public ScriptEngine
{
    Q_OBJECT

friend class QtScriptEngine_private;

public:
    QtScriptEngine(WidgetArea *area , quint32 w_id, ScriptWidget *parent);
    ~QtScriptEngine();

    void setSource(const QString& source);
    const QString& getSource() { return m_source; }

    QString dataChanged(analyzer_data *data, quint32 index);
    DataWidget *addWidget(quint8 type, QScriptContext *context, quint8 removeArg = 0);

    QTimer *newTimer();

    void onWidgetAdd(DataWidget *w);
    void onWidgetRemove(DataWidget *w);
    void callEventHandler(const QString& eventId, const QVariantList& args = QVariantList());
    void onSave();

    void handleException(const QString& text)
    {
        emit error(text);
    }

public slots:
    void keyPressed(const QString &key);
    void rawData(const QByteArray& data);

private slots:
    void widgetDestroyed(QObject *widget);
    void onTitleChange(const QString& newTitle);

private:
    void prepareNewContext();

    QScriptValue  m_global;
    QScriptValue  m_on_data;
    QScriptValue  m_on_key;
    QScriptValue  m_on_widget_add;
    QScriptValue  m_on_widget_remove;
    QScriptValue  m_on_script_exit;
    QScriptValue  m_on_save;
    QScriptValue  m_on_raw;

    QtScriptEngine_private *m_engine;
};

class QtScriptTimerCallback : public QObject {
    Q_OBJECT
public:
    QtScriptTimerCallback(const QScriptValue& this_object, const QScriptValue& callback, QObject *parent);
    ~QtScriptTimerCallback();

public slots:
    void trigger();

private:
    QScriptValue m_this_object;
    QScriptValue m_callback;
};

#endif // QTSCRIPTENGINE_H
