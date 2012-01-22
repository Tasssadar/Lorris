#include <QObject>

#include "common.h"
#include "shupito.h"
#include "shupitomode.h"
#include "shupitospi.h"
#include "chipdefs.h"

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
