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
    m_data_size = 0;
    m_data_alloc = 512;
    m_data = (char*)malloc(m_data_alloc);

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
    m_changed = true;
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
    connect(&m_updateTimer, SIGNAL(timeout()), SLOT(updateScrollBars()));

    setFmt(FMT_TEXT);

    m_updateTimer.start(100);
}

Terminal::~Terminal()
{
    free(m_data);
}

void Terminal::appendText(const QByteArray& text)
{
    m_data_size += text.size();
    if(m_data_size > m_data_alloc)
    {
        while(m_data_alloc < m_data_size)
            m_data_alloc += 512;

        m_data = (char*)realloc(m_data, m_data_alloc);
    }

    std::copy(text.data(), text.data()+text.size(), m_data+m_data_size-text.size());

    switch(m_fmt)
    {
        case FMT_TEXT: addLines(QString::fromUtf8(text)); break;
        case FMT_HEX:  addHex();       break;
    }

    if(!m_paused)
        m_changed = true;
}

void Terminal::addLines(const QString &text)
{
    quint32 pos = m_cursor_pos.y();
    QChar *line_start = (QChar*)text.data();
    QChar *line_end = line_start;

    for(int i = 0; i < text.size() && *line_end != 0; ++i)
    {
        switch((*line_end).unicode())
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
                {
                    --line_end;

                    addLine(pos, line_start, line_end);

                    if(*line_start == '\b')
                        ++line_end;
                    line_start = line_end;
                }
                else
                {
                    std::vector<QString>::iterator linePos = m_lines.begin() + pos;
                    if(linePos == m_lines.end())
                        break;

                    if(m_cursor_pos.x() != 0)
                        --m_cursor_pos.rx();

                    ++line_end;
                    ++line_start;

                    if((*linePos).size() == 1)
                        m_lines.erase(linePos);
                    else
                        (*linePos).chop(1);
                }

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

void Terminal::addLine(quint32 pos, QChar *&line_start, QChar *&line_end)
{
    std::vector<QString>::iterator linePos = m_lines.begin() + pos;
    if(linePos != m_lines.end())
    {
        if((line_end - line_start + m_cursor_pos.x()) > (*linePos).length())
            (*linePos).resize(line_end - line_start + m_cursor_pos.x());

        QChar *lineData = (*linePos).data()+m_cursor_pos.x();
        std::copy(line_start, line_end, lineData);

        m_cursor_pos.rx() += (line_end - line_start);
    }
    else
    {
        QChar tmp = *line_end;
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
    if(m_hex_pos%16 != 0)
    {
        m_hex_pos -= m_hex_pos%16;
        m_lines.pop_back();
    }

    char *chunk = NULL;
    char *end = m_data+m_data_size;

    int chunk_size;
    char *itr;
    char *line = new char[78];

    line[8] = line[57] = line[58] = line[59] = ' ';
    line[60] = line[77] = '|';

    for(quint32 i = m_hex_pos; i < m_data_size; i += 16)
    {
        itr = line;
        chunk = m_data+m_hex_pos;
        chunk_size = std::min(int(end - chunk), 16);

        static const char* hex = "0123456789ABCDEF";
        for(int x = 7; x >= 0; --x, ++itr)
            *itr = hex[(m_hex_pos >> x*4) & 0x0F];
        ++itr;

        m_hex_pos += chunk_size;

        for(int x = 0; x < chunk_size; ++x)
        {
            *(itr++) = hex[quint8(chunk[x]) >> 4];
            *(itr++) = hex[quint8(chunk[x]) & 0x0F];
            *(itr++) = ' ';

            line[61+x] = (chunk[x] < 32 || chunk[x] > 126) ? '.' : chunk[x];
        }

        memset(itr, ' ', (16 - chunk_size)*3);

        if(chunk_size != 16)
            *(line + chunk_size + 61) = '|';

        m_lines.push_back(QString::fromAscii(line, 62+chunk_size));
    }
    delete[] line;
    m_cursor_pos.setY(m_lines.size());
    m_cursor_pos.setX(0);
}

void Terminal::keyPressEvent(QKeyEvent *event)
{
    QString key = event->text();

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
        QAbstractScrollArea::keyPressEvent(event);
    else
        handleInput(key, event->key());
}

void Terminal::handleInput(const QString &data, int key)
{
    switch(m_input)
    {
        case INPUT_SEND_KEYPRESS:
        {
            emit keyPressed(data);
            break;
        }
        case INPUT_SEND_COMMAND:
        {
            m_command.append(data);
            appendText(data);

            if(key != Qt::Key_Return && key != Qt::Key_Enter)
                break;

            emit keyPressed(m_command);
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
    if(!m_changed)
        return;

    m_changed = false;
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

    update();
    viewport()->update();
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
    int maxLen = viewport()->width()/m_char_width + 1;
    for(y = 0; (int)i < maxLines && i < lines().size(); ++i, y += m_char_height)
    {
        QString& l = lines()[i];

        int len = std::min(l.length() - startX, maxLen);
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
    else
        m_pause_lines.clear();

    m_changed = true;
    updateScrollBars();
}

void Terminal::clear()
{
    m_data_alloc = 512;
    m_data = (char*)realloc(m_data, m_data_alloc);
    m_data_size = 0;

    m_lines.clear();
    m_pause_lines.clear();
    m_cursor_pos = m_cursor_pause_pos = QPoint(0, 0);
    m_hex_pos = 0;

    m_changed = true;
    updateScrollBars();
}

void Terminal::resizeEvent(QResizeEvent *)
{
    m_changed = true;
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
        case FMT_TEXT: addLines(QString::fromUtf8(m_data, m_data_size)); break;
        case FMT_HEX:  addHex();         break;
    }

    m_changed = true;
    updateScrollBars();
    pause(paused);
}

void Terminal::writeToFile(QFile *file)
{
    for(quint32 i = 0; i < lines().size(); ++i)
    {
        file->write(lines()[i].toUtf8());
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
