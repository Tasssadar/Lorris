/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "shupitopacket.h"
#include <cstdarg>

ShupitoPacket makeShupitoPacket(quint8 cmd, quint8 size, ...)
{
    ShupitoPacket res;
    res.reserve(1 + size);
    res.push_back(cmd);

    va_list args;
    va_start(args, size);
    for (quint8 i = 0; i < size; ++i)
        res.push_back((quint8)va_arg(args, int));
    va_end(args);

    return res;
}
