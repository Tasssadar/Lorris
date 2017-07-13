/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QWidget>
#include "../misc/config.h"

class QSyntaxHighlighter;
class QPlainTextEdit;
class LineNumber;
class QComboBox;

enum editors
{
    EDITOR_INTERNAL = 0,
    EDITOR_KATE,
    EDITOR_QSCINTILLA,

    EDITOR_MAX
};

enum EditorHighlight
{
    HIGHLIGHT_JSCRIPT  = 0,
    HIGHLIGHT_PYTHON
};

class EditorWidget : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void textChangedByUser();
    void applyShortcutPressed();
    void undoAvailable(bool available);
    void redoAvailable(bool available);

public:
    static EditorWidget *getEditor(int type, QWidget *parent);
    static void fillEditorBox(QComboBox *box);

    virtual void setText(const QString& text) = 0;
    virtual QString getText() const = 0;

    virtual void setHighlighter(EditorHighlight lang) = 0;
    virtual QWidget *getWidget() = 0;
    virtual bool hasSettings() = 0;
    virtual int getType() const = 0;
    virtual QString getUndoShortcut() const = 0;
    virtual QString getRedoShortcut() const = 0;

    virtual void setModified(bool /*modded*/) { }
    virtual void setReadOnly(bool readOnly) = 0;

    virtual void scrollToBottom() = 0;

public slots:
    virtual void settingsBtn() { }
    virtual void undo() = 0;
    virtual void redo() = 0;

protected:
    EditorWidget(QWidget *parent = 0);
};

class EditorWidgetLorris : public EditorWidget
{
    Q_OBJECT
public:
    EditorWidgetLorris(QWidget *parent = 0);
    ~EditorWidgetLorris();

    QWidget *getWidget() { return m_widget; }

    void setText(const QString& text);
    QString getText() const;

    void setHighlighter(EditorHighlight lang);

    bool hasSettings() { return false; }
    int getType() const { return EDITOR_INTERNAL; }
    void setReadOnly(bool readOnly);

    QString getUndoShortcut() const { return "Ctrl+Z"; }
    QString getRedoShortcut() const { return "Ctrl+Shift+Z"; }

    void scrollToBottom();

public slots:
    void undo();
    void redo();

private slots:
    void rangeChanged(int, int);
    void contentsChange(int /*position*/, int charsRemoved, int charsAdded);

private:
    LineNumber *m_lineNumber;
    QPlainTextEdit *m_edit;
    QSyntaxHighlighter *m_highlighter;
    QWidget *m_widget;
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

#ifdef USE_KATE

namespace KTextEditor {
    class Document;
    class View;
    class ConfigInterface;
}

class EditorWidgetKate : public EditorWidget
{
    Q_OBJECT
public:
    EditorWidgetKate(QWidget *parent);
    ~EditorWidgetKate();

    QWidget *getWidget();

    void setHighlighter(EditorHighlight lang);

    QString getText() const;
    void setText(const QString &text);

    bool hasSettings();
    int getType() const { return EDITOR_KATE; }

    QString getUndoShortcut() const { return "Ctrl+Z"; }
    QString getRedoShortcut() const { return "Ctrl+Shift+Z"; }

    void setModified(bool modded);
    void setReadOnly(bool readOnly);

    void scrollToBottom() { /* FIXME */ }

public slots:
    void settingsBtn();
    void undo();
    void redo();

protected:
    bool eventFilter(QObject *, QEvent *ev);

private slots:
    void modified(KTextEditor::Document*);

private:
    void save();
    void saveSettings(KTextEditor::ConfigInterface *iface, cfg_variant cfg);
    void loadSettings(KTextEditor::ConfigInterface *iface, cfg_variant cfg);

    KTextEditor::Document *m_doc;
    KTextEditor::View *m_view;
    bool m_readOnly;
    bool m_canUndo;
    bool m_canRedo;
};

#endif // USE_KATE

#ifdef USE_QSCI

class QsciScintilla;

#include "ui_qscisearchbar.h"

class EditorWidgetQSci : public EditorWidget
{
    Q_OBJECT
public:
    EditorWidgetQSci(QWidget *parent = 0);
    ~EditorWidgetQSci();

    void setText(const QString& text);
    QString getText() const;

    void setHighlighter(EditorHighlight lang);

    QWidget *getWidget();
    bool hasSettings() { return false; }
    int getType() const { return EDITOR_QSCINTILLA; }
    void setModified(bool modded);
    void setReadOnly(bool readOnly);

    QString getUndoShortcut() const { return "Ctrl+Z"; }
    QString getRedoShortcut() const { return "Ctrl+Y"; }

    void scrollToBottom();

public slots:
    void undo();
    void redo();

private slots:
    void modified(bool mod);
    void checkUndoRedo();
    void setSearchBarReplaceVisible(bool visible);
    void hideSearch();
    void showSearch();
    void showReplace();
    void findNext();
    void findPrev();
    void replace();
    void replaceAll();

private:
    QsciScintilla *m_editor;
    QWidget *m_widget;
    QWidget *m_search_widget;
    Ui::QSciSearchBar *m_search_ui;
    bool m_canUndo;
    bool m_canRedo;
    QString m_lastSearchForward;
    QString m_lastSearchBackward;
};

#endif // USE QSCI

#endif // EDITORWIDGET_H
