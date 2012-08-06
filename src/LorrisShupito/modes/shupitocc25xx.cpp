/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "../../common.h"
#include "../shupito.h"
#include "shupitocc25xx.h"
#include "../../shared/hexfile.h"

ShupitoCC25XX::ShupitoCC25XX(Shupito *shupito) : ShupitoMode(shupito)
{
}

ShupitoDesc::config *ShupitoCC25XX::getModeCfg()
{
    return m_shupito->getDesc()->getConfig("76e37480-3f61-4e7a-9b1b-37af6bd418fa");
}

chip_definition ShupitoCC25XX::readDeviceId()
{
    m_prepared = false;

    quint8 chipid = read_xdata(0x624A);
    QString id = "cc25xx:" + Utils::toBase16(&chipid, &chipid + 1);

    quint8 chipinfo0 = read_xdata(0x6276);
    quint8 chipinfo1 = read_xdata(0x6277);

    m_prepared = true;

    chip_definition cd(id);
    chip_definition::memorydef flash_memdef;
    flash_memdef.memid = MEM_FLASH;
    flash_memdef.pagesize = 128;

    switch((chipinfo0 >> 4) & 7)
    {
        case 1: flash_memdef.size = 32*1024; break;
        case 2: flash_memdef.size = 64*1024; break;
        case 3: flash_memdef.size = (chipid == 0x95) ? 96*1024 : 128*1024; break;
        case 4: flash_memdef.size = 256*1024; break;
        default:flash_memdef.size = 0; break;
    }

    if(flash_memdef.size)
        cd.getMems()["flash"] = flash_memdef;

    chip_definition::memorydef sdram_memdef;
    sdram_memdef.memid = MEM_SDRAM;
    sdram_memdef.size = ((chipinfo1 & 7) +1)*1024;
    cd.getMems()["sdram"] = sdram_memdef;

    return cd;
}

void ShupitoCC25XX::prepareMemForWriting(chip_definition::memorydef */*memdef*/, chip_definition &chip)
{
    this->erase_device(chip);

    quint16 sdram_size = chip.getMemDef(MEM_SDRAM)->size;

    quint8 trigger = 18; // flash
    quint16 len = 128;
    quint16 src_addr = sdram_size - 0x100;
    quint16 dest_addr = 0x6273;

    write_xdata(0, src_addr >> 8);
    write_xdata(1, src_addr);
    write_xdata(2, dest_addr >> 8);
    write_xdata(3, dest_addr);
    write_xdata(4, len >> 8);
    write_xdata(5, len);
    write_xdata(6, trigger);
    write_xdata(7, (1 << 6) | 1);

    write_sfr(0xD4, 0);
    write_sfr(0xD5, 0);

    quint8 read_config[] = { 0x20 };
    ShupitoPacket pkt = execute_cmd(read_config, 1);

    quint8 write_config[] = { 0x18, quint8(pkt[0] & ~(1<<2)) };
    execute_cmd(write_config, 1);
}

void ShupitoCC25XX::erase_device(chip_definition &/*chip*/)
{
    const quint8 chip_erase_cmd[] = { 0x10 };
    const quint8 read_status_cmd[] = { 0x30 };

    ShupitoPacket pkt = execute_cmd(chip_erase_cmd, 1);
    while(pkt[0] & 0x80)
        pkt = execute_cmd(read_status_cmd, 1);
}

void ShupitoCC25XX::readMemRange(quint8 /*memid*/, QByteArray &memory, quint32 address, quint32 size)
{
    quint32 bank = address >> 15;
    quint32 offset = (address & 0x7FFF) | 0x8000;
    for(; size > 0; ++bank, offset = 0x8000)
    {
        write_sfr(0xC7, bank & 7); // MEMCTR

        quint8 load_dptr_instr[] = { 0x90, quint8(offset >> 8), (quint8)offset }; // mov DPTR, offset
        execute_instr(load_dptr_instr);

        quint32 chunk = 0x10000 - offset;
        if(chunk > size)
            chunk = size;
        size -= chunk;

        ShupitoPacket pkt = makeShupitoPacket(m_prog_cmd_base + 3, 0x02, (quint8)chunk, (quint8)(chunk >> 8));

        QByteArray data = m_shupito->waitForStream(pkt, m_prog_cmd_base + 3);

        int data_size = data.size();
        if(data_size == 0 || ((char)data[data_size-1] == 0 && (quint32)data_size != chunk + 1))
            throw QString(QObject::tr("Invalid response."));

        if((char)data[data_size-1] != 0)
            throw QString(QObject::tr("Failed to read memory."));

        data.chop(1);
        memory.append(data);
    }
}

void ShupitoCC25XX::readFuses(std::vector<quint8> &/*data*/, chip_definition &/*chip*/)
{
    throw QString(QObject::tr("Reading fuses is not supported for this device."));
}

void ShupitoCC25XX::writeFuses(std::vector<quint8> &/*data*/, chip_definition &/*chip*/, quint8 /*verifyMode*/)
{
    throw QString(QObject::tr("Writing fuses is not supported for this device."));
}

void ShupitoCC25XX::flashPage(chip_definition::memorydef */*memdef*/, std::vector<quint8> &memory, quint32 address)
{
    quint32 size = memory.size();

    // Arm the DMA channel
    write_sfr(0xD6, (1 << 0)); // DMARM

    // Transfer all data into the DATA memory 0--127
    Q_ASSERT(size == 128);

    ShupitoPacket pkt;

    quint8 const * mem = memory.data();
    while(size > 0)
    {
        quint8 chunk = 14;
        if (chunk > size)
            chunk = size;
        size -= chunk;

        pkt.push_back(m_prog_cmd_base + 4);
        pkt.push_back(address & 0x7F);
        pkt.insert(pkt.end(), mem, mem + chunk);
        mem += chunk;

        ShupitoPacket res = m_shupito->waitForPacket(pkt, m_prog_cmd_base + 4);
        if(res.size() != 2 || res[1] != 0)
            throw QString(QObject::tr("Failed to write data into the chip."));
    }
    address -= 128;

    // Ensure that the DMA channel is armed
    if((read_sfr(0xD6) & (1 << 0)) == 0)
        throw QString(QObject::tr("Failed to arm the DMA channel."));

    write_xdata(0x6272, address >> 10); //FADDRH
    write_xdata(0x6271, address >> 2);  //FADDRL

    write_xdata(0x6270, (1 << 2) | (1 << 1)); // FCTL

    while(read_xdata(0x6270) & 0x80);

    // Ensure that the DMA channel is not armed
    if((read_sfr(0xD6) & (1 << 0)) != 0)
        throw QString(QObject::tr("Failed to disarm the DMA channel."));

}
quint8 ShupitoCC25XX::read_xdata(quint16 addr)
{
    quint8 instr1[] = { 0x90, quint8(addr >> 8), (quint8)addr }; // mov DPTR, addr
    execute_instr(instr1);

    const quint8 instr2[] = { 0xE0 }; // movx A,@DPTR
    return execute_instr(instr2);
}

void ShupitoCC25XX::write_xdata(quint16 addr, quint8 data)
{
    quint8 instr1[] = { 0x90, quint8(addr >> 8), (quint8)addr }; // mov DPTR, addr
    execute_instr(instr1);

    quint8 load_acc_instr[] = { 0x74, data }; // mov A,#data
    execute_instr(load_acc_instr);

    const quint8 instr2[] = { 0xF0 }; // movx @DPTR,A
    execute_instr(instr2);
}

quint8 ShupitoCC25XX::read_sfr(quint8 addr)
{
    quint8 load_direct[] = { 0xE5, addr }; // mov A, addr
    return execute_instr(load_direct);
}

void ShupitoCC25XX::write_sfr(quint8 addr, quint8 data)
{
    quint8 load[] = { 0x75, addr, data }; // mov addr, #data
    execute_instr(load);
}

quint8 ShupitoCC25XX::execute_instr(const char *inst, std::size_t len)
{
    ShupitoPacket pkt;
    pkt.push_back(m_prog_cmd_base + 2);
    pkt.push_back(1);
    pkt.push_back(0x50 | quint8(len));
    pkt.insert(pkt.end(), inst, inst + len);

    pkt = m_shupito->waitForPacket(pkt, m_prog_cmd_base + 2);

    if(pkt.size() != 3 || pkt[2] != 0)
        throw QString(QObject::tr("Invalid response from the device."));

    return pkt[1];
}

ShupitoPacket ShupitoCC25XX::execute_cmd(char const *cmd, std::size_t len, quint8 read_count)
{
    ShupitoPacket pkt;
    pkt.push_back(m_prog_cmd_base + 2);
    pkt.push_back(read_count);
    pkt.insert(pkt.end(), cmd, cmd + len);

    pkt = m_shupito->waitForPacket(pkt, m_prog_cmd_base + 2);
    if(pkt.size() != size_t(read_count + 2) || pkt[1+read_count] != 0)
        throw QString(QObject::tr("Invalid response from the device."));

    return pkt;
}
