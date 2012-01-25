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

#include <algorithm>

#include "shupito.h"
#include "shupitopdi.h"

ShupitoPDI::ShupitoPDI(Shupito *shupito) : ShupitoMode(shupito)
{
}

ShupitoDesc::config *ShupitoPDI::getModeCfg()
{
    return m_shupito->getDesc()->getConfig("71efb903-3030-4fd3-8896-1946aba37efc");
}


void ShupitoPDI::editIdArgs(QString &id, quint8 &id_lenght)
{
    id = "avr:";
    id_lenght = std::min(id_lenght, (quint8)3);
}

