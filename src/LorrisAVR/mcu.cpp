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

#include "mcu.h"

MCU::MCU()
{
    m_freq = 20000000; // 20MHz
    m_protype = &atmega328p;

    m_data_mem = m_prog_mem = m_eeprom = NULL;
    m_data_section = m_bss_section = NULL;
}

MCU::~MCU()
{
    delete[] m_data_mem;
    delete[] m_prog_mem;
    delete[] m_eeprom;
}

void MCU::init(HexFile *hex)
{
    // Allocate memory
    m_data_mem = new quint8[m_protype->sram_size + 0xFF];
    m_prog_mem = new quint8[m_protype->prog_mem_size];
    m_eeprom = new quint8[m_protype->eeprom_size];

    m_data_section = m_data_mem + 0x100;

    // set stack pointer to byte behind sram end
    m_stack_pointer = wrapper_16(m_data_mem + m_protype->SPL);
    m_stack_pointer.set(m_protype->sram_size + 0xFF);

    x_register = wrapper_16(m_data_mem + 0x1A);
    y_register = wrapper_16(m_data_mem + 0x1C);
    z_register = wrapper_16(m_data_mem + 0x1E);
    m_sreg = m_data_mem + 0x5F;

    addInstHandlers();

    HexFile::regionMap& data = hex->getData();
    for(HexFile::regionMap::iterator itr = data.begin(); itr != data.end(); ++itr)
    {
        quint32 offset = itr->first;

        std::vector<quint8>& sec_data = itr->second;

        for(quint32 i = 0; i+1 < sec_data.size();)
        {
            quint8 first = sec_data[i];
            quint8 second = sec_data[i+1];

            m_prog_mem[offset + i] = first;
            m_prog_mem[offset + i+1] = second;

            quint16 inst_num = (second << 8) | first;
            inst_prototype *prot = getInstPrototype(inst_num);

            quint16 next_inst = 0;
            if(prot->words == 2)
            {
                Q_ASSERT(sec_data.size() > i+3);
                next_inst = (sec_data[i+3] << 8) | sec_data[i+2];
            }

            if(!prot)
            {
                //Q_ASSERT?
                qDebug() << "Unhandled instruction " << inst_num << second << first;
                continue;
            }
            int arg1 = arg_resolvers[prot->arg1](int_num, next_inst, offset+i);
            int arg2 = arg_resolvers[prot->arg1](int_num, next_inst, offset+i);

            instruction inst = {arg1, arg2, prot,};

            m_instructions.insert(offset+i, inst);

            i += prot->words*2;
        }
    }
}

inst_prototype *MCU::getInstPrototype(quint16 val)
{
    for(inst_prototype *p = instructions; p->name; ++p)
        if((val & p->mask) == p->opcode)
            return p;
    return NULL;
}

void MCU::addInstHandlers()
{
    m_handlers
}







