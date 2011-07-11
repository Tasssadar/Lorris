#include <QScrollBar>
#include <QKeyEvent>

#include "terminal.h"

Terminal::Terminal(QWidget *parent) : QTextEdit(parent)
{
    content = "";

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setShown(true);
    setReadOnly(true);

    QColor color_black(0, 0, 0);\
    QColor color_white(255, 255, 255);\
    QPalette palette;
    palette.setColor(QPalette::Base, color_black);
    palette.setColor(QPalette::Text, color_white);
    setPalette(palette);

    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    setFont(font);
}

Terminal::~Terminal()
{

}

void Terminal::appendText(QString text, bool toEdit)
{
    if(text.contains(QChar('\f')))
    {
        content = "";
        setText("");
        qint32 index = -1;
        while(true)
        {
            index = text.indexOf(QChar('\f'), index+1);
            if(index == -1)
                break;
            text.remove(index, 1);
        }
    }

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
