/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SCRIPTWIDGET_H
#define SCRIPTWIDGET_H

#include <QTimer>
#include <QPointer>

#include "../datawidget.h"
#include "../../../misc/qtpointerarray.h"

class QLabel;
class ScriptEditor;
class ScriptEngine;
class Terminal;
class ExamplePreviewTab;
class QLineEdit;

class ScriptWidget : public DataWidget
{
    Q_OBJECT

Q_SIGNALS:
    void closeEdit();
    void setSourceDelayed(QString source);

public:
    ScriptWidget(QWidget *parent = 0);
    ~ScriptWidget();

    void setUp(Storage *);
    void saveWidgetInfo(DataFileParser *file);
    void loadWidgetInfo(DataFileParser *file);

    Terminal *getTerminal() const { return m_terminal; }
    QLineEdit *getInputEdit() const { return m_inputEdit; }

public slots:
    void onWidgetAdd(DataWidget *w);
    void onWidgetRemove(DataWidget *w);
    void onScriptEvent(const QString& eventId);

protected slots:
     void setSourceTriggered();
     void sourceSet(bool close);
     void closeEditor();
     void blinkError();
     void addExampleTab(const QString& name);
     void inputShowAct(bool show);
     void setSourceDirect(const QString& source);

protected:
     void newData(analyzer_data *data, quint32 index);
     void moveEvent(QMoveEvent *);
     void resizeEvent(QResizeEvent *);
     void titleDoubleClick();

     void createEngine();

     QPointer<ScriptEditor> m_editor;
     QtPointerArray<ExamplePreviewTab> m_examplePrevs;
     ScriptEngine *m_engine;
     int m_engine_type;
     Terminal *m_terminal;
     QLineEdit *m_inputEdit;
     QAction *m_inputAct;
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
