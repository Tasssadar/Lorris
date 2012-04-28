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

#ifndef SHUPITOPACKET_H
#define SHUPITOPACKET_H

#include <QtGlobal>
#include <vector>

/*
  Structure was removed from Shupito packets, as USB devices
  will have a different packet format. However, certain restrictions remain.
  The packet must always be at least one character long (the command).
  For UART-based Shupito devices limit the length of the packet to 16-bytes
  (including the command byte) and the value of the first byte
  must be no greater than 15.
  */
typedef std::vector<quint8> ShupitoPacket;
ShupitoPacket makeShupitoPacket(quint8 cmd, quint8 size, ...);

#endif // SHUPITOPACKET_H
