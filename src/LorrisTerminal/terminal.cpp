/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include <QScrollBar>
#include <QKeyEvent>

#include "terminal.h"

Terminal::Terminal(QWidget *parent) : QPlainTextEdit(parent)
{
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

    QFont font;
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
        if(sb->value() == sb->maximum())
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
        sb->setValue(sb->maximum());
    }
}

void Terminal::updateEditText()
{
    setPlainText(content);
    sb->setValue(sb->maximum());
}

void Terminal::keyPressEvent(QKeyEvent *event)
{
    QByteArray key = event->text().toAscii();
    emit keyPressedASCII(key);
}
