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

#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <QTypeInfo>

enum argTypes
{
    REG_D5,               // 0000000d dddd0000 destantion
    REG_S5,               // 000000d0 0000dddd source
    CONST_6,              // 00000000 dd00dddd const
    BIT_SREG,             // 00000000 0ddd0000
    BIT_SREG2,            // 00000000 00000ddd
    REG_D4,               // 00000000 dddd0000 destantion
    REG_S4,               // 00000000 0000dddd source
    NONE,
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
    ,
    WORD_S,
    LONGCALL,
    WORD,
    ARG_TYPE_MAX
};

namespace ArgResolvers
{
    static int reg_d5(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        return ((code1 & 0x01F0) >> 4);
    }

    static int reg_s5(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        return ((code1 & 0x0200) >> 5) | (code1 & 0x000F);
    }

    static int const_6(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        return ((code1 & 0x00C0) >> 2) | (code1 & 0xF);
    }

    static int bit_sreg(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        return ((code1 & 0x0070) >> 4);
    }

    static int bit_sreg2(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        return (code1 & 0b111);
    }

    static int reg_d4(quint16 code1, quint16 /*code2*/ quint32 /*address*/)
    {
        return ((code1 & 0x00F0) >> 4) | 0x10;
    }

    static int reg_s4(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        return ((code1 & 0x0F) | 0x10);
    }

    static int none(quint16 /*code1*/, quint16 /*code2*/, quint32 /*address*/)
    {
        return 0; // FIXME: return -1?
    }

    // dunno why, stolen from AVR simulator :/
    static int addr_shift(quint16 code1, quint16 /*code2*/, quint32 address)
    {
        int ret = ((code1 & 0x03F8) >> 3);
        if(code1 & 0x0200)
            ret |= (-1<<7);
        // ret += address+1; FIXME: I think I'll handle that in instruction handler
        return ret;
    }

    static int port(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        return ((code1 & 0x00F8) >> 3);
    }

    static int port_all(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        return ((code1 & 0b1100000) >> 5) | (code1 & 0x0F);
    }

    static int byte(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        return ((code1 & 0x0F00) >> 4) | (code1 & 0x0F);
    }

    static int indir_addr(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        return (code1 & 0x0003);
    }

    static int indir_shift(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        return ((code1 & 0x2000) >> 8) | ((code1 & 0x0C00) >> 7) | (code1 & 0b0111);
    }

    static int addr_shift12(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        int ret = 0;
        if(code1 & 0x0800)
            ret = (code1 | (-1^0xFFF));
        else
            ret = (code1 & 0x0FFF);
        return ret;
    }

    static int reg_d2(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        return ((code1 & 0b110000) >> 4) + 24;
    }

    static int addr22(quint16 code1, quint16 code2, quint32 /*address*/)
    {
        //g { unsigned short x = 0x940C;  unsigned short y = 0x0866; cout << hex << (y << (((x & 0b11111) >> 3) | (x & 0b1))); }
        return (code2 << ((code1 & 0b11111) >> 3) | (code1 & 0b1));
    }

    static int reg_s3(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        return (code1 & 0b111) + 16;
    }

    static int reg_d3(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        return ((code1 & 0b1110000) >> 4) + 16;
    }

    static int word_d(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        return ((code1 & 0x00F0) >> 3)
    }

    static int word_s(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        return ((code1 & 0x000F) << 1);
    }

    static int longcall(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        //FIXME: What is this?
        return 0;
    }

    static int word(quint16 /*code1*/, quint16 code2, quint32 /*address*/)
    {
        return code2;
    }
}

static int (*arg_resolvers[ARG_TYPE_MAX])(quint16, quint16, quint32) =
{
    ArgResolvers::reg_d5,
    ArgResolvers::reg_s5,
    ArgResolvers::const_6,
    ArgResolvers::bit_sreg,
    ArgResolvers::bit_sreg2,
    ArgResolvers::reg_d4,
    ArgResolvers::reg_s4,
    ArgResolvers::none,
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
    ArgResolvers::word_d,
    ArgResolvers::word_s,
    ArgResolvers::longcall,
    ArgResolvers::word
};

struct inst_prototype
{
    const char* name;
    quint16 opcode;
    quint16 mask;
    quint8 words;
    argTypes arg1;
    argTypes arg2;
};

static inst_prototype instructions[] =
{
    {"adc",   0x1C00, 0xFC00, 1, REG_D5, REG_S5},
    {"add",   0x0C00, 0xFC00, 1, REG_D5, REG_S5},
    {"adiw",  0x9600, 0xFF00, 1, REG_D2, CONST_6},
    {"and",   0x2000, 0xFC00, 1, REG_D5, REG_S5},
    {"andi",  0x7000, 0xF000, 1, REG_D4, BYTE},
    {"asr",   0x9405, 0xFE0F, 1, REG_D5, NONE},
    {"bclr",  0x9488, 0xFF8F, 1, BIT_SREG, NONE},
    {"bld",   0xF800, 0xFE08, 1, REG_D5, BIT_SREG2},
    {"brbc",  0xF400, 0xFC00, 1, BIT_SREG2, ADDR_SHIFT},
    {"brbs",  0xF000, 0xFC00, 1, BIT_SREG2, ADDR_SHIFT},
    {"brcc",  0xF400, 0xFC07, 1, ADDR_SHIFT, NONE},
    {"brcs",  0xF000, 0xFC07, 1, ADDR_SHIFT, NONE},
  //{"break", 0x9598, 0xFFFF, NONE, NONE},     // - on-chip debug, NYI
    {"breq",  0xF001, 0xFC07, 1, ADDR_SHIFT, NONE},
    {"brge",  0xF404, 0xFC07, 1, ADDR_SHIFT, NONE},
    {"brhc",  0xF405, 0xFC07, 1, ADDR_SHIFT, NONE},
    {"brhs",  0xF005, 0xFC07, 1, ADDR_SHIFT, NONE},
    {"brid",  0xF407, 0xFC07, 1, ADDR_SHIFT, NONE},
    {"brie",  0xF007, 0xFC07, 1, ADDR_SHIFT, NONE},
    {"brlo",  0xF000, 0xFC07, 1, ADDR_SHIFT, NONE},
    {"brlt",  0xF004, 0xFC07, 1, ADDR_SHIFT, NONE},
    {"brmi",  0xF002, 0xFC07, 1, ADDR_SHIFT, NONE},
    {"brne",  0xF401, 0xFC07, 1, ADDR_SHIFT, NONE},
    {"brpl",  0xF402, 0xFC07, 1, ADDR_SHIFT, NONE},
  //{"brsh",  0xF400, 0xFC07, 1, ADDR_SHIFT, NONE}, // WTF? same as brcc
    {"brtc",  0xF406, 0xFC07, 1, ADDR_SHIFT, NONE},
    {"brts",  0xF006, 0xFC07, 1, ADDR_SHIFT, NONE},
    {"brvc",  0xF403, 0xFC07, 1, ADDR_SHIFT, NONE},
    {"brvs",  0xF003, 0xFC07, 1, ADDR_SHIFT, NONE},
    {"bset",  0x9408, 0xFF8F, 1, BIT_SREG, NONE},
    {"bst",   0xFA00, 0xFE08, 1, REG_D5, BIT_SREG2},
    {"call",  0x940E, 0xFE0E, 2, ADDR22, NONE},
    {"cbi",   0x9800, 0xFF00, 1, PORT, BIT_SREG2},
    {"cbr",   0x7000, 0x7F0F, 1, REG_D4, NONE}, // andi Rd,0
    {"clc",   0x9488, 0xFFFF, 1, NONE, NONE},
    {"clh",   0x94D8, 0xFFFF, 1, NONE, NONE},
    {"cli",   0x94F8, 0xFFFF, 1, NONE, NONE},
    {"cln",   0x94A8, 0xFFFF, 1, NONE, NONE},
    {"clr",   0x9216, 0xFC00, 1, REG_D5, REG_S5}, // eor d,d
    {"cls",   0x94C8, 0xFFFF, 1, NONE, NONE},
    {"clt",   0x94E8, 0xFFFF, 1, NONE, NONE},
    {"clv",   0x94B8, 0xFFFF, 1, NONE, NONE},
    {"clz",   0x9498, 0xFFFF, 1, NONE, NONE},
    {"com",   0x9400, 0xFE0F, 1, REG_D5, NONE},
    {"cp",    0x1400, 0xFC00, 1, REG_D5, REG_S5},
    {"cpc",   0x0400, 0xFC00, 1, REG_D5, REG_S5},
    {"cpi",   0x3000, 0xF000, 1, REG_D4, BYTE},
    {"cpse",  0x1000, 0xFC00, 1, REG_D5, REG_S5},
    {"dec",   0x940A, 0xFE0F, 1, REG_D5, NONE},
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
    {NULL,    0,      0,      0, ARG_TYPE_MAX, ARG_TYPE_MAX, 0}
};

#endif // INSTRUCTIONS_H
