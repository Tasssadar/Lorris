/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "pythonhighlighter.h"

static const QString keywords[] =
{
    "def", "class", "from", "import", "for", "in", "while", "if", "elif", "else",
    "pass", "global", "return", "continue",

    QString()
};

static const QString consts[] =
{
    "True", "False", "None",

    QString()
};

PythonHighlighter::PythonHighlighter(QTextDocument *document) :
    QSyntaxHighlighter(document)
{
    QTextCharFormat fmt;
    fmt.setFontWeight(QFont::Bold);

    // keywords
    for(int i = 0; !keywords[i].isNull(); ++i)
        m_rules << rule("\\b" + keywords[i] + "\\b", 0, fmt);

    fmt.setForeground(Qt::darkRed);
    for(int i = 0; !consts[i].isNull(); ++i)
        m_rules << rule("\\b" + consts[i] + "\\b", 0, fmt);

    // numbers
    fmt = QTextCharFormat();
    fmt.setForeground(Qt::darkYellow);
    m_rules << rule("([0-9]+)", 0, fmt);

    // comments
    fmt = QTextCharFormat();
    fmt.setFontItalic(true);
    fmt.setForeground(Qt::darkGreen);
    m_rules << rule("#[^\\n]*", 0, fmt);

    // class/def
    fmt = QTextCharFormat();
    fmt.setFontUnderline(true);
    m_rules << rule("\\bdef\\b\\s*(\\w+)", 1, fmt)
            << rule("\\bclass\\b\\s*(\\w+)", 1, fmt);

    // strings
    fmt = QTextCharFormat();
    fmt.setForeground(QColor("#483d8b"));
    m_rules << rule("\"(.*)\"", 0, fmt);
    m_rules << rule("'(.*)'", 0, fmt);
}

void PythonHighlighter::highlightBlock(const QString &text)
{
    foreach(rule r, m_rules)
    {
        QRegExp exp(r.exp);
        int pos = exp.indexIn(text, 0);
        while(pos != -1)
        {
            pos = exp.pos(r.len);
            int len = exp.cap(r.len).length();
            setFormat(pos, len, r.fmt);
            pos = exp.indexIn(text, pos+exp.matchedLength());
        }
    }
}
