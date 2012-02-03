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

#define QT_USE_FAST_CONCATENATION

#include <QApplication>
#include <QScrollBar>
#include <QKeyEvent>
#include <QPainter>
#include <QTextBlock>
#include <QFile>
#include <QTextStream>
#include <QResizeEvent>
#include <QClipboard>

#include "common.h"
#include "terminal.h"

#define CHAR_HEIGHT 15
#define CHAR_WIDTH  8

Terminal::Terminal(QWidget *parent) : QAbstractScrollArea(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setShown(true);

    QColor color_black(0, 0, 0);
    QColor color_white(255, 255, 255);
    QPalette palette;
    palette.setColor(QPalette::Base, color_black);
    palette.setColor(QPalette::Text, color_white);
    setPalette(palette);

    setFont(Utils::getMonospaceFont());

    m_paused = false;
    m_fmt = FMT_TEXT;
    m_hex_pos = 0;

    m_cursor.setSize(QSize(CHAR_WIDTH, CHAR_HEIGHT));
    updateScrollBars();

}

Terminal::~Terminal()
{

}

void Terminal::appendText(QByteArray text)
{
    m_data.append(text);

    switch(m_fmt)
    {
        case FMT_TEXT: addLines(text); break;
        case FMT_HEX:  addHex();       break;
    }

    if(!m_paused)
    {
        updateScrollBars();
        viewport()->update();
    }
}

void Terminal::addLines(QByteArray text)
{
    quint32 size = 100;
    char *line = new char[size];
    char *itr = line;
    quint32 pos = m_cursor_pos.y();
    if(pos < m_lines.size())
    {
        std::string s = m_lines[pos].toStdString();
        if(s.length() >= size)
        {
            while(s.length() >= size)
                size *= 2;
            delete[] line;
            line = new char[size];
        }

        std::copy(s.c_str(), s.c_str()+s.length(), line);
        itr = line + m_cursor_pos.x();
    }

    for(quint32 i = 0; i < (quint32)text.size(); ++i)
    {
        switch(text[i])
        {
            case '\f':
            {
                addLine(pos, line, itr);
                m_cursor_pos.setX(0);
                m_cursor_pos.setY(0);
                pos = 0;
                break;
            }
            case '\r':
                itr = line;
                m_cursor_pos.setX(0);
                break;
            case '\n':
            {
                addLine(pos, line, itr);
                ++pos;
                m_cursor_pos.setX(0);
                m_cursor_pos.setY(pos);
                break;
            }
            default:
            {
                *itr = text[i];
                ++itr;
                if(itr - line >= size)
                {
                    quint32 diff = itr - line;
                    size *= 2;
                    char *tmp = new char[size];
                    std::copy(line, line+size, tmp);

                    delete[] line;
                    line = tmp;
                    itr = line + diff;
                }
                *itr = 0;
                break;
            }
        }
    }

    if(itr != line)
        addLine(pos, line, itr);

    delete[] line;
}

void Terminal::addHex()
{
    QByteArray chunk;
    if(m_hex_pos%16 != 0)
    {
        m_hex_pos -= m_hex_pos%16;
        m_lines.pop_back();
    }

    for(int i = m_hex_pos; i < m_data.length(); i += 16)
    {
        chunk = m_data.mid(i, 16);
        QString line = QString("%1 ").arg(m_hex_pos, 8, 16, QChar('0')).toUpper();
        m_hex_pos += chunk.size();

        for(int x = 0; x < chunk.size(); ++x)
            line += Utils::hexToString(chunk[x]) % " ";

        line += QString("%1").arg("", 3 + (16 - chunk.size())*3, QChar(' '));

        Utils::parseForHexEditor(chunk);
        line += "|" % chunk % "|";
        m_lines.push_back(line);
    }
    m_cursor_pos.setY(m_lines.size());
    m_cursor_pos.setX(0);
}

void Terminal::addLine(quint32 pos, char *&line, char *&itr)
{
    std::vector<QString>::iterator linePos = m_lines.begin() + pos;
    if(linePos != m_lines.end())
        linePos = m_lines.erase(linePos);
    m_lines.insert(linePos, QString(line));
    m_cursor_pos.setY(pos);
    m_cursor_pos.setX(m_lines[pos].length());
    itr = line;
}

void Terminal::keyPressEvent(QKeyEvent *event)
{
    if((event->modifiers() & Qt::ControlModifier))
    {

        switch(event->key())
        {
            case Qt::Key_C:
            case Qt::Key_X:
                copyToClipboard();
                return;
            case Qt::Key_A:
                selectAll();
                return;
        }
    }
    QByteArray key = event->text().toAscii();
    emit keyPressedASCII(key);
}

void Terminal::copyToClipboard()
{
    QString text;
    QString line;
    int start = m_sel_start.x();
    int stop;
    for(quint32 i = m_sel_start.y(); i <= (quint32)m_sel_stop.y(); ++i)
    {
        if(i >= lines().size())
            break;

        if(i == (quint32)m_sel_stop.y())
            stop = m_sel_stop.x() - m_sel_start.x();
        else
            stop = lines()[i].length();

        text += lines()[i].mid(start, stop);
        line = lines()[i].mid(start, stop);

        if(i != (quint32)m_sel_stop.y())
            text += "\r\n";

        start = 0;
    }
    QString test = text.mid(text.lastIndexOf("\n"));

    QApplication::clipboard()->setText(text);
}

void Terminal::selectAll()
{
    m_sel_begin = m_sel_start = QPoint(0, 0);
    m_sel_stop.setY(lines().size()-1);
    m_sel_stop.setX(lines()[m_sel_stop.y()].length());

    viewport()->update();
}

void Terminal::updateScrollBars()
{
    QSize areaSize = viewport()->size();
    verticalScrollBar()->setPageStep(areaSize.height());
    horizontalScrollBar()->setPageStep(areaSize.width());

    bool scroll = (verticalScrollBar()->value() == verticalScrollBar()->maximum());

    int height = lines().size();
    int width = 0;

    for(std::size_t i = 0; i < lines().size(); ++i)
    {
        if(width < lines()[i].length())
            width = lines()[i].length();
    }

    verticalScrollBar()->setRange(0, height - areaSize.height()/CHAR_HEIGHT + 1);
    horizontalScrollBar()->setRange(0, width - areaSize.width()/CHAR_WIDTH + 1);

    if(scroll)
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());

    this->update();
}

void Terminal::paintEvent(QPaintEvent *)
{
    QPainter painter(viewport());

    int width = viewport()->width()/CHAR_WIDTH;
    int height = viewport()->height()/CHAR_HEIGHT;

    int startX = horizontalScrollBar()->value();
    int startY = verticalScrollBar()->value();

    int y = 0;
    int x = 0;

    QPoint& cursor = m_paused ? m_cursor_pause_pos : m_cursor_pos;

    // Draw cursor
    if(cursor.x() >= startX && (cursor.x() - startX) < width &&
       cursor.y() >= startY && (cursor.y() - startY) < height)
    {
        x = (cursor.x() - startX)*CHAR_WIDTH;
        y = (cursor.y() - startY)*CHAR_HEIGHT;
        m_cursor.moveTo(x, y);

        painter.setPen(QPen(Qt::green));
        if(hasFocus())
            painter.setBrush(QBrush(Qt::green, Qt::SolidPattern));

        painter.drawRect(m_cursor);

        painter.setPen(QPen(Qt::white));
        painter.setBrush(Qt::NoBrush);
    }

    // draw selection
    if(m_sel_start != m_sel_stop && !m_sel_stop.isNull())
    {
        quint32 textLine = m_sel_start.y();
        quint32 max = m_sel_stop.y() - m_sel_start.y();

        x = (m_sel_start.x() - startX)*CHAR_WIDTH;
        y = (m_sel_start.y() - startY)*CHAR_HEIGHT;

        int w = width - m_sel_start.x();
        if(m_sel_stop.y() == m_sel_start.y())
          w = m_sel_stop.x() - m_sel_start.x();
        w *= CHAR_WIDTH;

        painter.setPen(QPen(this->palette().color(QPalette::Highlight)));
        painter.setBrush(this->palette().color(QPalette::Highlight));

        for(quint32 i = 0; i <= max; ++i,++textLine)
        {
            if(textLine >= lines().size())
                continue;

            int len = (lines()[textLine].length() - startX)*CHAR_WIDTH;
            if(i == 0)
            {
                if(w > len)
                    w = (len - m_sel_start.x()*CHAR_WIDTH);
            }
            else if(i == max && m_sel_stop.x()*CHAR_WIDTH < len)
                w = m_sel_stop.x()*CHAR_WIDTH;
            else
                w = len;

            QRect rec(x, y, w, CHAR_HEIGHT);
            painter.drawRect(rec);

            x = 0;
            y += CHAR_HEIGHT;
        }

        painter.setPen(QPen(Qt::white));
        painter.setBrush(Qt::NoBrush);
    }

    // draw text
    std::size_t i = startY;
    int maxLines = i + height + 1;
    for(y = 0; (int)i < maxLines && i < lines().size(); ++i, y += CHAR_HEIGHT)
    {
        QString l = lines()[i];
        l.remove(0, startX);

        painter.drawText(0, y, viewport()->width(), CHAR_HEIGHT, Qt::AlignLeft, l);
    }
}

void Terminal::pause(bool pause)
{
    if(pause == m_paused)
        return;

    m_paused = pause;

    if(pause)
    {
        m_pause_lines = m_lines;
        m_cursor_pause_pos = m_cursor_pos;
    }

    updateScrollBars();
    viewport()->update();
}

void Terminal::clear()
{
    m_data.clear();
    m_lines.clear();
    m_pause_lines.clear();
    m_cursor_pos = m_cursor_pause_pos = QPoint(0, 0);
    m_hex_pos = 0;

    updateScrollBars();
    viewport()->update();
}

void Terminal::resizeEvent(QResizeEvent *)
{
    updateScrollBars();
}

void Terminal::mousePressEvent(QMouseEvent *event)
{
    m_sel_stop = m_sel_start = m_sel_begin = mouseToTextPos(event->pos());
    viewport()->update();
}

QPoint Terminal::mouseToTextPos(const QPoint& pos)
{
    QPoint res;
    int& x = res.rx();
    int& y = res.ry();

    int width = viewport()->width()/CHAR_WIDTH;
    int height = viewport()->height()/CHAR_HEIGHT;

    int startX = horizontalScrollBar()->value();
    int startY = verticalScrollBar()->value();

    y = startY + pos.y()/CHAR_HEIGHT;
    if(y > startY + height)
    {
        y = 0;
        return res;
    }

    x = startX + pos.x()/CHAR_WIDTH;
    if(x > startX + width)
    {
        x = 0;
        y = 0;
    }

    return res;
}

void Terminal::mouseMoveEvent(QMouseEvent *event)
{
    if(!(event->buttons() & Qt::LeftButton))
        return;

    QPoint thisPos = mouseToTextPos(event->pos());
    if(thisPos.y() < m_sel_begin.y() ||
      (thisPos.x() <= m_sel_begin.x() && thisPos.y() <= m_sel_begin.y()))
    {
        m_sel_start = thisPos;
        m_sel_stop = m_sel_begin;
    }
    else
    {
        m_sel_start = m_sel_begin;
        m_sel_stop = thisPos;
    }
    viewport()->update();
}

void Terminal::focusInEvent(QFocusEvent *)
{
    viewport()->update();
}

void Terminal::focusOutEvent(QFocusEvent *)
{
    viewport()->update();
}

void Terminal::setFmt(quint8 fmt)
{
    if(fmt == m_fmt)
        return;

    m_fmt = fmt;
    m_lines.clear();
    m_hex_pos = 0;
    m_cursor_pos = m_cursor_pause_pos = QPoint(0, 0);

    bool paused = m_paused;
    pause(false);

    switch(fmt)
    {
        case FMT_TEXT: addLines(m_data); break;
        case FMT_HEX:  addHex();         break;
    }

    updateScrollBars();
    pause(paused);
}

void Terminal::writeToFile(QFile *file)
{
    for(quint32 i = 0; i < lines().size(); ++i)
    {
        file->write(lines()[i].toAscii());
        file->write("\n");
    }
}
