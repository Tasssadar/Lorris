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
#include <QMenu>
#include <QSignalMapper>
#include <QStringBuilder>

#include "common.h"
#include "terminal.h"

Terminal::Terminal(QWidget *parent) : QAbstractScrollArea(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QColor color_black(0, 0, 0);
    QColor color_white(255, 255, 255);
    QPalette palette;
    palette.setColor(QPalette::Base, color_black);
    palette.setColor(QPalette::Text, color_white);
    setPalette(palette);

    setFont(Utils::getMonospaceFont());
    viewport()->setCursor(Qt::IBeamCursor);

    m_char_width = fontMetrics().width(QLatin1Char('9'));
    m_char_height = fontMetrics().height();

    m_paused = false;
    m_fmt = FMT_MAX+1;
    m_input = INPUT_SEND_KEYPRESS;
    m_hex_pos = 0;

    m_cursor.setSize(QSize(m_char_width, m_char_height));
    updateScrollBars();

    m_context_menu = new QMenu(this);
    QAction *copy = m_context_menu->addAction(tr("Copy"));
    copy->setShortcut(QKeySequence("Ctrl+C"));

    QAction *paste = m_context_menu->addAction(tr("Paste"));
    paste->setShortcut(QKeySequence("Ctrl+V"));

    m_context_menu->addSeparator();

    QMenu *format = m_context_menu->addMenu(tr("Format"));
    QSignalMapper *fmtMap = new QSignalMapper(this);
    for(quint8 i = 0; i < FMT_MAX; ++i)
    {
        static const QString fmtText[] = { tr("Text"), tr("Hex dump") };

        m_fmt_act[i] = format->addAction(fmtText[i]);
        m_fmt_act[i]->setCheckable(true);
        fmtMap->setMapping(m_fmt_act[i], i);
        connect(m_fmt_act[i], SIGNAL(triggered()), fmtMap, SLOT(map()));
    }

    QAction *clear = m_context_menu->addAction(tr("Clear"));

    connect(copy,  SIGNAL(triggered()), SLOT(copyToClipboard()));
    connect(paste, SIGNAL(triggered()), SLOT(pasteFromClipboard()));
    connect(clear, SIGNAL(triggered()), SLOT(clear()));
    connect(fmtMap,SIGNAL(mapped(int)), SLOT(setFmt(int)));

    setFmt(FMT_TEXT);
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
    quint32 pos = m_cursor_pos.y();

    char *in_data = text.data();

    char *line_start = in_data;
    char *line_end = in_data;

    for(quint32 i = 0; *line_end && i < (quint32)text.size(); ++i)
    {
        switch(*line_end)
        {
            case '\f':
            {
                addLine(pos, line_start, line_end);
                m_cursor_pos.setX(0);
                m_cursor_pos.setY(0);
                pos = 0;
                break;
            }
            case '\r':
                addLine(pos, line_start, line_end);
                m_cursor_pos.setX(0);
                break;
            case '\n':
            {
                addLine(pos, line_start, line_end);
                ++pos;
                m_cursor_pos.setX(0);
                m_cursor_pos.setY(pos);
                break;
            }
            case '\b':
            {
                if(line_end != line_start)
                    --line_end;

                addLine(pos, line_start, line_end);

                if(*line_start == '\b')
                    ++line_end;
                line_start = line_end;

                if(m_cursor_pos.x() != 0)
                    --m_cursor_pos.rx();
                break;
            }
            default:
            {
                ++line_end;
                break;
            }
        }
    }

    if(line_start != line_end)
        addLine(pos, line_start, line_end);
}

void Terminal::addLine(quint32 pos, char *&line_start, char *&line_end)
{
    std::vector<QString>::iterator linePos = m_lines.begin() + pos;
    if(linePos != m_lines.end())
    {
        if((line_end - line_start + m_cursor_pos.x()) > (*linePos).length())
            (*linePos).resize(line_end - line_start + m_cursor_pos.x());

        QChar *lineData = (*linePos).data();

        QChar *line = new QChar[line_end - line_start];
        QChar *line_itr = line;
        for(char *itr = line_start; itr != line_end; ++itr,++line_itr)
            *line_itr = *itr;
        std::copy(line, line_itr, lineData+m_cursor_pos.x());

        m_cursor_pos.rx() += (line_end - line_start);
    }
    else
    {
        char tmp = *line_end;
        *line_end = 0;
        m_lines.insert(linePos, QString(line_start));
        *line_end = tmp;

        m_cursor_pos.setX(m_lines[pos].length());
    }

    m_cursor_pos.setY(pos);

    ++line_end;
    line_start = line_end;
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

void Terminal::keyPressEvent(QKeyEvent *event)
{
    QByteArray key = event->text().toAscii();

    if((event->modifiers() & Qt::ControlModifier))
    {
        switch(event->key())
        {
            case Qt::Key_C:
            case Qt::Key_X:
                copyToClipboard();
                return;
            case Qt::Key_V:
            {
               key = QApplication::clipboard()->text().toAscii();
               break;
            }
            case Qt::Key_A:
                selectAll();
                return;
        }
    }

    switch(event->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            key = "\r\n";
            break;
        }
        case Qt::Key_Backspace:
        {
            if(m_input != INPUT_SEND_COMMAND)
                break;
            appendText(key);
            m_command.chop(1);
            return;
        }
    }

    if(key.isEmpty())
        return;

    handleInput(key, event->key());
}

void Terminal::handleInput(const QByteArray &data, int key)
{
    switch(m_input)
    {
        case INPUT_SEND_KEYPRESS:
        {
            emit keyPressedASCII(data);
            break;
        }
        case INPUT_SEND_COMMAND:
        {
            m_command.append(data);
            appendText(data);

            if(key != Qt::Key_Return && key != Qt::Key_Enter)
                break;

            emit keyPressedASCII(m_command);
            m_command.clear();
        }
    }
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

    if(!text.isEmpty())
        QApplication::clipboard()->setText(text);
}

void Terminal::selectAll()
{
    m_sel_begin = m_sel_start = QPoint(0, 0);
    m_sel_stop.setY(lines().size()-1);
    m_sel_stop.setX(lines()[m_sel_stop.y()].length());

    viewport()->update();
}

void Terminal::pasteFromClipboard()
{
    handleInput(QApplication::clipboard()->text().toAscii());
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

    verticalScrollBar()->setRange(0, height - areaSize.height()/m_char_height + 1);
    horizontalScrollBar()->setRange(0, width - areaSize.width()/m_char_width + 1);

    if(scroll)
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());

    this->update();
}

void Terminal::paintEvent(QPaintEvent *)
{
    QPainter painter(viewport());

    int width = viewport()->width()/m_char_width;
    int height = viewport()->height()/m_char_height;

    int startX = horizontalScrollBar()->value();
    int startY = verticalScrollBar()->value();

    int y = 0;
    int x = 0;

    QPoint& cursor = m_paused ? m_cursor_pause_pos : m_cursor_pos;

    // Draw cursor
    if(cursor.x() >= startX && (cursor.x() - startX) < width &&
       cursor.y() >= startY && (cursor.y() - startY) < height)
    {
        x = (cursor.x() - startX)*m_char_width;
        y = (cursor.y() - startY)*m_char_height;
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

        x = (m_sel_start.x() - startX)*m_char_width;
        y = (m_sel_start.y() - startY)*m_char_height;

        int w = m_char_width;

        if(m_sel_stop.y() == m_sel_start.y())
            w *= m_sel_stop.x() - m_sel_start.x();
        else
            w *= width - m_sel_start.x();

        painter.setPen(Qt::NoPen);
        painter.setBrush(this->palette().color(QPalette::Highlight));

        for(quint32 i = 0; i <= max; ++i,++textLine)
        {
            if(textLine >= lines().size())
                continue;

            int len = (lines()[textLine].length() - startX)*m_char_width;
            adjustSelectionWidth(w, i, max, len);

            QRect rec(x, y, w, m_char_height);
            painter.drawRect(rec);

            x = 0;
            y += m_char_height;
        }

        painter.setPen(QPen(Qt::white));
        painter.setBrush(Qt::NoBrush);
    }

    // draw text
    std::size_t i = startY;
    int maxLines = i + height + 1;
    for(y = 0; (int)i < maxLines && i < lines().size(); ++i, y += m_char_height)
    {
        QString& l = lines()[i];
        int len = l.length() - startX;
        if(len <= 0)
            continue;
        painter.drawText(0, y, viewport()->width(), m_char_height, Qt::AlignLeft,
                         QString::fromRawData(l.data()+startX, len));
    }
}

void Terminal::adjustSelectionWidth(int &w, quint32 i, quint32 max, int len)
{
    if(i == 0 && m_sel_start.x()*m_char_width > len)
        w = 0;
    else if(i == 0)   w = std::min(w, (len - m_sel_start.x()*m_char_width));
    else if(i == max) w = std::min(len, m_sel_stop.x()*m_char_width);
    else              w = len;
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
    switch(event->button())
    {
        case Qt::LeftButton:
            m_sel_stop = m_sel_start = m_sel_begin = mouseToTextPos(event->pos());
            viewport()->update();
            break;
        case Qt::RightButton:
            m_context_menu->exec(event->globalPos());
            break;
        default:
            break;
    }
}

QPoint Terminal::mouseToTextPos(const QPoint& pos)
{
    QPoint res;
    int& x = res.rx();
    int& y = res.ry();

    int width = viewport()->width()/m_char_width;
    int height = viewport()->height()/m_char_height;

    int startX = horizontalScrollBar()->value();
    int startY = verticalScrollBar()->value();

    y = startY + pos.y()/m_char_height;
    if(y > startY + height)
    {
        y = 0;
        return res;
    }

    x = startX + pos.x()/m_char_width;
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

void Terminal::setFmt(int fmt)
{
    if(fmt == m_fmt)
        return;

    for(quint8 i = 0; i < FMT_MAX; ++i)
        m_fmt_act[i]->setChecked(i == fmt);

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

void Terminal::setInput(quint8 input)
{
    if(input == m_input)
        return;

    m_input = input;
    m_command.clear();
}
