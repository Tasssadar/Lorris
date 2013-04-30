/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

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
