/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef HANDLERS_H
#define HANDLERS_H

#include "mcu.h"

static quint8 (MCU::*instHandlers[INST_COUNT])(int, int) =
{
    NULL,            // 0
    NULL,            // 1
    &MCU::in_adiw,   // 2
    NULL,            // 3
    NULL,            // 4
    NULL,            // 5
    &MCU::in_bclr,   // 6
    NULL,            // 7
    &MCU::in_brbc,   // 8
    NULL,            // 9
    NULL,            // 10
    NULL,            // 11
    NULL,            // 12
    NULL,            // 13
    NULL,            // 14
    NULL,            // 15
    NULL,            // 16
    NULL,            // 17
    NULL,            // 18
    NULL,            // 19
    NULL,            // 20
    NULL,            // 21
    NULL,            // 22
    NULL,            // 23
    NULL,            // 24
    NULL,            // 25
    NULL,            // 26
    NULL,            // 27
    NULL,            // 28
    &MCU::in_bset,   // 29
    NULL,            // 30
    &MCU::in_call,   // 31
    NULL,            // 32
    NULL,            // 33
    NULL,            // 34
    NULL,            // 35
    NULL,            // 36
    NULL,            // 37
    NULL,            // 38
    NULL,            // 39
    NULL,            // 40
    NULL,            // 41
    NULL,            // 42
    NULL,            // 43
    NULL,            // 44
    NULL,            // 45
    NULL,            // 46
    NULL,            // 47
    NULL,            // 48
    NULL,            // 49
    NULL,            // 50
    NULL,            // 51
    NULL,            // 52
    NULL,            // 53
    NULL,            // 54
    &MCU::in_eor,    // 55
    NULL,            // 56
    NULL,            // 57
    NULL,            // 58
    NULL,            // 59
    NULL,            // 60
    &MCU::in_in,     // 61
    NULL,            // 62
    &MCU::in_jmp,    // 63
    NULL,            // 64
    NULL,            // 65
    NULL,            // 66
    NULL,            // 67
    NULL,            // 68
    NULL,            // 69
    NULL,            // 70
    NULL,            // 71
    NULL,            // 72
    NULL,            // 73
    NULL,            // 74
    NULL,            // 75
    &MCU::in_ldd_y_plus,// 76
    NULL,            // 77
    &MCU::in_ldi,    // 78
    NULL,            // 79
    NULL,            // 80
    NULL,            // 81
    NULL,            // 82
    NULL,            // 83
    NULL,            // 84
    NULL,            // 85
    NULL,            // 86
    NULL,            // 87
    NULL,            // 88
    NULL,            // 89
    NULL,            // 90
    NULL,            // 91
    &MCU::in_nop,    // 92
    NULL,            // 93
    NULL,            // 94
    &MCU::in_out,    // 95
    &MCU::in_pop,    // 96
    &MCU::in_push,   // 97
    &MCU::in_rcall,  // 98
    NULL,            // 99
    &MCU::in_reti,   // 100
    &MCU::in_rjmp,   // 101
    NULL,            // 102
    NULL,            // 103
    NULL,            // 104
    &MCU::in_sbci,   // 105
    NULL,            // 106
    NULL,            // 107
    NULL,            // 108
    NULL,            // 109
    NULL,            // 110
    NULL,            // 111
    NULL,            // 112
    NULL,            // 113
    NULL,            // 114
    NULL,            // 115
    NULL,            // 116
    NULL,            // 117
    NULL,            // 118
    NULL,            // 119
    NULL,            // 120
    NULL,            // 121
    NULL,            // 122
    NULL,            // 123
    NULL,            // 124
    NULL,            // 125
    NULL,            // 126
    NULL,            // 127
    NULL,            // 128
    NULL,            // 129
    &MCU::in_std_y_plus,// 130
    NULL,            // 131
    NULL,            // 132
    NULL,            // 133
    NULL,            // 134
    &MCU::in_sts,    // 135
    NULL,            // 136
    NULL,            // 137
    &MCU::in_subi,   // 138
    NULL,            // 139
    NULL,            // 140
    NULL             // 141
};

#endif // HANDLERS_H
