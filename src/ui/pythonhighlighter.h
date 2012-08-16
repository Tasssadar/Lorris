/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef PYTHONHIGHLIGHTER_H
#define PYTHONHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class PythonHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit PythonHighlighter(QTextDocument *document);
    
protected:
    void highlightBlock(const QString &text);

private:
    struct rule
    {
        rule(const QString& exp, int len, const QTextCharFormat& fmt)
        {
            this->exp = exp;
            this->len = len;
            this->fmt = fmt;
        }

        QString exp;
        int len;
        QTextCharFormat fmt;
    };

    QList<rule> m_rules;
};

#endif // PYTHONHIGHLIGHTER_H
