/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SCRIPTWIDGET_H
#define SCRIPTWIDGET_H

#include "../datawidget.h"

class QLabel;
class ScriptEditor;
class ScriptEnv;
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

protected:
    void newData(analyzer_data *data, quint32 index);
    void moveEvent(QMoveEvent *);
    void resizeEvent(QResizeEvent *);

protected slots:
     void setSourceTriggered();
     void sourceSet(bool close);

protected:
     ScriptEditor *m_editor;
     ScriptEnv *m_env;
     Terminal *m_terminal;
};

class ScriptWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    ScriptWidgetAddBtn(QWidget *parent = 0);

};
#endif // SCRIPTWIDGET_H
