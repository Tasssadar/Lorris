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
class QComboBox;
class SettingsPopup;

class ScriptEditor : public ChildTab, private Ui::ScriptEditor
{
    Q_OBJECT

Q_SIGNALS:
    void applySource();
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

    int getEditorType() const;
    EditorWidget *getEditor() const { return m_editor; }

    bool onTabClose();

public slots:
    void addError(const QString& error);
    void clearErrors();
    void setEditor(int editorType);
    void setLanguage(int lang);

private slots:
    void on_errorBtn_toggled(bool checked);

    void loadAct();
    void saveAct();
    void applyAct();
    void exampleBtn();
    void settingsBtn();
    void textChanged();
    void saveAs();
    void clearStatus() { ui->statusLabel->setText(QString()); }
    void checkChange();
    void focusChanged(QWidget *prev, QWidget *now);
    void examplePreviewDestroyed();
    void settingsDestroyed();
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
    int m_language;

    QString m_filename;
    QTimer m_status_timer;

    QPushButton *m_exampleBtn;
    QPushButton *m_settingsBtn;

    EditorWidget *m_editor;
    QPointer<ExamplesPreview> m_examples;
    QPointer<SettingsPopup> m_settings;
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

class SettingsPopup : public QFrame
{
    Q_OBJECT
public:
    SettingsPopup(ScriptEditor *editor);

private slots:
    void focusChanged(QWidget *, QWidget *to);
    void editorChanged(int idx);

private:
    bool isAncestorOf(const QWidget *child) const;
    inline ScriptEditor *editor() { return (ScriptEditor*)parent(); }

    QComboBox *m_editorBox;
    QPushButton *m_editorSett;
};

#endif // SCRIPTEDITOR_H

