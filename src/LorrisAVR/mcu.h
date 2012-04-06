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

#ifndef MCU_H
#define MCU_H

#include <QHash>
#include <QThread>
#include <QTimer>
#include <QMap>
#include <QMutex>

#include "hexfile.h"

struct inst_prototype;
struct mcu_prototype;

class MCU;

struct instruction
{
    int arg1;
    int arg2;
    inst_prototype *prototype;
    quint8 (MCU::*handler)(int, int);
};

struct wrapper_16
{
    wrapper_16()
    {
        addr = NULL;
    }

    wrapper_16(quint8 *low_byte)
    {
        this->addr = low_byte;
    }

    void set(quint16 val)
    {
        addr[0] = (val & 0xFF);
        addr[1] = (val >> 8);
    }

    quint16 get()
    {
        return (addr[1] << 8) || addr[0];
    }

    wrapper_16& operator= (const quint16 &val)
    {
        set(val);
        return *this;
    }

    wrapper_16& operator ++()
    {
        set(get()+1);
        return *this;
    }

    wrapper_16& operator --()
    {
        set(get()-1);
        return *this;
    }

    wrapper_16& operator +=(const quint16& val)
    {
        set(get()+val);
        return *this;
    }

    wrapper_16& operator -=(const quint16& val)
    {
        set(get()+val);
        return *this;
    }

    quint8 *addr;
};

class MCU : public QThread
{
    Q_OBJECT
public:
    typedef QHash<quint32, instruction> InstMap;
    typedef quint8 (MCU::*instHandler)(int, int);
    MCU();
    ~MCU();

    void init(HexFile *hex);
    void startMCU();

protected:
    void run();

private slots:
    void checkCycles();

private:
    inst_prototype *getInstPrototype(quint16 val);
    void addInstHandlers();
    instHandler getInstHandler(quint8 id);
    void setDataMem16(int idx, quint16 val);

    mcu_prototype *m_protype;
    quint32 m_freq;

    MCU *m_self;

    volatile bool m_run;

    // Memories
    /*
     * 0x0000 - 0x001F - 32 data registers
     * 0x0020 - 0x005F - 64 I/O registers
     * 0x0060 - 0x00FF - 160 extended I/O registers
     * 0x0100 - xxxxxx - SRAM
     */
    quint8 *m_data_mem;

    /*
     * - App flash section
     * - Bootloader - NYI?
     */
    quint8 *m_prog_mem;
    quint8 *m_eeprom;

    quint32 m_program_counter;
    wrapper_16 m_stack_pointer;
    wrapper_16 x_register;
    wrapper_16 y_register;
    wrapper_16 z_register;
    quint8 *m_sreg;
    quint8 *m_data_section;
    quint8 *m_bss_section;

    instruction **m_instructions;
    QHash<quint8, instHandler> m_handlers;

    // loop controls
    QTimer m_cycles_timer;
    quint8 m_cycles_debug;
    quint64 m_cycles_debug_counter;
    QMutex m_counter_mutex;
    volatile quint64 m_nop_count;
    volatile quint64 m_cycle_counter;
    quint8 m_cycles_sleep;

    // instruction handlers
    void check_ZNS(quint8 res);

public:
    quint8 __adiw(int arg1, int arg2);
    quint8 __bclr(int arg1, int /*arg2*/);
    quint8 __call(int arg1, int /*arg2*/);
    quint8 __eor(int arg1, int arg2);
    quint8 __jmp(int arg1, int /*arg2*/);
    quint8 __ldd_y_plus(int arg1, int arg2);
    quint8 __ldi(int arg1, int arg2);
    quint8 __in(int arg1, int arg2);
    quint8 __out(int arg1, int arg2);
    quint8 __push(int arg1, int /*arg2*/);
    quint8 __rcall(int arg1, int /*arg2*/);
    quint8 __rjmp(int arg1, int /*arg2*/);
    quint8 __std_y_plus(int arg1, int arg2);
};


#endif // MCU_H
