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

public:
    static EditorWidget *getEditor(int type, QWidget *parent);
    static void fillEditorBox(QComboBox *box);

    virtual void setText(const QString& text) = 0;
    virtual QString getText() const = 0;

    virtual void setHighlighter(EditorHighlight lang) = 0;
    virtual QWidget *getWidget() = 0;
    virtual bool hasSettings() = 0;
    virtual int getType() const = 0;

    virtual void setModified(bool /*modded*/) { }
    virtual void setReadOnly(bool readOnly) = 0;

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
    ~EditorWidgetLorris();

    QWidget *getWidget() { return m_widget; }

    void setText(const QString& text);
    QString getText() const;

    void setHighlighter(EditorHighlight lang);

    bool hasSettings() { return false; }
    int getType() const { return EDITOR_INTERNAL; }
    void setReadOnly(bool readOnly);

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

    void setModified(bool modded);
    void setReadOnly(bool readOnly);

public slots:
    void settingsBtn();
    void modified(KTextEditor::Document*);

protected:
    bool eventFilter(QObject *, QEvent *ev);

private:
    void save();
    void saveSettings(KTextEditor::ConfigInterface *iface, cfg_variant cfg);
    void loadSettings(KTextEditor::ConfigInterface *iface, cfg_variant cfg);

    KTextEditor::Document *m_doc;
    KTextEditor::View *m_view;
    bool m_readOnly;
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

    void setHighlighter(EditorHighlight lang);

    QWidget *getWidget();
    bool hasSettings() { return false; }
    int getType() const { return EDITOR_QSCINTILLA; }
    void setModified(bool modded);
    void setReadOnly(bool readOnly);

private slots:
    void modified(bool mod);

private:
    QsciScintilla *m_editor;
};

#endif // USE QSCI

#endif // EDITORWIDGET_H
