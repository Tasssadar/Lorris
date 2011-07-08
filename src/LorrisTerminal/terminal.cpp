#include <QScrollBar>
#include <QKeyEvent>

#include "terminal.h"

Terminal::Terminal(QWidget *parent) : QTextEdit(parent)
{
    content = "";
}

Terminal::~Terminal()
{

}

void Terminal::appendText(QString text, bool toEdit)
{
    content += text;
    if(toEdit)
    {
        moveCursor(QTextCursor::End);
        insertPlainText(text);
        QScrollBar *sb = verticalScrollBar();
        sb->setValue(sb->maximum());
    }
}

void Terminal::setTextTerm(QString text, bool toEdit)
{
    content = text;
    if(toEdit)
    {
        setText(text);
        QScrollBar *sb = verticalScrollBar();
        sb->setValue(sb->maximum());
    }
}

void Terminal::updateEditText()
{
    setText(content);
    QScrollBar *sb = verticalScrollBar();
    sb->setValue(sb->maximum());
}

void Terminal::keyPressEvent(QKeyEvent *event)
{
    QByteArray key = event->text().toAscii();
    emit keyPressedASCII(key);
}
