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
#include "../../../ui/hookedlineedit.h"

class QLabel;
class ScriptEditor;
class ScriptEngine;
class Terminal;
class ExamplePreviewTab;

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
    HookedLineEdit *getInputEdit() const { return m_inputEdit; }

public slots:
    void onWidgetAdd(DataWidget *w);
    void onWidgetRemove(DataWidget *w);
    void onScriptEvent(const QString& eventId);

protected slots:
     void setSourceTriggered();
     void sourceSet();
     void closeEditor();
     void blinkError(const QString& text);
     void addExampleTab(const QString& name);
     void inputShowAct(bool show);
     void setSourceDirect(const QString& source);
     void inputLineKeyPressed(int keyCode);
     void inputLineKeyReleased(int keyCode);

protected:
     void newData(analyzer_data *data, quint32 index);
     void moveEvent(QMoveEvent *);
     void resizeEvent(QResizeEvent *);
     void titleDoubleClick();

     void createEngine();
     void clearErrors();

     QPointer<ScriptEditor> m_editor;
     QtPointerArray<ExamplePreviewTab> m_examplePrevs;
     ScriptEngine *m_engine;
     int m_engine_type;
     Terminal *m_terminal;
     HookedLineEdit *m_inputEdit;
     QAction *m_inputAct;
     QString m_filename;
     QString m_errors;
};

#endif // SCRIPTWIDGET_H
