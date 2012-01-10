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

#include "utils.h"

QString Utils::hexToString(quint8 data, bool withZeroEx)
{
    QString result = QString::number(data, 16).toUpper();
    if(result.count()%2)
        result.push_front('0');
    if(withZeroEx)
        result.prepend("0x");
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
