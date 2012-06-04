/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef MCU_H
#define MCU_H

#include <QHash>
#include <QThread>
#include <QTimer>
#include <QMap>
#include <QMutex>
#include <vector>

#include "../shared/hexfile.h"

struct inst_prototype;
struct mcu_prototype;

class MCU;

struct instruction
{
    instruction()
    {
        arg1 = -1;
        arg2 = -1;
        prototype = NULL;
    }

    bool isNull() const { return arg1 == -1 && arg2 == -1 && prototype == NULL; }
    bool valid() const { return prototype != NULL; }

    int arg1;
    int arg2;
    inst_prototype *prototype;
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
        return ((addr[1] << 8) | addr[0]);
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

Q_SIGNALS:
    void realFreq(quint32 freq);
    void hackToEmulator(char c);

public:
    typedef QHash<quint32, instruction> InstMap;
    typedef quint8 (MCU::*instHandler)(int, int);
    typedef std::vector<quint8> vec;
    MCU();
    ~MCU();

    void init(HexFile *hex, mcu_prototype *proto);
    void startMCU();
    void stopMCU();

    void setFreq(quint32 freq)
    {
        m_freq = freq / 1000;
    }

    void setPaused(bool pause)
    {
        m_paused = pause;
    }
    bool isPaused() const { return m_paused; }

protected:
    void run();

private slots:
    void checkCycles();

private:
    inst_prototype *getInstPrototype(quint16 val);
    instHandler getInstHandler(quint8 id);
    void setDataMem16(int idx, quint16 val);
    instruction& getInstAt(quint32 idx);

    void AddToStack(quint8 byte);
    quint8 GetFromStack();

    mcu_prototype *m_protype;
    quint32 m_freq;

    volatile bool m_run;
    volatile bool m_paused;

    // Memories
    /*
     * 0x0000 - 0x001F - 32 data registers
     * 0x0020 - 0x005F - 64 I/O registers
     * 0x0060 - 0x00FF - 160 extended I/O registers
     * 0x0100 - xxxxxx - SRAM
     */
    vec m_data_mem;

    /*
     * - App flash section
     * - Bootloader - NYI?
     */
    vec m_prog_mem;
    vec m_eeprom;

    quint32 m_program_counter;
    wrapper_16 m_stack_pointer;
    wrapper_16 x_register;
    wrapper_16 y_register;
    wrapper_16 z_register;
    quint8 *m_sreg;
    quint8 *m_data_section;
    quint8 *m_bss_section;

    std::vector<instruction> m_instructions;

    // loop controls
    QTimer m_cycles_timer;
    quint8 m_cycles_debug;
    quint64 m_cycles_debug_counter;
    QMutex m_counter_mutex;
    volatile quint64 m_cycle_counter;
    quint8 m_cycles_sleep;

    // instruction handlers
    void check_ZNS(quint8 res);

public:
    quint8 in_adiw(int arg1, int arg2);
    quint8 in_bclr(int arg1, int /*arg2*/);
    quint8 in_brbc(int arg1, int arg2);
    quint8 in_bset(int arg1, int /*arg2*/);
    quint8 in_call(int arg1, int /*arg2*/);
    quint8 in_eor(int arg1, int arg2);
    quint8 in_jmp(int arg1, int /*arg2*/);
    quint8 in_ldd_y_plus(int arg1, int arg2);
    quint8 in_ldi(int arg1, int arg2);
    quint8 in_in(int arg1, int arg2);
    quint8 in_nop(int /*arg1*/, int /*arg2*/);
    quint8 in_out(int arg1, int arg2);
    quint8 in_push(int arg1, int /*arg2*/);
    quint8 in_rcall(int arg1, int /*arg2*/);
    quint8 in_reti(int /*arg1*/, int /*arg2*/);
    quint8 in_rjmp(int arg1, int /*arg2*/);
    quint8 in_sbci(int arg1, int arg2);
    quint8 in_std_y_plus(int arg1, int arg2);
    quint8 in_sts(int arg1, int arg2);
    quint8 in_subi(int arg1, int arg2);
};

#endif // MCU_H
