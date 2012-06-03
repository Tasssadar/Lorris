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

#include <QDebug>
#include <stdio.h>
#include <QElapsedTimer>

#define DEBUG 1

#include "mcu.h"
#include "instructions.h"
#include "mcu_prototype.h"
#include "handlers.h"

MCU::MCU() : QThread()
{
    m_freq = 2000000 / 1000; // 2MHz

    m_data_section = m_bss_section = NULL;
    m_protype = NULL;

    m_run = true;
    m_paused = false;

    connect(&m_cycles_timer, SIGNAL(timeout()), SLOT(checkCycles()));
}

MCU::~MCU()
{
    m_run = false;
    wait();
}

void MCU::init(HexFile *hex, mcu_prototype *proto)
{
    m_protype = proto;

    // Allocate memory
    m_data_mem = vec(m_protype->sram_size + 0xFF, 0);
    m_prog_mem = vec(m_protype->prog_mem_size, 0);
    m_eeprom = vec(m_protype->eeprom_size, 0);

    m_data_section = m_data_mem.data() + 0x100;

    // set stack pointer to byte behind sram end
    // program should do this by itself
    m_stack_pointer = wrapper_16(m_data_mem.data() + m_protype->SPL);
    m_stack_pointer.set(m_protype->sram_size + 0x100);

    x_register = wrapper_16(m_data_mem.data() + 0x1A);
    y_register = wrapper_16(m_data_mem.data() + 0x1C);
    z_register = wrapper_16(m_data_mem.data() + 0x1E);

    m_sreg = m_data_mem.data() + 0x5F;
    *m_sreg = 0;

    m_program_counter = 0x00;

    m_instructions = std::vector<instruction>(m_protype->prog_mem_size/2);

    HexFile::regionMap& data = hex->getData();
    quint32 maxAddress = 0;
    for(HexFile::regionMap::iterator itr = data.begin(); itr != data.end(); ++itr)
    {
        quint32 offset = itr->first;

        std::vector<quint8>& sec_data = itr->second;
        if(offset+sec_data.size() > m_prog_mem.size())
            throw tr("Program is too big!");

        for(quint32 i = 0; i < sec_data.size(); ++i)
            m_prog_mem[offset+i] = sec_data[i];
        maxAddress = std::max(maxAddress, (quint32)(offset+sec_data.size()));
    }

#if DEBUG
    for(quint32 i = 0; i+1 < m_prog_mem.size() && i < maxAddress;)
    {
        instruction inst = getInstAt(i);
        if(!inst.valid())
        {
            i += 2;
            continue;
        }

        int argType[] = { inst.prototype->arg1, inst.prototype->arg2 };
        int arg[] = { inst.arg1, inst.arg2 };

        QString str = QString("0x%1: %2").arg(i, 4, 16, QChar('0')).arg(QString(inst.prototype->name), -5, QChar(' '));

        for(int y = 0; y < 2; ++y)
        {
            if(argType[y] == NONE)
                continue;
            switch(argType[y])
            {
                case ADDR_SHIFT:
                case ADDR_SHIFT12:
                    str += QString(".%1 ").arg(arg[y]);
                    break;
                default:
                    str += QString("0x%1 ").arg(arg[y], 0, 16);
                    break;
            }
        }
        qDebug() << str;
        i += inst.prototype->words*2;
    }
#endif
}

instruction& MCU::getInstAt(quint32 idx)
{
    quint32 instIdx = idx/2;
    if(!m_instructions[instIdx].isNull())
        return m_instructions[instIdx];

    instruction& inst = m_instructions[instIdx];
    inst.arg1 = 0;
    inst.arg2 = 0;

    if(idx+1 >= m_prog_mem.size())
        return inst;

    quint16 opcode = (m_prog_mem[idx+1] << 8) | m_prog_mem[idx];
    inst_prototype *prot = getInstPrototype(opcode);
    if(!prot)
        return inst;

    quint16 next_inst = 0;
    if(prot->words == 2)
    {
        if(idx+3 >= m_prog_mem.size())
            return inst;
        next_inst = (m_prog_mem[idx+3] << 8) | m_prog_mem[idx+2];
    }

    inst.arg1 = arg_resolvers[prot->arg1](opcode, next_inst, idx+prot->words*2);
    inst.arg2 = arg_resolvers[prot->arg2](opcode, next_inst, idx+prot->words*2);
    inst.prototype = prot;
    return inst;
}

inst_prototype *MCU::getInstPrototype(quint16 val)
{
    for(inst_prototype *p = instructions; p->name; ++p)
        if((val & p->mask) == p->opcode)
            return p;
    return NULL;
}

void MCU::startMCU()
{
    wait();

    m_run = true;
    start();

    m_cycles_debug = 0;
    m_cycles_debug_counter = 0;
    m_cycles_timer.start(1000);
}

void MCU::stopMCU()
{
    m_cycles_timer.stop();
    m_run = false;
    wait(1000);
}

void MCU::run()
{
    m_cycle_counter = 0;
    instHandler handler = NULL;

    QElapsedTimer workT;
    workT.start();
    msleep(50);

    while(m_run)
    {
        if(m_paused)
        {
            workT.restart();
            msleep(50);
            continue;
        }

        quint32 toDo = m_freq * workT.elapsed();
        workT.restart();
        for(quint32 i = 0; i < toDo;)
        {
            instruction& inst = getInstAt(m_program_counter);
            Q_ASSERT(inst.valid());

            m_program_counter += (inst.prototype->words*2);

            handler = instHandlers[inst.prototype->id];
            if(!handler)
            {
                qDebug("No handler for inst %s", inst.prototype->name);
                ++i;
                continue;
            }

            int ticks = (this->*handler)(inst.arg1, inst.arg2);
            i += ticks;

            m_counter_mutex.lock();
            m_cycle_counter += ticks;
            m_counter_mutex.unlock();
        }
        //m_counter_mutex.lock();
        //m_cycle_counter += toDo;
        //m_counter_mutex.unlock();
        msleep(std::max(int(50 - workT.elapsed()), 0));
    }
}

void MCU::checkCycles()
{
    quint32 cycles;
    {
        QMutexLocker l(&m_counter_mutex);
        cycles = m_cycle_counter;
        m_cycle_counter = 0;
    }
    emit realFreq(cycles);
}

void MCU::setDataMem16(int idx, quint16 val)
{
    m_data_mem[idx++] = (val >> 8);
    m_data_mem[idx] = (val & 0x0F);
}

void MCU::check_ZNS(quint8 res)
{
    quint8 val = *m_sreg;
    if(!res)
        val |= SREG_ZERO;
    else
        val &= ~(SREG_ZERO);

    if(res & 0x80)
        val |= SREG_NEG;
    else
        val &= ~(SREG_NEG);

    if(bool(val & SREG_NEG) ^ bool(val & SREG_V))
        val |= SREG_SIGN;
    else
        val &= ~(SREG_SIGN);

    *m_sreg = val;
}

quint8 MCU::in_adiw(int arg1, int arg2)
{
    wrapper_16 wrap(m_data_mem.data() + arg1);
    wrap += arg2;
    return 2;
}

quint8 MCU::in_bclr(int arg1, int /*arg2*/)
{
    *m_sreg &= ~(1 << arg1);
    return 1;
}

quint8 MCU::in_call(int arg1, int /*arg2*/)
{
    if(m_protype->bit16)
    {
        setDataMem16(m_stack_pointer.get(), m_program_counter);
        m_program_counter = arg1;
        m_stack_pointer -= 2;
        return 4;
    }
    else
    {
        m_data_mem[m_stack_pointer.get()] = (m_program_counter >> 16);
        m_data_mem[m_stack_pointer.get()+1] = (m_program_counter >> 8);
        m_data_mem[m_stack_pointer.get()+2] = (m_program_counter & 0xF);
        m_stack_pointer -= 3;
        m_program_counter = arg1;
        return 5;
    }
}

quint8 MCU::in_eor(int arg1, int arg2)
{
    m_data_mem[arg1] ^= m_data_mem[arg2];

    *m_sreg &= ~(SREG_V);
    check_ZNS(m_data_mem[arg1]);
    return 1;
}

quint8 MCU::in_jmp(int arg1, int /*arg2*/)
{
    m_program_counter = arg1;
    return 3;
}

quint8 MCU::in_ldd_y_plus(int arg1, int arg2)
{
    m_data_mem[arg1] = m_data_mem[y_register.get()+arg2];
    return 3;
}

quint8 MCU::in_ldi(int arg1, int arg2)
{
    m_data_mem[arg1] = arg2;
    return 1;
}

quint8 MCU::in_in(int arg1, int arg2)
{
    m_data_mem[arg2] = m_data_mem[arg1];
    return 1;
}

quint8 MCU::in_out(int arg1, int arg2)
{
    m_data_mem[arg1] = m_data_mem[arg2];
    return 1;
}

quint8 MCU::in_push(int arg1, int /*arg2*/)
{
    m_data_mem[m_stack_pointer.get()] = m_data_mem[arg1];
    --m_stack_pointer;
    return 2;
}

quint8 MCU::in_rcall(int arg1, int /*arg2*/)
{
    if(m_protype->bit16)
    {
        setDataMem16(m_stack_pointer.get(), m_program_counter);
        m_program_counter += arg1;
        m_stack_pointer -= 2;
        return 3;
    }
    else
    {
        m_data_mem[m_stack_pointer.get()] = (m_program_counter >> 16);
        m_data_mem[m_stack_pointer.get()+1] = (m_program_counter >> 8);
        m_data_mem[m_stack_pointer.get()+2] = (m_program_counter & 0xF);
        m_stack_pointer -= 3;
        m_program_counter += arg1;
        return 4;
    }
}

quint8 MCU::in_rjmp(int arg1, int /*arg2*/)
{
    m_program_counter += arg1;
    return 2;
}

quint8 MCU::in_std_y_plus(int arg1, int arg2)
{
    m_data_mem[y_register.get()+arg1] = m_data_mem[arg2];
    return 2;
}
