#ifndef TERMINAL_H
#define TERMINAL_H

#include <QString>

class Terminal
{
public:
    Terminal() { content = ""; }
    ~Terminal() { }

    void appendText(QString text) { content += text; }
    void setText(QString text) { content = text; }
    QString getText() { return content; }

private:
    QString content;
};

#endif // TERMINAL_H
