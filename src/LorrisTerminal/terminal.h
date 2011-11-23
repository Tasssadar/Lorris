#ifndef TERMINAL_H
#define TERMINAL_H

#include <QString>
#include <QPlainTextEdit>

class QByteArray;

class Terminal : public QPlainTextEdit
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

private slots:
    void scrollPosChanged(int value);

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    QString content;
    QScrollBar *sb;
    bool autoScroll;
};

#endif // TERMINAL_H
