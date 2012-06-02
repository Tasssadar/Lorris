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
    WORD_D,
    WORD_S,
    LONGCALL,
    WORD,
    BYTE_SIGNED,
    SHIFT_LEFT,
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

    static int reg_d4(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
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

    static int addr_shift(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        int ret = ((code1 & 0x03F8) >> 3);
        if(code1 & 0x0200)
            ret |= (-1<<7);

        // ret += address+1; FIXME: I think I'll handle that in instruction handler
        return ret*2;
    }

    static int port(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        return ((code1 & 0x00F8) >> 3);
    }

    static int port_all(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        return ((code1 & 0x0600) >> 5) | (code1 & 0x0F);
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
            ret = (code1 | (-1^0xFFF))*2;
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
        return ((code1 & 0x00F0) >> 3);
    }

    static int word_s(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        return ((code1 & 0x000F) << 1);
    }

    static int longcall(quint16 /*code1*/, quint16 /*code2*/, quint32 /*address*/)
    {
        //FIXME: What is this?
        return 0;
    }

    static int word(quint16 /*code1*/, quint16 code2, quint32 /*address*/)
    {
        return code2;
    }

    static int byte_signed(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        return ((code1 & 0x0700) >> 4) | (code1 & 0x000F);
    }

    static int shift_left(quint16 code1, quint16 /*code2*/, quint32 /*address*/)
    {
        return (code1 & 0x3FF);
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
    ArgResolvers::word,
    ArgResolvers::byte_signed,
    ArgResolvers::shift_left
};

struct inst_prototype
{
    quint8 id;
    const char* name;
    quint16 opcode;
    quint16 mask;
    quint8 words;
    argTypes arg1;
    argTypes arg2;
};

static inst_prototype instructions[] =
{
    {0,  "adc",   0x1C00, 0xFC00, 1, REG_D5, REG_S5},
    {1,  "add",   0x0C00, 0xFC00, 1, REG_D5, REG_S5},
    {2,  "adiw",  0x9600, 0xFF00, 1, REG_D2, CONST_6},
    {3,  "and",   0x2000, 0xFC00, 1, REG_D5, REG_S5},
    {4,  "andi",  0x7000, 0xF000, 1, REG_D4, BYTE},
    {5,  "asr",   0x9405, 0xFE0F, 1, REG_D5, NONE},
    {6,  "bclr",  0x9488, 0xFF8F, 1, BIT_SREG, NONE},
    {7,  "bld",   0xF800, 0xFE08, 1, REG_D5, BIT_SREG2},
    {8,  "brbc",  0xF400, 0xFC00, 1, BIT_SREG2, ADDR_SHIFT},
    {9,  "brbs",  0xF000, 0xFC00, 1, BIT_SREG2, ADDR_SHIFT},

  // These are just named variations of brbs/brbc
  //{10, "brcc",  0xF400, 0xFC07, 1, ADDR_SHIFT, NONE}, brbc 0, k
  //{11, "brcs",  0xF000, 0xFC07, 1, ADDR_SHIFT, NONE}, brbs 0, k
  //{13, "breq",  0xF001, 0xFC07, 1, ADDR_SHIFT, NONE}, brbs 1, k
  //{14, "brge",  0xF404, 0xFC07, 1, ADDR_SHIFT, NONE}, brbc 4, k
  //{15, "brhc",  0xF405, 0xFC07, 1, ADDR_SHIFT, NONE}, brbc 5, k
  //{16, "brhs",  0xF005, 0xFC07, 1, ADDR_SHIFT, NONE}, brbs 5, k
  //{17, "brid",  0xF407, 0xFC07, 1, ADDR_SHIFT, NONE}, brbc 7, k
  //{18, "brie",  0xF007, 0xFC07, 1, ADDR_SHIFT, NONE}, brbs 7, k
  //{19, "brlo",  0xF000, 0xFC07, 1, ADDR_SHIFT, NONE}, brbs 0, k
  //{20, "brlt",  0xF004, 0xFC07, 1, ADDR_SHIFT, NONE}, brbs 4, k
  //{21, "brmi",  0xF002, 0xFC07, 1, ADDR_SHIFT, NONE}, brbs 2, k
  //{22, "brne",  0xF401, 0xFC07, 1, ADDR_SHIFT, NONE}, brbc 1, k
  //{23, "brpl",  0xF402, 0xFC07, 1, ADDR_SHIFT, NONE}, brbc 2, k
  //{24, "brsh",  0xF400, 0xFC07, 1, ADDR_SHIFT, NONE}, brbc 0, k
  //{25, "brtc",  0xF406, 0xFC07, 1, ADDR_SHIFT, NONE}, brbc 6, k
  //{26, "brts",  0xF006, 0xFC07, 1, ADDR_SHIFT, NONE}, brbs 6, k
  //{27, "brvc",  0xF403, 0xFC07, 1, ADDR_SHIFT, NONE}, brbc 3, k
  //{28, "brvs",  0xF003, 0xFC07, 1, ADDR_SHIFT, NONE}, brbs 3, k

  //{12, "break", 0x9598, 0xFFFF, NONE, NONE},     // - on-chip debug, NYI

    {29, "bset",  0x9408, 0xFF8F, 1, BIT_SREG, NONE},
    {30, "bst",   0xFA00, 0xFE08, 1, REG_D5, BIT_SREG2},
    {31, "call",  0x940E, 0xFE0E, 2, ADDR22, NONE},
    {32, "cbi",   0x9800, 0xFF00, 1, PORT, BIT_SREG2},
    {33, "cbr",   0x7000, 0x7F0F, 1, REG_D4, NONE}, // andi Rd,0
    {34, "clc",   0x9488, 0xFFFF, 1, NONE, NONE},
    {35, "clh",   0x94D8, 0xFFFF, 1, NONE, NONE},
    {36, "cli",   0x94F8, 0xFFFF, 1, NONE, NONE},
    {37, "cln",   0x94A8, 0xFFFF, 1, NONE, NONE},
    {38, "clr",   0x9216, 0xFC00, 1, REG_D5, REG_S5}, // eor d,d
    {39, "cls",   0x94C8, 0xFFFF, 1, NONE, NONE},
    {40, "clt",   0x94E8, 0xFFFF, 1, NONE, NONE},
    {41, "clv",   0x94B8, 0xFFFF, 1, NONE, NONE},
    {42, "clz",   0x9498, 0xFFFF, 1, NONE, NONE},
    {43, "com",   0x9400, 0xFE0F, 1, REG_D5, NONE},
    {44, "cp",    0x1400, 0xFC00, 1, REG_D5, REG_S5},
    {45, "cpc",   0x0400, 0xFC00, 1, REG_D5, REG_S5},
    {46, "cpi",   0x3000, 0xF000, 1, REG_D4, BYTE},
    {47, "cpse",  0x1000, 0xFC00, 1, REG_D5, REG_S5},
    {48, "dec",   0x940A, 0xFE0F, 1, REG_D5, NONE},
  //{49, "des",   0x940B, 0xFF0F, 1, NYI, NONE},  //data encryption, NYI
  //{50, "eicall",0x9519, 0xFFFF, 1, NONE, NONE}, // 22bit, NYI
    {51, "eijmp", 0x9419, 0xFFFF, 1, NONE, NONE},
    {52, "elpm",  0x95D8, 0xFFFF, 1, NONE, NONE},
    {53, "elpm",  0x9006, 0xFE0F, 1, REG_D5, NONE},
    {54, "elpm",  0x9007, 0xFE0F, 1, REG_D5, NONE},
    {55, "eor",   0x2400, 0xFC00, 1, REG_D5, REG_S5},
    {56, "fmul",  0x0308, 0xFF88, 1, REG_D3, REG_S3},
    {57, "fmuls", 0x0380, 0xFF88, 1, REG_D3, REG_S3},
    {58, "fmulsu",0x0388, 0xFF88, 1, REG_D3, REG_S3},
    {59, "icall", 0x9509, 0xFFFF, 1, NONE, NONE},
    {60, "ijmp",  0x9409, 0xFFFF, 1, NONE, NONE},
    {61, "in",    0xB000, 0xF800, 1, REG_D5, PORT_ALL},
    {62, "inc",   0x9403, 0xFE0F, 1, REG_D5, NONE},
    {63, "jmp",   0x940C, 0xFE0E, 2, ADDR22, NONE},
    {64, "lac",   0x4906, 0xFE0F, 1, REG_D5, NONE},
    {65, "las",   0x9205, 0xFE0F, 1, REG_D5, NONE},
    {66, "lat",   0x9207, 0xFE0F, 1, REG_D5, NONE},
    {67, "ld",    0x900C, 0xFE0F, 1, REG_D5, INDIR_ADDR},
    {68, "ld",    0x900D, 0xFE0F, 1, REG_D5, INDIR_ADDR},
    {69, "ld",    0x900E, 0xFE0F, 1, REG_D5, INDIR_ADDR},
    {70, "ld",    0x8008, 0xFE0F, 1, REG_D5, INDIR_ADDR},
    {71, "ld",    0x9009, 0xFE0F, 1, REG_D5, INDIR_ADDR},
    {72, "ld",    0x900A, 0xFE0F, 1, REG_D5, INDIR_ADDR},
    {73, "ld",    0x8000, 0xFE0F, 1, REG_D5, INDIR_ADDR},
    {74, "ld",    0x9001, 0xFE0F, 1, REG_D5, INDIR_ADDR},
    {75, "ld",    0x9002, 0xFE0F, 1, REG_D5, INDIR_ADDR},
    {76, "ldd",   0x8008, 0xD208, 1, REG_D5, INDIR_SHIFT},
    {77, "ldd",   0x8000, 0xD208, 1, REG_D5, INDIR_SHIFT},
    {78, "ldi",   0xE000, 0xF000, 1, REG_D4, BYTE},
    {79, "lds",   0x9000, 0xFE0F, 2, REG_D5, WORD},
    {80, "lds",   0xA000, 0xF800, 1, REG_D4, BYTE_SIGNED},
    {81, "lpm",   0x95C8, 0xFFFF, 1, NONE, NONE},
    {82, "lpm",   0x9004, 0xFE0F, 1, REG_D5, INDIR_ADDR},
    {83, "lpm",   0x9005, 0xFE0F, 1, REG_D5, INDIR_ADDR},
    {84, "lsl",   0x0C00, 0xFC00, 1, SHIFT_LEFT, NONE},
    {85, "lsr",   0x9406, 0xFE0F, 1, REG_D5, NONE},
    {86, "mov",   0x2C00, 0xFC00, 1, REG_D5, REG_S5},
    {87, "movw",  0x0100, 0xFF00, 1, WORD_D, WORD_S},
    {88, "mul",   0x9C00, 0xFC00, 1, REG_D5, REG_S5},
    {89, "muls",  0x0200, 0xFF00, 1, REG_D4, REG_S4},
    {90, "mulsu", 0x0300, 0xFF88, 1, REG_D4, REG_S4},
    {91, "neg",   0x9401, 0xFE0F, 1, REG_D5, NONE},
    {92, "nop",   0x0000, 0xFFFF, 1, NONE, NONE},
    {93, "or",    0x2800, 0xFC00, 1, REG_D5, REG_S5},
    {94, "ori",   0x6000, 0xF000, 1, REG_D4, BYTE},
    {95, "out",   0xB800, 0xF800, 1, PORT_ALL, REG_D5},
    {96, "pop",   0x900F, 0xFE0F, 1, REG_D5, NONE},
    {97, "push",  0x920F, 0xFE0F, 1, REG_D5, NONE},
    {98, "rcall", 0xD000, 0xF000, 1, ADDR_SHIFT12, NONE},
    {99, "ret",   0x9508, 0xFFFF, 1, NONE, NONE},
    {100,"reti",  0x9518, 0xFFFF, 1, NONE, NONE},
    {101,"rjmp",  0xC000, 0xF000, 1, ADDR_SHIFT12, NONE},
    {102,"rol",   0x1c00, 0xFC00, 1, SHIFT_LEFT, NONE},
    {103,"ror",   0x9407, 0xFE0F, 1, REG_D5, NONE},
    {104,"sbc",   0x0800, 0xFC00, 1, REG_D5, REG_S5},
    {105,"sbci",  0x4000, 0xF000, 1, REG_D4, BYTE},
    {106,"sbi",   0x9A00, 0xFF00, 1, PORT, BIT_SREG2},
    {107,"sbic",  0x9900, 0xFF00, 1, PORT, BIT_SREG2},
    {108,"sbis",  0x9B00, 0xFF00, 1, PORT, BIT_SREG2},
    {109,"sbiw",  0x9700, 0xFF00, 1, REG_D2, CONST_6},
    {110,"sbr",   0x6000, 0xF000, 1, REG_D4, BYTE},
    {111,"sbrc",  0xFC00, 0xFE08, 1, REG_D5, BIT_SREG2},
    {112,"sbrs",  0xFE00, 0xFE08, 1, REG_D5, BIT_SREG2},
    {113,"sec",   0x9408, 0xFFFF, 1, NONE, NONE},
    {114,"seh",   0x9458, 0xFFFF, 1, NONE, NONE},
    {115,"sei",   0x9478, 0xFFFF, 1, NONE, NONE},
    {116,"sen",   0x9428, 0xFFFF, 1, NONE, NONE},
    {117,"ser",   0xEF0F, 0xFF0F, 1, REG_D4, NONE},
    {118,"ses",   0x9448, 0xFFFF, 1, NONE, NONE},
    {119,"set",   0x9468, 0xFFFF, 1, NONE, NONE},
    {120,"sev",   0x9438, 0xFFFF, 1, NONE, NONE},
    {121,"sez",   0x9418, 0xFFFF, 1, NONE, NONE},
    {122,"sleep", 0x9588, 0xFFFF, 1, NONE, NONE}, //NYI
    //123 - spm - self-programing - NYI
    {124,"st",    0x920C, 0xFE0F, 1, INDIR_ADDR, REG_D5},
    {125,"st",    0x920D, 0xFE0F, 1, INDIR_ADDR, REG_D5},
    {126,"st",    0x920E, 0xFE0F, 1, INDIR_ADDR, REG_D5},
    {127,"st",    0x8208, 0xFE0F, 1, INDIR_ADDR, REG_D5},
    {128,"st",    0x9209, 0xFE0F, 1, INDIR_ADDR, REG_D5},
    {129,"st",    0x920A, 0xFE0F, 1, INDIR_ADDR, REG_D5},
    {130,"std",   0x8208, 0xD208, 1, INDIR_SHIFT, REG_D5},
    {131,"st",    0x8200, 0xFE0F, 1, INDIR_ADDR, REG_D5},
    {132,"st",    0x9201, 0xFE0F, 1, INDIR_ADDR, REG_D5},
    {133,"st",    0x9202, 0xFE0F, 1, INDIR_ADDR, REG_D5},
    {134,"std",   0x8200, 0xD208, 1, INDIR_ADDR, REG_D5},
    {135,"sts",   0x9200, 0xFE0F, 2, ADDR22, REG_D5},
    {136,"sts",   0xA800, 0xF800, 1, REG_D4, BYTE_SIGNED},
    {137,"sub",   0x1800, 0xFC00, 1, REG_D5, REG_S5},
    {138,"subi",  0x5000, 0xF000, 1, REG_D4, BYTE},
    {139,"swap",  0x9402, 0xFE0F, 1, REG_D5, NONE},
    {140,"tst",   0x2000, 0xFC00, 1, SHIFT_LEFT, NONE},
    {140,"wdr",   0x95A8, 0xFFFF, 1, NONE, NONE},
    {141,"xch",   0x9204, 0xFE0F, 1, REG_D5, NONE},
    {255, NULL,   0,      0,      0, NONE, NONE}
};
#define INST_COUNT 142

#endif // INSTRUCTIONS_H
