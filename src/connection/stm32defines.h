/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

/*
 * Some parts of this file are derived from project stlink:
 *
 *     https://github.com/texane/stlink
 *
 * See stlink_COPYING in root of Lorris' repository
 *
 */

#ifndef STM32DEFINES_H
#define STM32DEFINES_H

#define STLINK_OK			0x80
#define STLINK_FALSE			0x81
#define STLINK_CORE_RUNNING		0x80
#define STLINK_CORE_HALTED		0x81
#define STLINK_CORE_STAT_UNKNOWN	-1

#define STLINK_GET_VERSION		0xf1
#define STLINK_GET_CURRENT_MODE	0xf5

#define STLINK_DEBUG_COMMAND		0xF2
#define STLINK_DFU_COMMAND		0xF3
#define STLINK_DFU_ENTER		0x06
#define STLINK_DFU_EXIT		0x07
// enter dfu could be 0x08?

// STLINK_GET_CURRENT_MODE
#define STLINK_DEV_DFU_MODE		0x00
#define STLINK_DEV_MASS_MODE		0x01
#define STLINK_DEV_DEBUG_MODE		0x02
#define STLINK_DEV_UNKNOWN_MODE	-1

// jtag mode cmds
#define STLINK_DEBUG_ENTER		0x20
#define STLINK_DEBUG_EXIT		0x21
#define STLINK_DEBUG_READCOREID	0x22
#define STLINK_DEBUG_GETSTATUS		0x01
#define STLINK_DEBUG_FORCEDEBUG	0x02
#define STLINK_DEBUG_RESETSYS		0x03
#define STLINK_DEBUG_READALLREGS	0x04
#define STLINK_DEBUG_READREG		0x05
#define STLINK_DEBUG_WRITEREG		0x06
#define STLINK_DEBUG_READMEM_32BIT	0x07
#define STLINK_DEBUG_WRITEMEM_32BIT	0x08
#define STLINK_DEBUG_RUNCORE		0x09
#define STLINK_DEBUG_STEPCORE		0x0a
#define STLINK_DEBUG_SETFP		0x0b
#define STLINK_DEBUG_WRITEMEM_8BIT	0x0d
#define STLINK_DEBUG_CLEARFP		0x0e
#define STLINK_DEBUG_WRITEDEBUGREG	0x0f
#define STLINK_DEBUG_ENTER_SWD		0xa3
#define STLINK_DEBUG_ENTER_JTAG	0x00

#define STLINK_SWD_ENTER 0x30
#define STLINK_SWD_READCOREID 0x32  // TBD
#define STLINK_JTAG_WRITEDEBUG_32BIT 0x35
#define STLINK_JTAG_READDEBUG_32BIT 0x36
#define STLINK_JTAG_DRIVE_NRST 0x3c
#define STLINK_JTAG_DRIVE_NRST 0x3c

#define CM3_REG_CPUID 0xE000ED00
#define CM3_REG_FP_CTRL 0xE0002000
#define CM3_REG_FP_COMP0 0xE0002008

/* cortex core ids */
    // TODO clean this up...
#define STM32VL_CORE_ID 0x1ba01477
#define STM32L_CORE_ID 0x2ba01477
#define STM32F3_CORE_ID 0x2ba01477
#define STM32F4_CORE_ID 0x2ba01477
#define STM32F0_CORE_ID 0xbb11477
#define CORE_M3_R1 0x1BA00477
#define CORE_M3_R2 0x4BA00477
#define CORE_M4_R0 0x2BA01477

/*
 * Chip IDs are explained in the appropriate programming manual for the
 * DBGMCU_IDCODE register (0xE0042000)
 */
// stm32 chipids, only lower 12 bits..
#define STM32_CHIPID_F1_MEDIUM 0x410
#define STM32_CHIPID_F2 0x411
#define STM32_CHIPID_F1_LOW 0x412
#define STM32_CHIPID_F3 0x422
#define STM32_CHIPID_F37x 0x432
#define STM32_CHIPID_F4 0x413
#define STM32_CHIPID_F4_LP 0x423
#define STM32_CHIPID_F1_HIGH 0x414
#define STM32_CHIPID_L1_MEDIUM 0x416
#define STM32_CHIPID_L1_MEDIUM_PLUS 0x427
/*
 * 0x436 is actually assigned to some L1 chips that are called "Medium-Plus"
 * and some that are called "High".  0x427 is assigned to the other "Medium-
 * plus" chips.  To make it a bit simpler we just call 427 MEDIUM_PLUS and
 * 0x436 HIGH.
 */
#define STM32_CHIPID_L1_HIGH 0x436
#define STM32_CHIPID_F1_CONN 0x418
#define STM32_CHIPID_F1_VL_MEDIUM 0x420
#define STM32_CHIPID_F1_VL_HIGH 0x428
#define STM32_CHIPID_F1_XL 0x430
#define STM32_CHIPID_F0 0x440
#define STM32_CHIPID_F0_SMALL 0x444

// Constant STM32 memory map figures
#define STM32_FLASH_BASE 0x08000000
#define STM32_SRAM_BASE 0x20000000


// ARM debug registers
#define DHCSR 0xe000edf0
#define DCRSR 0xe000edf4
#define DCRDR 0xe000edf8
#define DEMCR 0xe000edfc
#define DBGKEY 0xa05f0000

#define DHCSR_C_DEBUGEN (1 << 0)
#define DHCSR_C_HALT    (1 << 1)

// System control block register - 0xE000ED00
// STM32F3xxx and STM32F4xxx Cortex-M4 programming manual (PM0214)
#define AIRCR 0xE000ED0C
#define AIRCR_SYSRESETREQ (1 << 2)
#define AIRCR_VECTKEY 0x05FA0000

#endif // STM32DEFINES_H
