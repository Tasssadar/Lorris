/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef PYTHONENGINE_H
#define PYTHONENGINE_H

#include "../../../../../dep/pythonqt/src/PythonQt.h"

#include "scriptengine.h"

class PythonEngine : public ScriptEngine
{
    Q_OBJECT
public:
    PythonEngine(WidgetArea *area, quint32 w_id, Terminal *terminal, QObject *parent = 0);
    
    void setSource(const QString& source);
    QString dataChanged(analyzer_data *data, quint32 index);
    void onWidgetAdd(DataWidget *w);
    void onWidgetRemove(DataWidget *w);
    void callEventHandler(const QString& eventId);
    void onSave();

public slots:
    void keyPressed(const QString &key);

private:
    static QString getNewModuleName();

    PythonQtObjectPtr m_module;
};

#endif // PYTHONENGINE_H
