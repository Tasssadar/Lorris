/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QDialog>
#include <QTimer>
#include <QScrollArea>
#include <QPointer>
#include "../../../WorkTab/childtab.h"

#include "ui_scripteditor.h"

class QAbstractButton;
class LineNumber;
class QSyntaxHighlighter;
class EditorWidget;
class ExamplesPreview;

class ScriptEditor : public ChildTab, private Ui::ScriptEditor
{
    Q_OBJECT

Q_SIGNALS:
    void applySource(bool close);
    void rejected();
    void openPreview(const QString& name);
    
public:
    ScriptEditor(const QString& source, const QString& filename, int type);
    ~ScriptEditor();

    void setSource(const QString& source);
    QString getSource();
    int getEngine();

    QString getFilename() const
    {
        return m_filename;
    }

    bool onTabClose();

public slots:
    void addError(const QString& error);
    void clearErrors();
    void reject();

private slots:
    void on_buttonBox_clicked(QAbstractButton *btn);
    void on_loadBtn_clicked();
    void on_langBox_currentIndexChanged(int idx);
    void on_errorBtn_toggled(bool checked);
    void on_editorBox_currentIndexChanged(int idx);
    void on_saveBtn_clicked();
    void on_exampleBtn_clicked();

    void textChanged();
    void saveAs();
    void clearStatus() { ui->statusLabel->setText(QString()); }
    void checkChange();
    void focusChanged(QWidget *prev, QWidget *now);
    void examplePreviewDestroyed();
    void loadExample(const QString& name);

private:
    bool save(const QString& file);
    void setStatus(const QString& status);
    void setFilename(const QString& filename);

    Ui::ScriptEditor *ui;
    LineNumber *m_line_num;
    bool m_changed;
    bool m_contentChanged;
    bool m_fileChanged;
    bool m_ignoreNextFocus;
    bool m_ignoreFocus;
    quint32 m_errors;

    QString m_filename;
    QTimer m_status_timer;

    EditorWidget *m_editor;
    QPointer<ExamplesPreview> m_examples;
};

class ExamplesPreview : public QScrollArea
{
    Q_OBJECT

Q_SIGNALS:
    void openInEditor(const QString& file);
    void openPreview(const QString& file);

public:
    ExamplesPreview(int engine, QWidget *parent);

private slots:
    void focusChanged(QWidget *, QWidget *to);

private:
    QFrame *getSeparator();
};

class ExamplePreviewItem : public QFrame
{
    Q_OBJECT

Q_SIGNALS:
    void openInEditor(const QString& file);
    void openPreview(const QString& file);

public:
    ExamplePreviewItem(const QString& name, QString line, QWidget *parent);
};

class ExamplePreviewTab : public ChildTab
{
    Q_OBJECT
public:
    ExamplePreviewTab(const QString& name);

    void loadExample(const QString& name);

private:
    EditorWidget *m_editor;
};

#endif // SCRIPTEDITOR_H

