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

// There are random parts of code from
// GNU AVR Simulator (http://sourceforge.net/projects/avr/)
// used in this file

#ifndef MCU_H
#define MCU_H

#include <QTypeInfo>
#include <QHash>

enum argTypes
{
    REG_D5,               // 0000000d dddd0000 destantion
    REG_S5,               // 000000d0 0000dddd source
    CONST_6,              // 00000000 dd00dddd const
    BIT_SREG,             // 00000000 0ddd0000
    BIT_SREG2,            // 00000000 00000ddd
    BIT_NUM2,             // 00000000 00000ddd bld
    REG_D4,               // 00000000 dddd0000 destantion
    REG_S4,               // 00000000 0000dddd source
    NONE,
    NONE_LAST,
    ADDR_SHIFT,           // 000000dd ddddd000
    PORT,                 // 00000000 ddddd000 port I/O low
    PORT_ALL,             // 00000dd0 0000dddd port I/O all
    BYTE,                 // 0000dddd 0000dddd
    INDIR_ADDR,           // 00000000 000000dd
    INDIR_SHIFT,          // 00d0dd00 00000ddd ldd
    ADDR_SHIFT12,         // 0000dddd dddddddd rcall,rjmp
    REG_D2,               // 00000000 00dd0000
    ADDR22,               // for 22bit PC
    REG_S3,
    REG_D3,
    PUT_Z,
    PUT_ZPLUS,
    WORD_D,
    WORD_S,
    LONGCALL,
    WORD,
    ARG_TYPE_MAX
};

namespace ArgResolvers
{
    static int reg_d5(int cmd, int addr);
    static int reg_s5(int cmd, int addr);
    static int const_6(int cmd, int addr);
    static int bit_sreg(int cmd, int addr);
    static int bit_sreg2(int cmd, int addr);
    static int bit_num2(int cmd, int addr);
    static int reg_d4(int cmd, int addr);
    static int reg_s4(int cmd, int addr);
    static int none(int cmd, int addr);
    static int none_last(int cmd, int addr);
    static int addr_shift(int cmd, int addr);
    static int port(int cmd, int addr);
    static int port_all(int cmd, int addr);
    static int byte(int cmd, int addr);
    static int indir_addr(int cmd, int addr);
    static int indir_shift(int cmd, int addr);
    static int addr_shift2(int cmd, int addr);
    static int reg_d2(int cmd, int addr);
    static int addr22(int cmd, int addr);
    static int reg_s3(int cmd, int addr);
    static int reg_d3(int cmd, int addr);
    static int put_z(int cmd, int addr);
    static int put_zplus(int cmd, int addr);
    static int word_d(int cmd, int addr);
    static int word_s(int cmd, int addr);
    static int longcall(int cmd, int addr);
    static int word(int cmd, int addr);
}

static int (*arg_resolvers[ARG_TYPE_MAX])(int, int) =
{
    ArgResolvers::reg_d5,
    ArgResolvers::reg_s5,
    ArgResolvers::const_6,
    ArgResolvers::bit_sreg,
    ArgResolvers::bit_sreg2,
    ArgResolvers::bit_num2,
    ArgResolvers::reg_d4,
    ArgResolvers::reg_s4,
    ArgResolvers::none,
    ArgResolvers::none_last,
    ArgResolvers::addr_shift,
    ArgResolvers::port,
    ArgResolvers::port_all,
    ArgResolvers::byte,
    ArgResolvers::indir_addr,
    ArgResolvers::indir_shift,
    ArgResolvers::addr_shift12,
    ArgResolvers::reg_d2,
    ArgResolvers::addr22,
    ArgResolvers::reg_s3,
    ArgResolvers::reg_d3,
    ArgResolvers::put_z,
    ArgResolvers::put_zplus,
    ArgResolvers::word_d,
    ArgResolvers::word_s,
    ArgResolvers::longcall,
    ArgResolvers::word
};

struct Instruction
{
    const char* name;
    quint16 opcode;
    quint16 mask;
    quint8 words;
    argTypes arg1;
    argTypes arg2;
};

static Instruction instructions[] =
{
    {"adc",   0x1C00, 0xFC00, 1, REG_D5, REG_S5},
    {"add",   0x0C00, 0xFC00, 1, REG_D5, REG_S5},
    {"adiw",  0x9600, 0xFF00, 1, REG_D2, CONST_6},
    {"and",   0x2000, 0xFC00, 1, REG_D5, REG_S5},
    {"andi",  0x7000, 0xF000, 1, REG_D4, BYTE},
    {"asr",   0x9405, 0xFE0F, 1, REG_D5, NONE_LAST},
    {"bclr",  0x9488, 0xFF8F, 1, BIT_SREG, NONE_LAST},
    {"bld",   0xF800, 0xFE08, 1, REG_D5, BIT_NUM2},
    {"brbc",  0xF400, 0xFC00, 1, BIT_SREG2, ADDR_SHIFT},
    {"brbs",  0xF000, 0xFC00, 1, BIT_SREG2, ADDR_SHIFT},
    //brcc
    //brcs
    //break
    //breq
    //brge
    //brhc
    {"bset",  0x9408, 0xFF8F, 1, BIT_SREG, NONE_LAST},
    {"bst",   0xFA00, 0xFE08, 1, REG_D5, BIT_NUM2},
    {"call",  0x940E, 0xFE0E, 2, ADDR22, NONE_LAST},
    {"cbi",   0x9800, 0xFF00, 1,port,bit_num2,cbi_command},
    {"com",   0x9400, 0xFE0F, 1,reg_d5,none_last,com_command},
    {"cp",    0x1400, 0xFC00, 1,reg_d5,reg_s5,cp_command},
    {"cpc",   0x0400, 0xFC00, 1,reg_d5,reg_s5,cpc_command},
    {"cpi",   0x3000, 0xF000, 1,reg_d4,byte,cpi_command},
    {"cpse",  0x1000, 0xFC00, 1,reg_d5,reg_s5,cpse_command},
    {"dec",   0x940A, 0xFE0F, 1,reg_d5,none_last,dec_command},
    {"elpm",  0x95D8, 0xFFFF, 1,none,none_last,elpm1_command},
    {"elpm",  0x9006, 0xFE0F, 1,reg_d5,put_z,elpm2_command},
    {"elpm",  0x9007, 0xFE0F, 1,reg_d5,put_zplus,elpm3_command},
    {"eor",   0x2400, 0xFC00, 1,reg_d5,reg_s5,eor_command},
    {"fmul",  0x0308, 0xFF88, 1,reg_d3,reg_s3,fmul_command},
    {"fmuls", 0x0380, 0xFF88, 1,reg_d3,reg_s3,fmuls_command},
    {"fmulsu",0x0388, 0xFF88, 1,reg_d3,reg_s3,fmulsu_command},
    {"icall", 0x9509, 0xFFFF, 1,none,none_last,icall_command},
    {"ijmp",  0x9409, 0xFFFF, 1,none,none_last,ijmp_command},
    {"in",    0xB000, 0xF800, 1,reg_d5,port_all,in_command},
    {"inc",   0x9403, 0xFE0F, 1,reg_d5,none_last,inc_command},
    {"jmp",   0x940C, 0xFE0E, 2,addr22,none_last,jmp_command},
    {"ld",    0x900C, 0xFE0F, 1,reg_d5,indir_addr,ld_x_command},
    {"ld",    0x900D, 0xFE0F, 1,reg_d5,indir_addr,ld_x_plus_command},
    {"ld",    0x900E, 0xFE0F, 1,reg_d5,indir_addr,ld_minus_x_command},
    {"ld",    0x8008, 0xFE0F, 1,reg_d5,indir_addr,ld_y_command},
    {"ld",    0x9009, 0xFE0F, 1,reg_d5,indir_addr,ld_y_plus_command},
    {"ld",    0x900A, 0xFE0F, 1,reg_d5,indir_addr,ld_minus_y_command},
    {"ld",    0x8000, 0xFE0F, 1,reg_d5,indir_addr,ld_z_command},
    {"ld",    0x9001, 0xFE0F, 1,reg_d5,indir_addr,ld_z_plus_command},
    {"ld",    0x9002, 0xFE0F, 1,reg_d5,indir_addr,ld_minus_z_command},
    {"ldd",   0x8008, 0xD208, 1,reg_d5,indir_shift,ldd_y_command},
    {"ldd",   0x8000, 0xD208, 1,reg_d5,indir_shift,ldd_z_command},
    {"ldi",   0xE000, 0xF000, 1,reg_d4,byte,ldi_command},
    {"lpm",   0x95C8, 0xFFFF, 1,none,none_last,lpm_command},
    {"lpm",   0x9004, 0xFE0F, 1,reg_d5,indir_addr,lpm_z_command},
    {"lpm",   0x9005, 0xFE0F, 1,reg_d5,indir_addr,lpm_z_plus_command},
    {"lsr",   0x9406, 0xFE0F, 1,reg_d5,none_last,lsr_command},
    {"mov",   0x2C00, 0xFC00, 1,reg_d5,reg_s5,mov_command},
    {"movw",  0x0100, 0xFF00, 1,word_d,word_s,movw_command},
    {"mul",   0x9C00, 0xFC00, 1,reg_d5,reg_s5,mul_command},
    {"muls",  0x0200, 0xFF00, 1,reg_d4,reg_s4,muls_command},
    {"mulsu", 0x0300, 0xFF88, 1,reg_d4,reg_s4,mulsu_command},
    {"neg",   0x9401, 0xFE0F, 1,reg_d5,none_last,neg_command},
    {"nop",   0x0000, 0xFFFF, 1,none,none_last,nop_command},
    {"or",    0x2800, 0xFC00, 1,reg_d5,reg_s5,or_command},
    {"ori",   0x6000, 0xF000, 1,reg_d4,byte,ori_command},
    {"out",   0xB800, 0xF800, 1,port_all,reg_d5,out_command},
    {"pop",   0x900F, 0xFE0F, 1,reg_d5,none_last,pop_command},
    {"push",  0x920F, 0xFE0F, 1,reg_d5,none_last,push_command},
    {"rcall", 0xD000, 0xF000, 1,addr_shift12,none_last,rcall_command},
    {"ret",   0x9508, 0xFFFF, 1,none,none_last,ret_command},
    {"reti",  0x9518, 0xFFFF, 1,none,none_last,reti_command},
    {"rjmp",  0xC000, 0xF000, 1,addr_shift12,none_last,rjmp_command},
    {"ror",   0x9407, 0xFE0F, 1,reg_d5,none_last,ror_command},
    {"sbc",   0x0800, 0xFC00, 1,reg_d5,reg_s5,sbc_command},
    {"sbci",  0x4000, 0xF000, 1,reg_d4,byte,sbci_command},
    {"sbi",   0x9A00, 0xFF00, 1,port,bit_num2,sbi_command},
    {"sbic",  0x9900, 0xFF00, 1,port,bit_num2,sbic_command},
    {"sbis",  0x9B00, 0xFF00, 1,port,bit_num2,sbis_command},
    {"sbiw",  0x9700, 0xFF00, 1,reg_d2,const_6,sbiw_command},
    {"sbrc",  0xFC00, 0xFE08, 1,reg_d5,bit_num2,sbrc_command},
    {"sbrs",  0xFE00, 0xFE08, 1,reg_d5,bit_num2,sbrs_command},
    {"sleep", 0x9588, 0xFFFF, 1,none,none_last,sleep_command},
    {"st",    0x920C, 0xFE0F, 1,indir_addr,reg_d5,st_x_command},
    {"st",    0x920D, 0xFE0F, 1,indir_addr,reg_d5,st_x_plus_command},
    {"st",    0x920E, 0xFE0F, 1,indir_addr,reg_d5,st_minus_x_command},
    {"st",    0x8208, 0xFE0F, 1,indir_addr,reg_d5,st_y_command},
    {"st",    0x9209, 0xFE0F, 1,indir_addr,reg_d5,st_y_plus_command},
    {"st",    0x920A, 0xFE0F, 1,indir_addr,reg_d5,st_minus_y_command},
    {"std",   0x8208, 0xD208, 1,indir_shift,reg_d5,std_y_commad},
    {"st",    0x8200, 0xFE0F, 1,indir_addr,reg_d5,st_z_command},
    {"st",    0x9201, 0xFE0F, 1,indir_addr,reg_d5,st_z_plus_command},
    {"st",    0x9202, 0xFE0F, 1,indir_addr,reg_d5,st_minus_z_command},
    {"std",   0x8200, 0xD208, 1,indir_shift,reg_d5,std_z_commad},

    {"sub",   0x1800, 0xFC00, 1,reg_d5,reg_s5,sub_command},
    {"subi",  0x5000, 0xF000, 1,reg_d4,byte,subi_command},
    {"swap",  0x9402, 0xFE0F, 1,reg_d5,none_last,swap_command},
    {"wdr",   0x95A8, 0xFFFF, 1,none,none_last,wdr_command},
    {"lds",   0x9000, 0xFE0F, 2,reg_d5,word,lds_command},
    {"sts",   0x9200, 0xFE0F, 2,addr22,reg_d5,sts_command},

};

class MCU
{
public:
    MCU();
};


#endif // MCU_H
