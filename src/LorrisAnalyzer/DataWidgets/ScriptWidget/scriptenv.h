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

#ifndef SCRIPTENV_H
#define SCRIPTENV_H

#include <QObject>
#include <QScriptEngine>
#include <QScriptProgram>
#include <QSize>

#include "../../packet.h"
#include "scriptstorage.h"

class WidgetArea;
class DataWidget;

class ScriptEnv : public QScriptEngine
{
    Q_OBJECT

Q_SIGNALS:
    void clearTerm();
    void appendTerm(const QString& text);
    void appendTermRaw(const QByteArray& text);
    void SendData(const QByteArray& data);

    void stopUsingJoy(QObject *object);

public:
    explicit ScriptEnv(WidgetArea *area , quint32 w_id, QObject *parent = 0);
    ~ScriptEnv();

    void setSource(const QString& source);
    const QString& getSource() { return m_source; }

    QString dataChanged(analyzer_data *data, quint32 index);
    DataWidget *addWidget(quint8 type, QScriptContext *context, quint8 removeArg = 0);

    void setPos(int x, int y)
    {
        m_x = x;
        m_y = y;
    }

    void setSize(const QSize& size)
    {
        m_width = size.width();
        m_height = size.height();
    }

    int getWidth() { return m_width; }
    int getHeight() { return m_height; }

    QScriptValue newTimer();

    void onWidgetAdd(DataWidget *w);
    void onWidgetRemove(DataWidget *w);
    void callEventHandler(const QString& eventId);
    void onSave();

    ScriptStorage *getStorage() const
    {
        return m_storage;
    }

public slots:
    void keyPressed(const QString &key);

private slots:
    void widgetDestroyed(QObject *widget);
    void onTitleChange(const QString& newTitle);

private:
    void prepareNewContext();
    QString sanitizeWidgetName(QString const & name);

    static QScriptValue __clearTerm(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __appendTerm(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __sendData(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __getWidth(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __getHeight(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __throwException(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __getJoystick(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __getJoystickNames(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newTimer(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __addComboBoxItems(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __moveWidget(QScriptContext *context, QScriptEngine *engine);

    static QScriptValue __newNumberWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newBarWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newColorWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newGraphWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newInputWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newWidget(QScriptContext *context, QScriptEngine *engine);

    QString m_source;
    WidgetArea *m_area;
    QScriptEngine m_engine;
    QScriptValue  m_global;
    QScriptValue  m_on_data;
    QScriptValue  m_on_key;
    QScriptValue  m_on_widget_add;
    QScriptValue  m_on_widget_remove;
    QScriptValue  m_on_script_exit;
    QScriptValue  m_on_save;

    qint32 m_widget_id;
    int m_x;
    int m_y;
    int m_width;
    int m_height;

    QHash<quint32, DataWidget*> m_widgets;
    std::list<QTimer*> m_timers;

    ScriptStorage *m_storage;
};

#endif // SCRIPTENV_H
