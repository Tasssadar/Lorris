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
#include "../../../WorkTab/childtab.h"

#include "ui_scripteditor.h"

class QAbstractButton;
class LineNumber;
class QSyntaxHighlighter;
class EditorWidget;

class ScriptEditor : public ChildTab, private Ui::ScriptEditor
{
    Q_OBJECT

Q_SIGNALS:
    void applySource(bool close);
    void rejected();
    
public:
    ScriptEditor(const QString& source, const QString& filename, int type);
    ~ScriptEditor();

    QString getSource();
    int getEngine();

    QString getFilename() const
    {
        return m_filename;
    }

public slots:
    void addError(const QString& error);
    void clearErrors();
    void reject();

private slots:
    void on_buttonBox_clicked(QAbstractButton *btn);
    void on_loadBtn_clicked();
    void on_langBox_currentIndexChanged(int idx);
    void on_errorBtn_toggled(bool checked);
    void on_exampleBox_activated(int index);
    void on_saveBtn_clicked();
    void on_editorBox_currentIndexChanged(int idx);

    void textChanged();
    void saveAs();
    void clearStatus() { ui->statusLabel->setText(QString()); }
    void checkChange();
    void focusChanged(QWidget *prev, QWidget *now);

private:
    bool save(const QString& file);
    void updateExampleList();
    void setStatus(const QString& status);
    void setFilename(const QString& filename);

    Ui::ScriptEditor *ui;
    LineNumber *m_line_num;
    bool m_changed;
    bool m_contentChanged;
    bool m_ignoreNextFocus;
    bool m_ignoreFocus;
    quint32 m_errors;

    QString m_filename;
    QTimer m_status_timer;

    EditorWidget *m_editor;
};

class LineNumber : public QWidget
{
    Q_OBJECT
public:
    LineNumber(QWidget *parent = 0);

    void setLineNum(int lineNum);

public slots:
    void setScroll(int line);

protected:
    void paintEvent(QPaintEvent *event);

private:
    int m_char_h;
    int m_line_num;
    int m_scroll;
    quint8 m_last_w;
};


enum editors
{
    EDITOR_INTERNAL = 0,
    EDITOR_KATE,
    EDITOR_QSCINTILLA,

    EDITOR_MAX
};

class EditorWidget : public QWidget
{
    Q_OBJECT

Q_SIGNALS:
    void textChangedByUser();

public:
    static EditorWidget *getEditor(int type, QWidget *parent);
    static void fillEditorBox(QComboBox *box);

    virtual void setText(const QString& text) = 0;
    virtual QString getText() const = 0;

    virtual void setEngine(int idx) = 0;
    virtual QWidget *getWidget() = 0;
    virtual bool hasSettings() = 0;
    virtual int getType() const = 0;

    virtual void setModified(bool modded) { }

public slots:
    virtual void settingsBtn() { }

protected:
    EditorWidget(QWidget *parent = 0);
};

class EditorWidgetLorris : public EditorWidget
{
    Q_OBJECT
public:
    EditorWidgetLorris(QWidget *parent = 0);

    QWidget *getWidget() { return this; }

    void setText(const QString& text)
    {
        m_edit->setPlainText(text);
    }

    QString getText() const
    {
        return m_edit->toPlainText();
    }

    void setEngine(int idx);

    bool hasSettings() { return false; }
    int getType() const { return EDITOR_INTERNAL; }

private slots:
    void rangeChanged(int, int);
    void contentsChange(int /*position*/, int charsRemoved, int charsAdded);

private:
    LineNumber *m_lineNumber;
    QPlainTextEdit *m_edit;
    QSyntaxHighlighter *m_highlighter;
};

#ifdef USE_KATE

namespace KTextEditor {
    class Document;
    class View;
    class ConfigInterface;
}

enum cfg_variant;

class EditorWidgetKate : public EditorWidget
{
    Q_OBJECT
public:
    EditorWidgetKate(QWidget *parent);
    ~EditorWidgetKate();

    QWidget *getWidget();

    void setEngine(int idx);

    QString getText() const;
    void setText(const QString &text);

    bool hasSettings();
    int getType() const { return EDITOR_KATE; }

public slots:
    void settingsBtn();
    void modified(KTextEditor::Document*);

private:
    void save();
    void saveSettings(KTextEditor::ConfigInterface *iface, cfg_variant cfg);
    void loadSettings(KTextEditor::ConfigInterface *iface, cfg_variant cfg);

    KTextEditor::Document *m_doc;
    KTextEditor::View *m_view;
};

#endif // USE_KATE

#ifdef USE_QSCI

class QsciScintilla;

class EditorWidgetQSci : public EditorWidget
{
    Q_OBJECT
public:
    EditorWidgetQSci(QWidget *parent = 0);
    ~EditorWidgetQSci();

    void setText(const QString& text);
    QString getText() const;

    void setEngine(int idx);

    QWidget *getWidget();
    bool hasSettings() { return false; }
    int getType() const { return EDITOR_QSCINTILLA; }
    void setModified(bool modded);

private slots:
    void modified(bool mod);

private:
    QsciScintilla *m_editor;
};

#endif // USE QSCI

#endif // SCRIPTEDITOR_H

