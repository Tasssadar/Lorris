#ifndef SCRIPTENV_H
#define SCRIPTENV_H

#include <QObject>
#include <QScriptEngine>
#include <QScriptProgram>

#include "../../packet.h"

class ScriptEnv : public QScriptEngine
{
    Q_OBJECT

Q_SIGNALS:
    void clearTerm();
    void appendTerm(const QByteArray& text);

public:
    explicit ScriptEnv(QObject *parent = 0);

    void setSource(const QString& source);
    const QString& getSource() { return m_source; }

    QString dataChanged(analyzer_data *data);
    
public slots:
    void keyPressed(const QByteArray& key);

private:
    static QScriptValue __clearTerm(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue __appendTerm(QScriptContext *context, QScriptEngine *engine);

    QString m_source;
    QScriptEngine m_engine;
    QScriptValue  m_global;
    QScriptValue  m_on_data;
    QScriptValue  m_on_key;
};

#endif // SCRIPTENV_H
