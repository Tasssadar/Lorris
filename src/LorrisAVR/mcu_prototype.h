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

#ifndef MCU_PROTOTYPE_H
#define MCU_PROTOTYPE_H

#include <QTypeInfo>

enum sreg_flags
{
    SREG_IRQ      = (1 << 7),
    SREG_TBITCOPY = (1 << 6),
    SREG_HALFCARRY= (1 << 5),
    SREG_SIGN     = (1 << 4),
    SREG_V        = (1 << 3),
    SREG_NEG      = (1 << 2),
    SREG_ZERO     = (1 << 1),
    SREG_CARRY    = (1 << 9)
};

struct mcu_prototype
{
    bool bit16;
    quint32 sram_size;
    quint32 prog_mem_size;
    quint32 eeprom_size;

    quint8 vector_words; // currently unused

    quint8 SPL;
    //quint8 SPH; -- it is expected to be SPL+1

    // USART 0
    quint8 UDR0;
    quint8 UBRR0H;
    quint8 UBRR0L;
    quint8 UCSR0C;
    quint8 UCSR0B;
    quint8 UCSR0A;
};

static mcu_prototype atmega328p = {true,  2048, 32768, 1024, 2, 0x5D, 0xC6, 0xC5, 0xC4, 0xC2, 0xC1, 0xC0 };

#endif // MCU_PROTOTYPE_H
