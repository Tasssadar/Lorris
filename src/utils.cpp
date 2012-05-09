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

#include <QMessageBox>
#include <QStatusBar>

#include "utils.h"

QStatusBar *Utils::m_status_bar = NULL;

QString Utils::hexToString(quint8 data, bool withZeroEx)
{
    static const char* hex = "0123456789ABCDEF";

    QString result(withZeroEx ? "0x" : "");
    result += hex[data >> 4];
    result += hex[data & 0x0F];
    return result;
}

QString Utils::parseChar(char c)
{
    switch(c)
    {
        case '\n': return "\\n";
        case '\r': return "\\r";
        case '\f': return "\\f";
        default:   return QString((QChar)c);
    }
}

QString Utils::toBase16(quint8 const * first, quint8 const * last)
{
    QString res;
    for (; first != last; ++first)
    {
        static char const digits[] = "0123456789abcdef";
        res.append(digits[(*first >> 4)]);
        res.append(digits[(*first & 0xF)]);
    }
    return res;
}

QString Utils::toBinary(std::size_t width, int value)
{
    QString res("0b");

    for(; width != 0; --width)
    {
        res[(int)(width+1)] = (QChar)((value % 2) ? '1' : '0');
        value >>= 1;
    }
    return res;
}

QFont Utils::getMonospaceFont(quint8 size)
{
    return QFont("Courier New", size);
}

void Utils::ThrowException(const QString& text, QWidget* parent)
{
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Critical);
    box.setWindowTitle(tr("Error!"));
    box.setText(text);
    box.exec();
}

void Utils::setStatusBar(QStatusBar *bar)
{
    m_status_bar = bar;
}

void Utils::printToStatusBar(const QString& msg, int timeout)
{
    if(m_status_bar)
        m_status_bar->showMessage(msg, timeout);
}
