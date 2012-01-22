#include <QObject>
#include <set>

#include "common.h"
#include "shupito.h"
#include "shupitomode.h"
#include "shupitospi.h"
#include "shared/chipdefs.h"
#include "shared/hexfile.h"

ShupitoMode::ShupitoMode(Shupito *shupito) : QObject()
{
    m_prepared = false;
    m_flash_mode = false;

    m_bsel_base = 0;
    m_bsel_min = 0;
    m_bsel_max = 0;

    m_prog_cmd_base = 0;

    m_shupito = shupito;

    m_cancel_requested = false;
}

ShupitoMode *ShupitoMode::getMode(quint8 mode, Shupito *shupito)
{
    switch(mode)
    {
        case MODE_SPI: return new ShupitoSPI(shupito);
        // TODO
        case MODE_PDI:
        case MODE_JTAG:
            return NULL;
    }
    return NULL;
}

// void switch_to_flash_mode(boost::uint32_t speed_hz), device_shupito.hpp
void ShupitoMode::switchToFlashMode(quint32 speed_hz)
{
    m_flash_mode = false;

    if(!m_prepared)
        prepare();
    m_prepared = false;

    quint32 bsel = (quint32)((m_bsel_base + (speed_hz - 1)) / speed_hz);

    bsel = std::min(bsel, (quint32)m_bsel_max);
    bsel = std::max(bsel, (quint32)m_bsel_min);

    ShupitoPacket pkt(m_prog_cmd_base, 2, (quint8)bsel, (quint8)(bsel >> 8));

    pkt = m_shupito->waitForPacket(pkt.getData(false), 1);
    if(pkt.getLen() != 1 || pkt[0] != 0)
        throw QString(QObject::tr("Failed to switch to the flash mode (error %1)")).arg((int)pkt[1]);

    m_prepared = true;
    m_flash_mode = true;
}

// void switch_to_run_mode(), device_shupito.hpp
void ShupitoMode::switchToRunMode()
{
    m_flash_mode = false;
    m_prepared = false;

    ShupitoPacket pkt(m_prog_cmd_base + 1, 0);
    pkt = m_shupito->waitForPacket(pkt.getData(false), m_prog_cmd_base + 1);

    if(pkt.getLen() < 1 || pkt[0] != 0)
        throw QString(QObject::tr("Failed to switch to the run mode"));

    m_prepared = true;
}

// void prepare(), device_shupito.hpp
// Shupito 1.0 support dropped!
void ShupitoMode::prepare()
{
    m_bsel_base = 1000000;
    m_bsel_min = 1;
    m_bsel_max = 255;

    ShupitoDesc::config *prog_cfg = getModeCfg();
    if(!prog_cfg)
        throw QString(QObject::tr("The device can't program these types of chips."));

    m_shupito->sendPacket(prog_cfg->getStateChangeCmd(true));

    if(prog_cfg->data.size() == 9 && prog_cfg->data[0] == 1)
    {
        m_bsel_base = 0;

        for (qint8 i = 4; i != 0; --i)
            m_bsel_base = (m_bsel_base << 8) | prog_cfg->data[i];

        m_bsel_min = prog_cfg->data[5] | (prog_cfg->data[6] << 8);
        m_bsel_max = prog_cfg->data[7] | (prog_cfg->data[8] << 8);
    }

    m_prog_cmd_base = prog_cfg->cmd;
    m_prepared = true;
}

ShupitoDesc::config *ShupitoMode::getModeCfg()
{
    return NULL;
}

// std::string read_device_id(), device_shupito.hpp
QString ShupitoMode::readDeviceId()
{
    m_prepared = false;

    ShupitoPacket pkt(m_prog_cmd_base + 2, 0x00);
    pkt = m_shupito->waitForPacket(pkt.getData(false), m_prog_cmd_base + 2);

    if(pkt.getLen() == 0 || pkt[pkt.getLen()-1] != 0)
        throw QString(QObject::tr("Failed to read the chip's signature (error %1).")).arg((int)pkt[pkt.getLen()-1]);

    quint8 id_lenght = (pkt.getLen() - 1 );
    QString id("");
    this->editIdArgs(id, id_lenght);

    quint8 *p_data = new quint8[id_lenght];
    for(quint8 i = 0; i < id_lenght; ++i)
        p_data[i] = (quint8)pkt[i];

    id += Utils::toBase16(p_data, p_data + id_lenght);

    delete[] p_data;

    m_prepared = true;
    return id;
}

void ShupitoMode::editIdArgs(QString &id, quint8 &/*id_lenght*/)
{
    id = "unk:";
}

// virtual void read_memory(std::ostream & s, std::string const & memid, avrflash::chip_definition const & chip)
// device.hpp
QByteArray ShupitoMode::readMemory(const QString &mem, chip_definition &chip)
{
    m_cancel_requested = false;

    chip_definition::memorydef const *memdef = chip.getMemDef(mem);

    if(!memdef)
        throw QString(QObject::tr("Unknown memory id"));

    QByteArray res;
    quint32 len = memdef->size;
    quint32 offset = 0;
    while(len && !m_cancel_requested)
    {
        quint32 chunk = std::min(len, (quint32)1024);

        readMemRange(memdef->memid, res, offset, chunk);

        len -= chunk;
        offset += chunk;

        emit updateProgressDialog((offset*100)/memdef->size);
    }
    if(m_cancel_requested)
    {
        emit updateProgressDialog(-1);
        m_cancel_requested = false;
        res.append(QByteArray(memdef->size - res.size(), 0xFF));
    }
    return res;
}
// void read_memory_range(int memid, unsigned char * memory, size_t address, size_t size)
// device_shupito.hpp
void ShupitoMode::readMemRange(quint8 memid, QByteArray& memory, quint32 address, quint32 size)
{
    Q_ASSERT(size < 65536);

    ShupitoPacket pkt(4, 7, memid,
                     (quint8)address, (quint8)(address >> 8), (quint8)(address >> 16), (quint8)(address >> 24),
                     (quint8)size, (quint8)(size >> 8));

    QByteArray p = m_shupito->waitForStream(pkt.getData(false), 4);
    if((quint32)p.size() != size)
        throw QString(QObject::tr("The read returned wrong-sized stream."));

    memory.append(p);
}

void ShupitoMode::cancelRequested()
{
    m_cancel_requested = true;
}

//void erase_device(avrflash::chip_definition const & chip), device_shupito.hpp
void ShupitoMode::erase_device()
{
    m_prepared = false;
    m_flash_mode = false;

    ShupitoPacket pkt(m_prog_cmd_base + 4, 0);
    pkt = m_shupito->waitForPacket(pkt.getData(false), m_prog_cmd_base + 4);

    if(pkt.getLen() != 1 || pkt[0] != 0)
        throw QString(QObject::tr("Failed to erase chip's memory"));

    m_flash_mode = true;
    m_prepared = true;

    // This should really be on the device side...
    Utils::msleep(100);
}

// void read_fuses(std::vector<boost::uint8_t> & data, avrflash::chip_definition const & chip)
// device_shupito.hpp
void ShupitoMode::readFuses(std::vector<quint8> &data, chip_definition &chip)
{
    chip_definition::memorydef *memdef = chip.getMemDef("fuses");
    if(!memdef)
        throw QString(QObject::tr("Can't read fuses"));

    data.resize(memdef->size);
    QByteArray memory;

    if(memdef->size != 0)
        readMemRange(memdef->memid, memory, 0, memdef->size);

    for(quint16 i = 0; i < memory.size(); ++i)
        data[i] = (quint8)memory[i];
}

//void write_fuses(std::vector<boost::uint8_t> const & data, avrflash::chip_definition const & chip, bool verify)
//device.hpp
void ShupitoMode::writeFuses(std::vector<quint8> &data, chip_definition &chip, bool verify)
{
    HexFile file;
    file.addRegion(0, &data[0], &data[0] + data.size(), 0);
    flashRaw(file, MEM_FUSES, chip, verify);
}

//void flash_raw(avrflash::memory const & mem, std::string const & memid, avrflash::chip_definition const & chip, bool verify)
//device.hpp
void ShupitoMode::flashRaw(HexFile& file, quint8 memId, chip_definition& chip, bool verify)
{
    m_cancel_requested = false;

    chip_definition::memorydef *memdef = chip.getMemDef(memId);
    if(!memdef)
        throw QString(QObject::tr("Chip does not have mem id %1")).arg(memId);

    std::vector<page> pages;
    file.makePages(pages, memId, chip);

    quint32 countNoSkipped = 0;
    quint32 flashedCount = 0;
    std::set<quint32> skipped;
    for(quint32 i = 0; !m_cancel_requested && i < pages.size(); ++i)
    {
        if(!skipPage(memId, pages[i].data))
            ++countNoSkipped;
        else
            skipped.insert(i);
    }

    prepareMemForWriting(memdef, chip);

    for(quint32 i = 0; !m_cancel_requested && i < pages.size(); ++i)
    {
        if(skipped.find(i) != skipped.end())
            continue;

        flashPage(memdef, pages[i].data, pages[i].address);

        int pct = (++flashedCount)*100/countNoSkipped;
        if(pct == 100)
            --pct;
        emit updateProgressDialog(pct);
    }

    if(m_cancel_requested)
    {
        m_cancel_requested = false;
        throw QString(QObject::tr("Flashing interruped!"));
    }

    if(verify && is_read_memory_supported(memdef))
    {
        emit updateProgressLabel(QObject::tr("Verifying data"));
        QByteArray buff;
        for(quint32 i = 0; !m_cancel_requested && i < pages.size(); ++i)
        {
            buff.clear();
            readMemRange(memId, buff, pages[i].address, pages[i].data.size());

            bool wrong = false;
            if((uint)buff.size() != pages[i].data.size())
                wrong = true;
            else
            {
                for(quint32 x = 0; !wrong && x < pages[i].data.size(); ++x)
                    if(pages[i].data[x] != (quint8)buff[x])
                        wrong = true;
            }
            if(wrong)
                throw QString(QObject::tr("Verification failed!"));

            emit updateProgressDialog((i*100)/pages.size());
        }

        if(m_cancel_requested)
        {
            m_cancel_requested = false;
            throw QString(QObject::tr("Flashing interruped!"));
        }
    }
}

//void prepare_memory_for_writing(chip_definition::memorydef const * memdef, avrflash::chip_definition const & chip)
//device_shupito.hpp
void ShupitoMode::prepareMemForWriting(chip_definition::memorydef *memdef, chip_definition& /*chip*/)
{
    m_prepared = false;
    m_flash_mode = false;

    ShupitoPacket pkt(m_prog_cmd_base + 4, 0x01, memdef->memid);
    pkt = m_shupito->waitForPacket(pkt.getData(false), m_prog_cmd_base + 4);

    if(pkt.getLen() != 1 || pkt[0] != 0)
        throw QString(QObject::tr("Failed to prepare chip's memory for writing"));

    m_flash_mode = true;
    m_prepared = true;
}

//void flash_page(chip_definition::memorydef const * memdef, const unsigned char * memory, size_t address, size_t size)
//device_shupito.hpp
void ShupitoMode::flashPage(chip_definition::memorydef *memdef, std::vector<quint8>& memory, quint32 address)
{
    m_prepared = false;
    m_flash_mode = false;

    quint32 size = memory.size();
    // Prepare
    {
        ShupitoPacket pkt(m_prog_cmd_base + 5, 0x05, memdef->memid,
                          (quint8)address, (quint8)(address >> 8),
                          (quint8)(address >> 16), (quint8)(address >> 24));
        pkt = m_shupito->waitForPacket(pkt.getData(false), m_prog_cmd_base + 5);
        if(pkt.getLen() != 1 || pkt[0] != 0)
            throw QString(QObject::tr("Failed to flash a page"));
    }

    //send data
    {
        ShupitoPacket res;
        QByteArray pkt;
        pkt[0] = 0x80;
        pkt[2] = memdef->memid;

        quint32 itr = 0;

        while(size > 0)
        {
            quint32 chunk = size > 14 ? 14 : size;
            pkt[1] = ((m_prog_cmd_base + 6) << 4) | (chunk + 1);
            pkt.resize(3 + chunk);
            for(quint8 i = 0; i < chunk; ++i)
                pkt[i+3] = memory[itr++];
            size -= chunk;

            res = m_shupito->waitForPacket(pkt, m_prog_cmd_base + 6);
            if(res.getLen() != 1 || res[0] != 0)
                throw QString(QObject::tr("Failed to flash a page"));
        }
    }

    // "seal"
    {
        ShupitoPacket pkt(m_prog_cmd_base + 7, 0x05, memdef->memid,
                          (quint8)address, (quint8)(address >> 8),
                          (quint8)(address >> 16), (quint8)(address >> 24));
        pkt = m_shupito->waitForPacket(pkt.getData(false), m_prog_cmd_base + 7);
        if(pkt.getLen() != 1 || pkt[0] != 0)
            throw QString(QObject::tr("Failed to flash a page"));
    }

    m_flash_mode = true;
    m_prepared = true;
}

bool ShupitoMode::skipPage(quint8 memId, std::vector<quint8> &pageData)
{
    if(memId != MEM_FLASH)
        return false;

    bool skip = true;
    for(quint32 x = 0; skip && x < pageData.size(); ++x)
        if(pageData[x] != 0xFF)
            skip = false;
    return skip;
}
