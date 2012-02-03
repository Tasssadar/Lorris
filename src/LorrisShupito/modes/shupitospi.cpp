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

#include "../shupito.h"
#include "shupitospi.h"

ShupitoSPI::ShupitoSPI(Shupito *shupito) : ShupitoMode(shupito)
{
}

ShupitoDesc::config *ShupitoSPI::getModeCfg()
{
    return m_shupito->getDesc()->getConfig("46dbc865-b4d0-466b-9b70-2f3f5b264e65");
}


void ShupitoSPI::editIdArgs(QString &id, quint8 &/*id_lenght*/)
{
    id = "avr:";
}
