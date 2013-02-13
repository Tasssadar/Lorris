/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SHUPITOCC25XX_H
#define SHUPITOCC25XX_H

#include "../shupitopacket.h"
#include "shupitomode.h"

class ShupitoCC25XX : public ShupitoMode
{
    Q_OBJECT
public:
    ShupitoCC25XX(Shupito *shupito);

    chip_definition readDeviceId();
    void prepareMemForWriting(chip_definition::memorydef *memdef, chip_definition& chip);
    void erase_device(chip_definition& chip);
    void readMemRange(quint8, QByteArray& memory, quint32 address, quint32 size);
    void readFuses(std::vector<quint8> &data, chip_definition &chip);
    void writeFuses(std::vector<quint8> &data, chip_definition &chip, quint8 verifyMode);
    void flashPage(chip_definition::memorydef *memdef, std::vector<quint8>& memory, quint32 address);


protected:
    ShupitoDesc::config const *getModeCfg();

private:
    quint8 read_xdata(quint16 addr);
    void write_xdata(quint16 addr, quint8 data);

    quint8 read_sfr(quint8 addr);
    void write_sfr(quint8 addr, quint8 data);

    template <std::size_t N>
    quint8 execute_instr(quint8 const (&inst)[N]);
    quint8 execute_instr(char const *inst, std::size_t len);

    template <std::size_t N>
    ShupitoPacket execute_cmd(quint8 const (&cmd)[N], quint8 read_count);
    ShupitoPacket execute_cmd(char const *cmd, std::size_t len, quint8 read_count);
};

template <std::size_t N>
quint8 ShupitoCC25XX::execute_instr(quint8 const (&inst)[N])
{
    return execute_instr((const char*)inst, N);
}

template <std::size_t N>
ShupitoPacket ShupitoCC25XX::execute_cmd(quint8 const (&cmd)[N], quint8 read_count)
{
    return execute_cmd((const char*)cmd, N, read_count);
}

#endif // SHUPITOCC25XX_H
