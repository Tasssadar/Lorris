#ifndef TERMINAL_H
#define TERMINAL_H

#include <QString>
#include <QTextEdit>

class QByteArray;

class Terminal : public QTextEdit
{
    Q_OBJECT

Q_SIGNALS:
    void keyPressedASCII(QByteArray key);

public:
    Terminal(QWidget *parent);
    ~Terminal();

    void appendText(QString text, bool toEdit = true);
    void setTextTerm(QString text, bool toEdit = true);
    QString getText() { return content; }
    void updateEditText();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    QString content;
};

#endif // TERMINAL_H
