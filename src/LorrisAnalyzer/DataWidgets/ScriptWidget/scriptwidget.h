/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SCRIPTWIDGET_H
#define SCRIPTWIDGET_H

#include <QTimer>

#include "../datawidget.h"

class QLabel;
class ScriptEditor;
class ScriptEngine;
class Terminal;

class ScriptWidget : public DataWidget
{
    Q_OBJECT
public:
    ScriptWidget(QWidget *parent = 0);
    ~ScriptWidget();

    void setUp(Storage *);
    void saveWidgetInfo(DataFileParser *file);
    void loadWidgetInfo(DataFileParser *file);

public slots:
    void onWidgetAdd(DataWidget *w);
    void onWidgetRemove(DataWidget *w);
    void onScriptEvent(const QString& eventId);

protected slots:
     void setSourceTriggered();
     void sourceSet(bool close);
     void editorRejected();
     void blinkError();

protected:
     void newData(analyzer_data *data, quint32 index);
     void moveEvent(QMoveEvent *);
     void resizeEvent(QResizeEvent *);

     void createEngine();

     ScriptEditor *m_editor;
     ScriptEngine *m_engine;
     int m_engine_type;
     Terminal *m_terminal;
     QString m_filename;
     QLabel *m_error_label;
     QTimer m_error_blink_timer;
};

class ScriptWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    ScriptWidgetAddBtn(QWidget *parent = 0);

};
#endif // SCRIPTWIDGET_H
