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

#include "../../packet.h"

class AnalyzerDataArea;
class DataWidget;

class ScriptEnv : public QScriptEngine
{
    Q_OBJECT

Q_SIGNALS:
    void clearTerm();
    void appendTerm(const QByteArray& text);
    void SendData(const QByteArray& data);

public:
    explicit ScriptEnv(AnalyzerDataArea *area , quint32 w_id, QObject *parent = 0);

    void setSource(const QString& source);
    const QString& getSource() { return m_source; }

    QString dataChanged(analyzer_data *data, quint32 index);
    DataWidget *addWidget(quint8 type, QScriptContext *context);

    void setPos(int x, int y)
    {
        m_x = x;
        m_y = y;
    }

public slots:
    void keyPressed(const QByteArray& key);

private:
    void prepareNewContext();

    static QScriptValue __clearTerm(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __appendTerm(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __sendData(QScriptContext *context, QScriptEngine *engine);

    static QScriptValue __newNumberWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newBarWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newColorWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newGraphWidget(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __newInputWidget(QScriptContext *context, QScriptEngine *engine);

    QString m_source;
    AnalyzerDataArea *m_area;
    QScriptEngine m_engine;
    QScriptValue  m_global;
    QScriptValue  m_on_data;
    QScriptValue  m_on_key;

    qint32 m_widget_id;
    int m_x;
    int m_y;

    std::list<DataWidget*> m_widgets;
};

#endif // SCRIPTENV_H
