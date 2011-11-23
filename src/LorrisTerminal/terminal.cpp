#include <QScrollBar>
#include <QKeyEvent>

#include "terminal.h"

Terminal::Terminal(QWidget *parent) : QPlainTextEdit(parent)
{
    autoScroll = true;
    sb = verticalScrollBar();

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setShown(true);
    setReadOnly(true);

    QColor color_black(0, 0, 0);
    QColor color_white(255, 255, 255);
    QPalette palette;
    palette.setColor(QPalette::Base, color_black);
    palette.setColor(QPalette::Text, color_white);
    setPalette(palette);

    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    setFont(font);

    connect(sb, SIGNAL(valueChanged(int)), this, SLOT(scrollPosChanged(int)));
}

Terminal::~Terminal()
{

}

void Terminal::scrollPosChanged(int value)
{
    if(value == sb->maximum())
        autoScroll = true;
    else if(autoScroll)
        autoScroll = false;
}

void Terminal::appendText(QString text, bool toEdit)
{
    if(text.contains(QChar('\f')))
    {
        content = "";
        if(toEdit)
            setPlainText("");
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
        setUpdatesEnabled(false);
        if(autoScroll)
        {
            moveCursor(QTextCursor::End);
            insertPlainText(text);
            sb->setValue(sb->maximum());
        }
        else
        {
            int val = sb->value();
            moveCursor(QTextCursor::End);
            insertPlainText(text);
            sb->setValue(val);
        }
        setUpdatesEnabled(true);
    }
}

void Terminal::setTextTerm(QString text, bool toEdit)
{
    content = text;
    if(toEdit)
    {
        setPlainText(text);
        autoScroll = true;
        sb->setValue(sb->maximum());
    }
}

void Terminal::updateEditText()
{
    setPlainText(content);
    autoScroll = true;
    sb->setValue(sb->maximum());
}

void Terminal::keyPressEvent(QKeyEvent *event)
{
    QByteArray key = event->text().toAscii();
    emit keyPressedASCII(key);
}
