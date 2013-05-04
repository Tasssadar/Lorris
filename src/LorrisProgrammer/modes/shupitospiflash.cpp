/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "../shupito.h"
#include "shupitospiflash.h"
#include "../../shared/defmgr.h"
#include "../../misc/utils.h"

void ShupitoSpiFlash::transfer(uint8_t const * out_data, uint8_t * in_data, size_t size)
{
    size_t ms = m_shupito->maxPacketSize() - 1;

    while (size)
    {
        size_t chunk = (std::min)(ms, size);

        uint8_t flags = 0;
        if (chunk == size)
            flags |= 0x02;

        ShupitoPacket pkt;
        pkt.push_back(m_prog_cmd_base + 2);
        pkt.push_back(flags);
        pkt.insert(pkt.end(), out_data, out_data + chunk);
        pkt = m_shupito->waitForPacket(pkt, m_prog_cmd_base + 2);

        if (pkt.size() != chunk + 2)
            throw QString(tr("Invalid response."));

        if (in_data)
        {
            std::copy(pkt.begin() + 2, pkt.end(), in_data);
            in_data += chunk;
        }

        out_data += chunk;
        size -= chunk;
    }
}

void ShupitoSpiFlash::readSfdp(uint32_t addr, uint8_t * data, size_t size)
{
    std::vector<uint8_t> out(size + 5, 0);
    out[0] = 0x5a;
    out[1] = addr >> 16;
    out[2] = addr >> 8;
    out[3] = addr;

    std::vector<uint8_t> in(size + 5);
    this->transfer(out.data(), in.data(), size + 5);

    std::copy(in.begin() + 5, in.end(), data);
}

ShupitoSpiFlash::ShupitoSpiFlash(Shupito *shupito)
    : ShupitoMode(shupito)
{
}

chip_definition ShupitoSpiFlash::readDeviceId()
{
    chip_definition cd;
    cd.setName("spiflash");

    {
        uint8_t const req[] = { m_prog_cmd_base + 2, (1<<1), 0x9f, 0, 0, 0 };
        ShupitoPacket in = m_shupito->waitForPacket(ShupitoPacket(req, req + sizeof req), req[0]);

        if (in[3] == 0 && in[4] == 0 && in[5] == 0)
            throw QString(tr("No flash device detected"));

        char sign[] = "spiflash:000000";
        Utils::toBase16(sign + 9, in[3]);
        Utils::toBase16(sign + 11, in[4]);
        Utils::toBase16(sign + 13, in[5]);
        cd.setSign(sign);
    }

    uint8_t header[16];
    this->readSfdp(0, header, sizeof header);

    uint32_t flash_size_bytes = 0;
    if (header[0] == 'S' && header[1] == 'F' && header[2] == 'D' && header[3] == 'P')
    {
        uint32_t table_address = deserialize_le<uint32_t>(header + 0xc, 3);

        uint8_t flash_params[16];
        this->readSfdp(table_address, flash_params, sizeof flash_params);

        uint32_t flash_size_bits = deserialize_le<uint32_t>(flash_params + 4);
        flash_size_bytes = (flash_size_bits + 1) / 8;
    }

    QHash<QString, chip_definition::memorydef> & mds = cd.getMems();
    chip_definition::memorydef & md = mds["flash"];
    md.memid = 1;
    md.size = flash_size_bytes;
    md.pagesize = 256;

    sDefMgr.update(cd);
    return cd;
}

void ShupitoSpiFlash::readMemRange(quint8 memid, QByteArray& memory, quint32 address, quint32 size)
{
    ShupitoPacket out;
    out.push_back(m_prog_cmd_base + 2);
    out.push_back(0);
    out.push_back(3);
    out.push_back(address >> 16);
    out.push_back(address >> 8);
    out.push_back(address);

    while (size && !m_cancel_requested)
    {
        size_t offset = out.size() - 2;
        size_t chunk = (std::min)((size_t)size, m_shupito->maxPacketSize() - (out.size() - 1));
        size -= chunk;

        if (size == 0)
            out[1] = (1<<1);
        out.resize(chunk + offset + 2, 0);

        ShupitoPacket in = m_shupito->waitForPacket(out, m_prog_cmd_base + 2);
        if (in.size() != out.size())
            throw QString(tr("Invalid response."));

        memory.append((char *)in.data() + offset + 2, in.size() - offset - 2);

        out.clear();
        out.push_back(m_prog_cmd_base + 2);
        out.push_back(0);
    }
}

void ShupitoSpiFlash::flashPage(chip_definition::memorydef *memdef, std::vector<quint8>& memory, quint32 address)
{
    this->writeEnable();
    if ((this->readStatus() & (1<<1)) == 0)
        throw tr("Failed to enable write");

    std::vector<uint8_t> out;
    out.push_back(2);
    out.push_back(address >> 16);
    out.push_back(address >> 8);
    out.push_back(address);
    out.insert(out.end(), memory.begin(), memory.end());
    this->transfer(out.data(), 0, out.size());

    while (this->readStatus() & (1<<0))
    {
    }

    if ((this->readStatus() & (1<<1)) != 0)
        throw tr("The device didn't reset the write enable latch");
}

void ShupitoSpiFlash::erase_device(chip_definition& chip)
{
    this->writeEnable();
    if ((this->readStatus() & (1<<1)) == 0)
        throw tr("Failed to enable write");

    uint8_t const req[] = { m_prog_cmd_base + 2, (1<<1), 0xC7 };
    m_shupito->waitForPacket(ShupitoPacket(req, req + sizeof req), m_prog_cmd_base + 2);

    while (this->readStatus() & (1<<0))
    {
    }

    if ((this->readStatus() & (1<<1)) != 0)
        throw tr("The device didn't reset the write enable latch");
}

void ShupitoSpiFlash::writeEnable()
{
    uint8_t const req[] = { m_prog_cmd_base + 2, (1<<1), 6 };
    m_shupito->waitForPacket(ShupitoPacket(req, req + sizeof req), m_prog_cmd_base + 2);
}

uint8_t ShupitoSpiFlash::readStatus()
{
    uint8_t const req[] = { m_prog_cmd_base + 2, (1<<1), 5, 0 };
    ShupitoPacket in = m_shupito->waitForPacket(ShupitoPacket(req, req + sizeof req), m_prog_cmd_base + 2);
    if (in.size() != sizeof req)
        throw QString(tr("Invalid response.")); //XXX
    return in[3];
}

ShupitoDesc::config const * ShupitoSpiFlash::getModeCfg()
{
    return m_shupito->getDesc()->getConfig("633125ab-32e0-49ec-b240-7d845bb70b2d");
}

ProgrammerCapabilities ShupitoSpiFlash::capabilities() const
{
    ProgrammerCapabilities ps;
    ps.flash = true;
    return ps;
}
