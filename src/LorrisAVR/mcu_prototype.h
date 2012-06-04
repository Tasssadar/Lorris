/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

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
    SREG_CARRY    = (1 << 0)
};

struct mcu_prototype
{
    mcu_prototype(QString name, bool bit16, quint32 sram_size, quint32 prog_mem_size, quint32 eeprom_size,
                  quint8 vector_words, quint8 SPL, quint8 UDR0, quint8 UBRR0H, quint8 UBRR0L, quint8 UCSR0C,
                  quint8 UCSR0B, quint8 UCSR0A)
    {
        this->name = name;

        this->bit16 = bit16;
        this->sram_size = sram_size;
        this->prog_mem_size = prog_mem_size;
        this->eeprom_size = eeprom_size;

        this->vector_words = vector_words;

        this->SPL = SPL;

        this->UDR0 = UDR0;
        this->UBRR0H = UBRR0H;
        this->UBRR0L = UBRR0L;
        this->UCSR0C = UCSR0C;
        this->UCSR0B = UCSR0B;
        this->UCSR0A = UCSR0A;
    }

    QString name;

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

static mcu_prototype atmega328p("ATmega 328p", true,  2048, 32768, 1024, 2, 0x5D, 0xC6, 0xC5, 0xC4, 0xC2, 0xC1, 0xC0);

#endif // MCU_PROTOTYPE_H
