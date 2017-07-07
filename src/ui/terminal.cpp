/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

/*
 * This terminal supports UTF8 encoding, it can output data
 * as text or hex dump and pause output. It also allows font selection,
 * but if user selects non-monospace font, cursor position and maybe something
 * else will get ugly. I do not want to add support for non-monospace fonts
 * because it is completely useless - it is expected that user will want to
 * change only font size.
 *
 * This QWidget should be usable outside Lorris without problems.
 */

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
#include <QFontDialog>

#include "../common.h"
#include "terminal.h"
#include "terminalsettings.h"
#include "termina-colors.h"

Terminal::Terminal(QWidget *parent) : QAbstractScrollArea(parent)
{
    m_data.reserve(512);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::black);
    p.setColor(QPalette::Text, Qt::white);
    setPalette(p);

    m_paused = false;
    m_fmt = FMT_MAX+1;
    m_input = INPUT_SEND_KEYPRESS;
    m_hex_pos = 0;
    m_last_esc = NULL;

    viewport()->setCursor(Qt::IBeamCursor);
    setFont(Utils::getMonospaceFont());

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

    QAction *settings = m_context_menu->addAction(tr("Terminal settings..."));
    m_pauseAct = m_context_menu->addAction(tr("Pause"));
    QAction *clear = m_context_menu->addAction(tr("Clear"));

    m_pauseAct->setCheckable(true);

    connect(copy,  SIGNAL(triggered()), SLOT(copyToClipboard()));
    connect(paste, SIGNAL(triggered()), SLOT(pasteFromClipboard()));
    connect(clear, SIGNAL(triggered()), SLOT(clear()));
    connect(m_pauseAct, SIGNAL(toggled(bool)), SLOT(pause(bool)));
    connect(settings,   SIGNAL(triggered()),   SLOT(showSettings()));
    connect(fmtMap,SIGNAL(mapped(int)), SLOT(setFmt(int)));
    connect(&m_updateTimer, SIGNAL(timeout()), SLOT(updateScrollBars()));

    setFmt(FMT_TEXT);

    m_updateTimer.start(100);
    viewport()->setAutoFillBackground(true);

    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled);
}

Terminal::~Terminal()
{
}

void Terminal::appendText(const QByteArray& text)
{
    if(m_data.size()+text.size() > m_data.capacity())
        while(m_data.capacity() < m_data.size()+text.size())
            m_data.reserve(m_data.capacity()+512);

    m_data.insert(m_data.end(), text.data(), text.data()+text.size());

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

    for(int i = 0; i < text.size(); ++i)
    {
        const auto c = (*line_end).unicode();

        if(!m_esc_seq.isEmpty()) {
            if(m_esc_seq.length() == 1 && c != '[') {
                m_esc_seq.clear();
                line_start = line_end;
                ++line_end;
            } else if((*line_end).isLetter()) {
                m_esc_seq.append(c);
                handleEscSeq();
                m_esc_seq.clear();
                ++line_end;
                line_start = line_end;
            } else {
                m_esc_seq.append(c);
                ++line_end;
            }
            continue;
        }

        switch(c)
        {
            case '\f':
            {
                addLine(pos, line_start, line_end);

                if(!m_settings.chars[SET_FORMFEED])
                    break;

                m_cursor_pos.setX(0);
                m_cursor_pos.setY(0);
                pos = 0;
                break;
            }
            case '\r':
            {
                addLine(pos, line_start, line_end);
                newlineChar(m_settings.chars[SET_RETURN], pos);
                break;
            }
            case '\n':
            {
                addLine(pos, line_start, line_end);
                newlineChar(m_settings.chars[SET_NEWLINE], pos);
                break;
            }
            case '\b':
            {
                if(!m_settings.chars[SET_BACKSPACE])
                {
                    addLine(pos, line_start, line_end);
                    break;
                }

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
            case '\a':
                if(m_settings.chars[SET_ALARM])
                    Utils::playErrorSound();
                break;
            case '\t':
            {
                if(m_settings.chars[SET_REPLACE_TAB])
                {
                    addLine(pos, line_start, line_end);
                    std::vector<QString>::iterator linePos = m_lines.begin() + pos;
                    if(linePos != m_lines.end())
                    {
                        (*linePos).append(QByteArray(m_settings.tabReplace, ' '));
                         m_cursor_pos.rx() += m_settings.tabReplace;
                    }
                }
                else
                    ++line_end;
                break;
            }
            case 0:
            {
                if(!m_settings.chars[SET_IGNORE_NULL])
                {
                    i = text.size()+1;
                    break;
                }
                else
                {
                    addLine(pos, line_start, line_end);

                    std::vector<QString>::iterator linePos = m_lines.begin() + pos;
                    if(linePos != m_lines.end())
                        (*linePos).append('.');
                    else
                        m_lines.push_back(QString("."));
                    break;
                }
            }
            case '\e':
            {
                if(m_settings.chars[SET_HANDLE_ESCAPE]) {
                    addLine(pos, line_start, line_end);
                    m_esc_seq.append(c);
                } else {
                    ++line_end;
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

        if(m_last_esc && !m_last_esc->isEmpty())
            m_escapes[pos][0] = *m_last_esc;
    }

    m_cursor_pos.setY(pos);

    ++line_end;
    line_start = line_end;
}

void Terminal::newlineChar(quint8 option, quint32& pos)
{
    switch(option)
    {
        case NL_NEWLINE_RETURN:
            ++pos;
            m_cursor_pos.setY(pos);
            m_cursor_pos.setX(0);
            break;
        case NL_NEWLINE:
        {
            ++pos;
            m_cursor_pos.setY(pos);

            if(m_cursor_pos.x() == 0)
                break;

            std::vector<QString>::iterator linePos = m_lines.begin() + pos;
            if(linePos != m_lines.end() && m_cursor_pos.x() > (*linePos).size())
                (*linePos).append(QByteArray(m_cursor_pos.x() - (*linePos).size(), ' '));
            else
                m_lines.push_back(QString(m_cursor_pos.x(), ' '));
            break;
        }
        case NL_RETURN:
            m_cursor_pos.setX(0);
            break;
        case NL_NOTHING:
            break;
    }
}

void Terminal::addHex()
{
    if(m_hex_pos%16 != 0)
    {
        m_hex_pos -= m_hex_pos%16;
        m_lines.pop_back();
    }

    std::vector<char>::iterator chunk;

    int chunk_size;
    char *itr;
    char line[78];

    line[8] = line[57] = line[58] = line[59] = ' ';
    line[60] = line[77] = '|';

    for(quint32 i = m_hex_pos; i < m_data.size(); i += 16)
    {
        itr = line;
        chunk = m_data.begin()+m_hex_pos;
        chunk_size = std::min(int(m_data.end() - chunk), 16);

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

        m_lines.push_back(QString::fromLatin1(line, 62+chunk_size));
    }
    m_cursor_pos.setY(m_lines.size());
    m_cursor_pos.setX(0);
}

void Terminal::inputMethodEvent(QInputMethodEvent *e) {
    handleInput(e->commitString(), 0);
}

void Terminal::keyPressEvent(QKeyEvent *event)
{
    QString key = event->text();

    if((event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::ShiftModifier))
    {
        switch(event->key())
        {
            case Qt::Key_C:
            case Qt::Key_X:
                copyToClipboard();
                return;
            case Qt::Key_V:
            {
               key = QApplication::clipboard()->text();
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
            key = getCurrNewlineStr(event->modifiers());
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
        case Qt::Key_Tab:
        {
            if(event->modifiers() & Qt::ControlModifier)
                return QAbstractScrollArea::keyPressEvent(event);
            break;
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

bool Terminal::event(QEvent *event)
{
    if(event->type() != QEvent::ShortcutOverride)
        return QAbstractScrollArea::event(event);

    QKeyEvent *ke = (QKeyEvent*)event;
    if(ke->modifiers() == Qt::NoModifier)
    {
        ke->accept();
        return true;
    }
    return QAbstractScrollArea::event(event);
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
    handleInput(QApplication::clipboard()->text());
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

    const int width = viewport()->width()/m_char_width;
    const int height = viewport()->height()/m_char_height;

    const int startX = horizontalScrollBar()->value();
    const int startY = verticalScrollBar()->value();

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

        painter.setPen(QPen(m_settings.colors[COLOR_CURSOR]));
        if(hasFocus())
            painter.setBrush(QBrush(m_settings.colors[COLOR_CURSOR], Qt::SolidPattern));

        painter.drawRect(m_cursor);
        painter.setBrush(Qt::NoBrush);
    }

    // draw selection
    const bool drawSelection = m_sel_start != m_sel_stop && !m_sel_stop.isNull();
    if(drawSelection)
    {
        quint32 textLine = m_sel_start.y();
        quint32 max = m_sel_stop.y() - m_sel_start.y();

        x = (m_sel_start.x() - startX)*m_char_width;
        y = (m_sel_start.y() - startY)*m_char_height;

        int w = m_char_width;

        if(m_sel_stop.y() == m_sel_start.y())
            w *= m_sel_stop.x() - m_sel_start.x();
        else
            w *= width - (m_sel_start.x() - startX);

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
        painter.setBrush(Qt::NoBrush);
    }

    // draw text
    std::size_t i = startY;
    int maxLines = i + height + 1;
    int maxLen = viewport()->width()/m_char_width + 1;

    painter.setPen(QPen(m_settings.colors[COLOR_TEXT]));

    bool colorChanged = false;
    bool fontChanged = false;

    for(y = 0; (int)i < maxLines && i < lines().size(); ++i, y += m_char_height)
    {
        QString& l = lines()[i];

        int len = std::min(l.length() - startX, maxLen);
        if(len <= 0)
            continue;

        const auto ei = m_escapes.find(i);
        if(ei != m_escapes.end()) {
            const auto blks = ei->second;
            int curX = startX;
            for(auto itr = blks.begin(); itr != blks.end();) {
                const auto cur = itr++;
                if(itr != blks.end() && itr->first < curX)
                    continue;

                int end = std::min(l.length(), maxLen);
                if(itr != blks.end() && itr->first < end)
                    end = itr->first;

                if(cur->second.color.isValid()) {
                    painter.setPen(cur->second.color);
                    colorChanged = true;
                } else if(colorChanged) {
                    painter.setPen(m_settings.colors[COLOR_TEXT]);
                    colorChanged = false;
                }

                if(!drawSelection && cur->second.background.isValid()) {
                    painter.save();

                    painter.setBrush(QBrush(cur->second.background));
                    painter.setPen(Qt::NoPen);

                    QRectF bgrect((curX - startX)*m_char_width, y, (end - curX)*m_char_width, m_char_height);
                    painter.drawRect(bgrect);

                    painter.restore();
                }

                if(cur->second.flags & ESC_BOLD) {
                    fontChanged = true;
                    QFont f = painter.font();
                    f.setBold(true);
                    painter.setFont(f);
                } else if(fontChanged) {
                    QFont f = painter.font();
                    f.setBold(false);
                    painter.setFont(f);
                }

                painter.drawText((curX - startX)*m_char_width, y, viewport()->width(), m_char_height, 0,
                             QString::fromRawData(l.data()+curX, end - curX));

                curX = end;
            }
        } else {
            painter.drawText(0, y, viewport()->width(), m_char_height, 0,
                         QString::fromRawData(l.data()+startX, len));
        }
    }
}

void Terminal::adjustSelectionWidth(int &w, quint32 i, quint32 max, int len)
{
    int startX = horizontalScrollBar()->value();
    if(i == 0 && (m_sel_start.x() - startX)*m_char_width > len)
        w = 0;
    else if(i == 0)   w = std::min(w, (len - (m_sel_start.x() - startX)*m_char_width));
    else if(i == max) w = std::min(len, (m_sel_stop.x() - startX)*m_char_width);
    else              w = len;
}

void Terminal::pause(bool pause)
{
    if(pause == m_paused)
        return;

    m_paused = pause;
    m_pauseAct->setChecked(pause);

    if(pause)
    {
        m_pause_lines = m_lines;
        m_cursor_pause_pos = m_cursor_pos;
    }
    else
        m_pause_lines.clear();

    m_changed = true;
    updateScrollBars();

    emit paused(pause);
}

void Terminal::clear()
{
    m_data.clear();
    m_data.reserve(512);

    m_last_esc = NULL;
    m_esc_seq.clear();
    m_escapes.clear();
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

    x = startX + pos.x()/m_char_width;
    if(x > startX + width)
        x = startX + width;
    else if(x < 0)
        x = 0;

    y = startY + pos.y()/m_char_height;
    if(y > startY + height) {
        y = startY + height;
    } else if(y < 0) {
        y = 0;
        x = 0;
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

void Terminal::focusInEvent(QFocusEvent *event)
{
    viewport()->update();
    QAbstractScrollArea::focusInEvent(event);
}

void Terminal::focusOutEvent(QFocusEvent *event)
{
    viewport()->update();
    QAbstractScrollArea::focusOutEvent(event);
}

void Terminal::setFmt(int fmt)
{
    if(fmt == m_fmt)
        return;

    for(quint8 i = 0; i < FMT_MAX; ++i)
        m_fmt_act[i]->setChecked(i == fmt);

    m_fmt = fmt;
    redrawAll();

    emit fmtSelected(fmt);
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

QString Terminal::getSettingsData()
{
    QString res;
    for(int i = 0; i < SET_MAX; ++i)
        res += QString("%1;").arg(m_settings.chars[i]);

    res += QString("|%1|").arg(m_settings.tabReplace);
    res += Utils::getFontSaveString(font()) + "|";

    for(int i = 0; i < COLOR_MAX; ++i)
        res += QString("%1;").arg(m_settings.colors[i].name());

    res += "|" + QString::number(m_fmt);
    res += "|" + QString::number(m_input);
    return res;
}

void Terminal::loadSettings(const QString& data)
{
    QStringList lst = data.split('|', QString::SkipEmptyParts);
    if(lst.size() < 3)
        return;

    QStringList chars = lst[0].split(';', QString::SkipEmptyParts);
    QStringList addVals = lst[1].split(';', QString::SkipEmptyParts);
    for(int i = 0; i < chars.size() && i < SET_MAX; ++i)
        m_settings.chars[i] = chars[i].toUInt();

    if(addVals.size() > 0)
        m_settings.tabReplace = addVals[0].toUInt();

    setFont(Utils::getFontFromString(lst[2]));

    if(lst.size() >= 4)
    {
        QStringList colors = lst[3].split(';', QString::SkipEmptyParts);
        for(int i = 0; i < colors.size() && i < COLOR_MAX; ++i)
            m_settings.colors[i] = QColor(colors[i]);

        QPalette p = palette();
        p.setColor(QPalette::Base, m_settings.colors[COLOR_BG]);
        p.setColor(QPalette::Text, m_settings.colors[COLOR_TEXT]);
        setPalette(p);
    }

    if(lst.size() >= 5)
        setFmt(lst[4].toUInt());

    if(lst.size() >= 6)
        setInput(lst[5].toUInt());
}

void Terminal::setFont(const QFont &f)
{
    QAbstractScrollArea::setFont(f);

    m_char_width = fontMetrics().width(QLatin1Char('m'));
    m_char_height = fontMetrics().height();

    m_cursor.setSize(QSize(m_char_width, m_char_height));

    m_changed = true;
    m_settings.font = f;
    updateScrollBars();
}

void Terminal::showSettings()
{
    TerminalSettings s(m_settings, this);
    connect(&s, SIGNAL(applySettings(terminal_settings)), SLOT(applySettings(terminal_settings)));

    if(s.exec() == QDialog::Accepted)
        applySettings(s.getSettings());
}

void Terminal::applySettings(const terminal_settings& set)
{
    m_settings.copy(set);
    setFont(set.font);

    QPalette p = palette();
    p.setColor(QPalette::Base, m_settings.colors[COLOR_BG]);
    p.setColor(QPalette::Text, m_settings.colors[COLOR_TEXT]);
    setPalette(p);

    redrawAll();

    emit settingsChanged();
}

void Terminal::redrawAll()
{
    m_lines.clear();
    m_escapes.clear();
    m_last_esc = NULL;
    m_esc_seq.clear();
    m_hex_pos = 0;
    m_cursor_pos = m_cursor_pause_pos = QPoint(0, 0);

    bool paused = m_paused;
    pause(false);

    switch(m_fmt)
    {
        case FMT_TEXT: addLines(QString::fromUtf8(m_data.data(), m_data.size())); break;
        case FMT_HEX:  addHex();         break;
    }

    m_changed = true;
    updateScrollBars();
    pause(paused);
}

QString Terminal::getCurrNewlineStr(Qt::KeyboardModifiers modifiers)
{
    static const QString nl[] = {
        "\r\n",   // NLS_RN
        "\n",     // NLS_N
        "\r",     // NLS_R
        "\n\r",   // NLS_NR
    };

    static const int ctrl_mapping[NLS_MAX] = {
        NLS_NR,
        NLS_R,
        NLS_N,
        NLS_RN,
    };

    int res_idx = 0;
    if(m_settings.chars[SET_ENTER_SEND] < NLS_MAX)
        res_idx = m_settings.chars[SET_ENTER_SEND];

    if(modifiers & Qt::ControlModifier)
        res_idx = ctrl_mapping[res_idx];
    return nl[res_idx];
}

void Terminal::blink(const QColor &color)
{
    if(!color.isValid())
        return;

    QTimer::singleShot(100, this, SLOT(endBlink()));

    QPalette p = palette();
    p.setColor(QPalette::Base, color);
    setPalette(p);

    update();
}

void Terminal::endBlink()
{
    QPalette p = palette();
    p.setColor(QPalette::Base, m_settings.colors[COLOR_BG]);
    setPalette(p);

    update();
}

void Terminal::handleEscSeq()
{
    EscBlock blk;
    if(m_last_esc)
        blk = *m_last_esc;

    if(m_esc_seq[m_esc_seq.size()-1] != 'm')
        return;

    QString cnt = m_esc_seq.mid(2, m_esc_seq.size()-3);
    QStringList parts = cnt.split(';');
    bool ok = false;
    for(int i = 0; i < parts.length(); ++i) {
        int code = parts[i].toInt(&ok);
        if(!ok)
            continue;

        switch(code) {
        case 0:
            blk = EscBlock();
            break;
        case 1:
            blk.flags = EscFlags(blk.flags | ESC_BOLD);
            break;
        case 21:
            blk.flags = EscFlags(blk.flags & ~ESC_BOLD);
            break;
        case 39:
            blk.color = QColor();
            break;
        case 49:
            blk.background = QColor();
            break;

        case 30: blk.color = Qt::black; break;
        case 31: blk.color = Qt::darkRed; break;
        case 32: blk.color = Qt::darkGreen; break;
        case 33: blk.color = Qt::darkYellow; break;
        case 34: blk.color = Qt::darkBlue; break;
        case 35: blk.color = Qt::darkMagenta; break;
        case 36: blk.color = Qt::darkCyan; break;
        case 37: blk.color = Qt::lightGray; break;
        case 90: blk.color = Qt::darkGray; break;
        case 91: blk.color = Qt::red; break;
        case 92: blk.color = Qt::green; break;
        case 93: blk.color = Qt::yellow; break;
        case 94: blk.color = Qt::blue; break;
        case 95: blk.color = Qt::magenta; break;
        case 96: blk.color = Qt::cyan; break;
        case 97: blk.color = Qt::white; break;

        case 40:  blk.background = Qt::black; break;
        case 41:  blk.background = Qt::darkRed; break;
        case 42:  blk.background = Qt::darkGreen; break;
        case 43:  blk.background = Qt::darkYellow; break;
        case 44:  blk.background = Qt::darkBlue; break;
        case 45:  blk.background = Qt::darkMagenta; break;
        case 46:  blk.background = Qt::darkCyan; break;
        case 47:  blk.background = Qt::lightGray; break;
        case 100: blk.background = Qt::darkGray; break;
        case 101: blk.background = Qt::red; break;
        case 102: blk.background = Qt::green; break;
        case 103: blk.background = Qt::yellow; break;
        case 104: blk.background = Qt::blue; break;
        case 105: blk.background = Qt::magenta; break;
        case 106: blk.background = Qt::cyan; break;
        case 107: blk.background = Qt::white; break;

        case 38:
        case 48: {
            if(i+2 >= parts.length())
                break;

            i += 2;
            if(parts[i-1] != "5")
                break;

            uint clridx = parts[i].toUInt(&ok);
            if(!ok || clridx >= sizeof_array(term256colors))
                break;
            if(code == 38)
                blk.color = term256colors[clridx];
            else
                blk.background = term256colors[clridx];
            break;
        }
        }
    }

    if(!blk.isEmpty() || m_last_esc) {
        m_escapes[m_cursor_pos.y()][m_cursor_pos.x()] = blk;
        if(!blk.isEmpty())
            m_last_esc = &m_escapes[m_cursor_pos.y()][m_cursor_pos.x()];
        else
            m_last_esc = NULL;
    }
}
