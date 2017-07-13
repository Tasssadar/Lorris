/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QComboBox>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <qscriptsyntaxhighlighter_p.h>
#include <QPainter>
#include <QHBoxLayout>

#include "pythonhighlighter.h"
#include "editorwidget.h"
#include "../misc/utils.h"

EditorWidget::EditorWidget(QWidget *parent) : QObject(parent)
{

}

EditorWidget *EditorWidget::getEditor(int type, QWidget *parent)
{
    switch(type)
    {
        case EDITOR_INTERNAL: return new EditorWidgetLorris(parent);
#ifdef USE_QSCI
        case EDITOR_QSCINTILLA: return new EditorWidgetQSci(parent);
#endif
    }
    return NULL;
}

void EditorWidget::fillEditorBox(QComboBox *box)
{
    box->addItem(tr("Internal (basic)"), QVariant(EDITOR_INTERNAL));

#ifdef USE_QSCI
    box->addItem(tr("QScintilla (advanced)"), QVariant(EDITOR_QSCINTILLA));
#endif
}

EditorWidgetLorris::EditorWidgetLorris(QWidget *parent) : EditorWidget(parent)
{
    m_widget = new QWidget(parent);

    QHBoxLayout *l = new QHBoxLayout(m_widget);
    m_widget->setStyleSheet("margin: 0px; padding: 0px");

    m_lineNumber = new LineNumber(m_widget);
    m_edit = new QPlainTextEdit(m_widget);

    l->addWidget(m_lineNumber);
    l->addWidget(m_edit, 1);

    m_highlighter = NULL;

    m_edit->setTabStopWidth(m_edit->fontMetrics().width(' ') * 4);

#ifdef Q_OS_MAC
    m_edit->setFont(Utils::getMonospaceFont(12));
#else
    m_edit->setFont(Utils::getMonospaceFont());
#endif

    QScrollBar *bar = m_edit->verticalScrollBar();
    connect(bar,    SIGNAL(rangeChanged(int,int)),           SLOT(rangeChanged(int,int)));
    connect(bar,    SIGNAL(valueChanged(int)), m_lineNumber, SLOT(setScroll(int)));
    connect(m_edit->document(), SIGNAL(contentsChange(int,int,int)), SLOT(contentsChange(int,int,int)));
    connect(m_edit, SIGNAL(undoAvailable(bool)),             SIGNAL(undoAvailable(bool)));
    connect(m_edit, SIGNAL(redoAvailable(bool)),             SIGNAL(redoAvailable(bool)));
}

EditorWidgetLorris::~EditorWidgetLorris()
{
    delete m_widget;
}

void EditorWidgetLorris::setText(const QString &text)
{
    m_edit->setPlainText(text);
}

QString EditorWidgetLorris::getText() const
{
    return m_edit->toPlainText();
}

void EditorWidgetLorris::rangeChanged(int, int)
{
    m_lineNumber->setScroll(m_edit->verticalScrollBar()->value());
}

void EditorWidgetLorris::contentsChange(int /*position*/, int charsRemoved, int charsAdded)
{
    if(charsRemoved != charsAdded)
        emit textChangedByUser();
    m_lineNumber->setLineNum(m_edit->document()->lineCount());
}

void EditorWidgetLorris::setHighlighter(EditorHighlight lang)
{
    delete m_highlighter;
    switch(lang)
    {
        case HIGHLIGHT_JSCRIPT:
            m_highlighter = new QScriptSyntaxHighlighter(m_edit->document());
            break;
        case HIGHLIGHT_PYTHON:
            m_highlighter = new PythonHighlighter(m_edit->document());
            break;
        default:
            m_highlighter = NULL;
            break;
    }
}

void EditorWidgetLorris::setReadOnly(bool readOnly)
{
    m_edit->setReadOnly(readOnly);
}

void EditorWidgetLorris::undo()
{
    m_edit->undo();
}

void EditorWidgetLorris::redo()
{
    m_edit->redo();
}

void EditorWidgetLorris::scrollToBottom()
{
    m_edit->verticalScrollBar()->setValue(m_edit->verticalScrollBar()->maximum());
}

LineNumber::LineNumber(QWidget *parent) : QWidget(parent)
{
#ifdef Q_OS_MAC
    setFont(Utils::getMonospaceFont(12));
#else
    setFont(Utils::getMonospaceFont());
#endif
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    setMinimumSize(5, 5);

    m_char_h = fontMetrics().height();
    m_line_num = 0;
    m_last_w = 0;
    m_scroll = 0;
}

void LineNumber::setLineNum(int lineNum)
{
    m_line_num = lineNum;
    update();
}

void LineNumber::setScroll(int line)
{
    m_scroll = line;
    update();
}

void LineNumber::paintEvent(QPaintEvent */*event*/)
{
    QPainter painter(this);

    int h = 5;
    for(int line = m_scroll; h+m_char_h < height() && line < m_line_num; ++line)
    {
        QString text;
        text.setNum(line+1);

        int text_w = fontMetrics().width(text);
        if(m_last_w < text_w)
        {
            m_last_w = text_w;
            setFixedWidth(text_w);
        }

        painter.drawText(0, h, m_last_w, m_char_h, Qt::AlignRight, text);
        h += m_char_h;
    }
}

#ifdef USE_QSCI

#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qscilexerpython.h>

#include <QShortcut>

EditorWidgetQSci::EditorWidgetQSci(QWidget *parent) : EditorWidget(parent)
{
    m_widget = new QWidget(parent);
    QVBoxLayout *l = new QVBoxLayout(m_widget);

    m_editor = new QsciScintilla(m_widget);
    m_editor->setMarginLineNumbers(QsciScintilla::NumberMargin, true);
    m_editor->setMarginWidth(QsciScintilla::NumberMargin, "12322");
    m_editor->setBraceMatching(QsciScintilla::SloppyBraceMatch);
    m_editor->setAutoCompletionSource(QsciScintilla::AcsAll);
    m_editor->setAutoCompletionThreshold(3);
    m_editor->setAutoIndent(true);
    m_editor->setIndentationsUseTabs(false);
    m_editor->setIndentationWidth(4);
    m_editor->setUtf8(true);
    m_editor->setEolMode(QsciScintilla::EolUnix);

    m_search_widget = new QWidget(m_widget);
    m_search_ui = new Ui::QSciSearchBar;
    m_search_ui->setupUi(m_search_widget);

    l->addWidget(m_editor, 1);
    l->addWidget(m_search_widget);

    m_search_widget->setVisible(false);
    setSearchBarReplaceVisible(false);

    m_canUndo = false;
    m_canRedo = false;

    QShortcut *find_s = new QShortcut(QKeySequence("Ctrl+F"), m_widget);
    QShortcut *replace_s = new QShortcut(QKeySequence("Ctrl+R"), m_widget);
    QShortcut *find_esc_s = new QShortcut(QKeySequence("Esc"), m_widget);

    connect(m_editor, SIGNAL(modificationChanged(bool)), SLOT(modified(bool)));
    connect(m_editor, SIGNAL(textChanged()),             SLOT(checkUndoRedo()));
    connect(m_search_ui->expandBtn, SIGNAL(clicked(bool)), SLOT(setSearchBarReplaceVisible(bool)));
    connect(m_search_ui->closeBtn, SIGNAL(clicked()),    SLOT(hideSearch()));
    connect(find_esc_s, SIGNAL(activated()),             SLOT(hideSearch()));
    connect(find_s, SIGNAL(activated()),                 SLOT(showSearch()));
    connect(replace_s, SIGNAL(activated()),              SLOT(showReplace()));
    connect(m_search_ui->nextBtn, SIGNAL(clicked()), SLOT(findNext()));
    connect(m_search_ui->prevBtn, SIGNAL(clicked()), SLOT(findPrev()));
    connect(m_search_ui->findEdit, SIGNAL(textEdited(QString)), SLOT(findNext()));
    connect(m_search_ui->replaceBtn, SIGNAL(clicked()), SLOT(replace()));
    connect(m_search_ui->replaceAllBtn, SIGNAL(clicked()), SLOT(replaceAll()));
    connect(m_search_ui->findEdit, SIGNAL(returnPressed()), SLOT(findNext()));
}

EditorWidgetQSci::~EditorWidgetQSci()
{
    QsciLexer *lex = m_editor->lexer();
    m_editor->setLexer(NULL);
    delete lex;

    delete m_editor;
    delete m_widget;
}

QString EditorWidgetQSci::getText() const
{
    return m_editor->text();
}

void EditorWidgetQSci::setText(const QString &text)
{
    m_editor->setText(text);
    m_editor->setModified(false);
    checkUndoRedo();
}

void EditorWidgetQSci::setHighlighter(EditorHighlight lang)
{
    QsciLexer *lex = m_editor->lexer();
    m_editor->setLexer(NULL);
    delete lex;

    switch(lang)
    {
        case HIGHLIGHT_JSCRIPT: {
            m_editor->setLexer(new QsciLexerJavaScript);
            QFont monospace = Utils::getMonospaceFont(10);
            m_editor->lexer()->setDefaultFont(monospace);
            m_editor->lexer()->setFont(monospace);
            break;
        }
        case HIGHLIGHT_PYTHON: {
            m_editor->setLexer(new QsciLexerPython);
            QFont monospace = Utils::getMonospaceFont(10);
            m_editor->lexer()->setDefaultFont(monospace);
            m_editor->lexer()->setFont(monospace);
            break;
        }
        default:
            m_editor->setLexer(NULL);
            return;
    }
}

QWidget *EditorWidgetQSci::getWidget()
{
    return m_widget;
}

void EditorWidgetQSci::scrollToBottom()
{
    m_editor->verticalScrollBar()->setValue(m_editor->verticalScrollBar()->maximum());
}

void EditorWidgetQSci::modified(bool mod)
{
    if(mod)
        emit textChangedByUser();
}

void EditorWidgetQSci::checkUndoRedo()
{
    bool can = m_editor->isUndoAvailable();
    if(can ^ m_canUndo)
    {
        emit undoAvailable(can);
        m_canUndo = can;
    }

    can = m_editor->isRedoAvailable();
    if(can ^ m_canRedo)
    {
        emit redoAvailable(can);
        m_canRedo = can;
    }
}

void EditorWidgetQSci::setModified(bool modded)
{
    m_editor->setModified(modded);
}

void EditorWidgetQSci::setReadOnly(bool readOnly)
{
    m_editor->setReadOnly(readOnly);
}

void EditorWidgetQSci::undo()
{
    m_editor->undo();
}

void EditorWidgetQSci::redo()
{
    m_editor->redo();
}

void EditorWidgetQSci::setSearchBarReplaceVisible(bool visible)
{
    QWidget *widgets[] = {
        m_search_ui->replaceLabel, m_search_ui->replaceEdit, m_search_ui->replaceBtn,
        m_search_ui->replaceAllBtn
    };

    for(size_t i = 0; i < sizeof_array(widgets); ++i)
        widgets[i]->setVisible(visible);

    m_search_ui->expandBtn->setChecked(visible);
}

void EditorWidgetQSci::showSearch()
{
    m_search_widget->show();
    m_search_ui->findEdit->setFocus();
}

void EditorWidgetQSci::showReplace()
{
    setSearchBarReplaceVisible(true);
    showSearch();
}

void EditorWidgetQSci::hideSearch()
{
    m_search_widget->hide();
    setSearchBarReplaceVisible(false);
    m_lastSearchForward = "";
    m_lastSearchBackward = "";
    m_search_ui->findEdit->clear();
    m_search_ui->replaceEdit->clear();
}

void EditorWidgetQSci::findNext()
{
    QString curSearch = m_search_ui->findEdit->text();
    bool caseSens = m_search_ui->caseBox->isChecked();
    if(curSearch.compare(m_lastSearchForward, caseSens ? Qt::CaseSensitive : Qt::CaseInsensitive) == 0)
        m_editor->findNext();
    else
    {
        int lineFrom, lineTo, idxFrom, idxTo;
        m_editor->getSelection(&lineFrom, &idxFrom, &lineTo, &idxTo);
        if(m_editor->findFirst(curSearch, false, caseSens, m_search_ui->wholeBox->isChecked(),
                true, true, lineFrom, idxFrom))
        {
            m_lastSearchForward = curSearch;
        }
    }
}

void EditorWidgetQSci::findPrev()
{
    QString curSearch = m_search_ui->findEdit->text();
    bool caseSens = m_search_ui->caseBox->isChecked();
    if(curSearch.compare(m_lastSearchBackward, caseSens ? Qt::CaseSensitive : Qt::CaseInsensitive) == 0)
        m_editor->findNext();
    else if(m_editor->findFirst(curSearch, false, caseSens, m_search_ui->wholeBox->isChecked(), true, false))
    {
        m_lastSearchBackward = curSearch;
    }
}

void EditorWidgetQSci::replace()
{
    QString curSearch = m_search_ui->findEdit->text();
    bool caseSens = m_search_ui->caseBox->isChecked();
    if(m_editor->selectedText().compare(curSearch, caseSens ? Qt::CaseSensitive : Qt::CaseInsensitive) != 0)
        findNext();

    if(m_editor->selectedText().compare(curSearch, caseSens ? Qt::CaseSensitive : Qt::CaseInsensitive) == 0)
    {
        m_editor->replaceSelectedText(m_search_ui->replaceEdit->text());
        findNext();
    }
}

void EditorWidgetQSci::replaceAll()
{
    QString curSearch = m_search_ui->findEdit->text();
    bool caseSens = m_search_ui->caseBox->isChecked();

    m_lastSearchForward = "";
    findNext();

    while(m_editor->selectedText().compare(curSearch, caseSens ? Qt::CaseSensitive : Qt::CaseInsensitive) == 0)
    {
        m_editor->replaceSelectedText(m_search_ui->replaceEdit->text());
        findNext();
    }
}

#endif // USE_QSCI
