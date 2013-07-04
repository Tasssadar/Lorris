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

#include <QApplication>

#include "stm32programmer.h"
#include "../../connection/stm32defines.h"
#include "../../misc/utils.h"
#include "../../shared/defmgr.h"

#include <libyb/usb/usb_descriptors.hpp>

STM32Programmer::STM32Programmer(const ConnectionPointer<STM32Connection> &conn, ProgrammerLogSink *logsink) :
    Programmer(logsink), m_conn(conn)
{

}

STM32Programmer::~STM32Programmer()
{

}

int STM32Programmer::getType()
{
    return programmer_stm32;
}

void STM32Programmer::stopAll(bool /*wait*/)
{
}

void STM32Programmer::switchToFlashMode(quint32 /*prog_speed_hz*/)
{
    if(m_conn->c_current_mode() == STLINK_DEV_DFU_MODE)
        m_conn->c_exit_dfu_mode();

    if(m_conn->c_current_mode() != STLINK_DEV_DEBUG_MODE)
        m_conn->c_enter_swd_mode();

    m_conn->c_reset();
    m_conn->c_version();
}

void STM32Programmer::switchToRunMode()
{
    m_conn->c_run();
    m_conn->c_force_reset();
    m_conn->c_exit_debug_mode();
}

bool STM32Programmer::isInFlashMode()
{
    return m_conn->c_current_mode() == STLINK_DEV_DEBUG_MODE;
}

uint32_t STM32Programmer::readChipId()
{
    uint32_t chip_id = m_conn->c_read_debug32(0xE0042000);
    if (chip_id == 0) //Try Corex M0 DBGMCU_IDCODE register address
        chip_id = m_conn->c_read_debug32(0x40015800);
    chip_id &= 0xFFF;

    /* Fix chip_id for F4 rev A errata , Read CPU ID, as CoreID is the same for F2/F4*/
    if (chip_id == 0x411) {
        uint32_t cpuid = m_conn->c_read_debug32(0xE000ED00);
        if ((cpuid  & 0xfff0) == 0xc240)
            chip_id = 0x413;
    }
    return chip_id;
}

chip_definition STM32Programmer::readDeviceId()
{
    uint32_t core_id = m_conn->c_core_id();
    uint32_t chip_id = readChipId();

    QString sign = QString("stm32:%1-%2").arg(core_id, 0, 16).arg(chip_id, 0, 16);
    chip_definition def = sDefMgr.findChipdef(sign);
    if(def.getName().isEmpty())
        return def;

    // Try to read flash size from the chip
    chip_definition::memorydef *flash = def.getMemDef(MEM_FLASH);
    if(flash->size == 0)
    {
        bool ok;
        uint32_t reg = def.getOptionUInt("flash_size_reg", &ok);
        if(!ok)
            throw tr("Invalid chip definition %1!").arg(sign);

        flash->size = (m_conn->c_read_debug32(reg) & 0xFFFF) * 1024;
    }

    return def;
}

QByteArray STM32Programmer::readMemory(const QString& mem, chip_definition &chip)
{
    if(mem != "flash")
        throw tr("Unsupported memory type");

    size_t addr = STM32_FLASH_BASE;
    size_t size = chip.getMemDef(MEM_FLASH)->size;

    QByteArray res;
    res.reserve(size);

    emit updateProgressDialog(0);

    for(size_t off = 0; off < size; off += 1024)
    {
        size_t read_size = 1024;
        size_t rounded_size;
        if ((off + read_size) > size)
            read_size = size - off;

        /* round size if needed */
        rounded_size = read_size;
        if (rounded_size & 3)
            rounded_size = (rounded_size + 4) & ~(3);

        res.append(m_conn->c_read_mem32(addr + off, rounded_size));
        emit updateProgressDialog((off*100)/size);
    }
    return res;
}

void STM32Programmer::readFuses(std::vector<quint8>&, chip_definition &)
{
    // Not available
}

void STM32Programmer::writeFuses(std::vector<quint8>&, chip_definition &, VerifyMode)
{
    // Not available
}

void STM32Programmer::flashRaw(HexFile& file, quint8 memId, chip_definition& chip, VerifyMode verifyMode)
{
    if(memId != MEM_FLASH)
        throw tr("Unsupported memory type");

    const char erased_pattern = chip.getOption("erased_pattern_zeros") == "true" ? 0 : 0xFF;
    const uint32_t addr = STM32_FLASH_BASE;
    QByteArray data = file.getDataArray(file.getTopAddress());
    for(int i = data.size()-1; i >= 0; --i)
    {
        if(data[i] != erased_pattern)
        {
            data.chop(data.size()-i-1);
            break;
        }
    }

    chip_definition::memorydef *flash_mem = chip.getMemDef(MEM_FLASH);

    flash_ptr flash(STM32FlashController::getController(chip.getOption("flash_controller"), m_conn));
    connect(flash.data(), SIGNAL(updateProgressDialog(int)), SIGNAL(updateProgressDialog(int)));

    emit updateProgressLabel(tr("Waiting for flash operations to finish..."));
    while(flash->is_busy())
    {
        emit updateProgressDialog(0);
        Utils::msleep(50);
    }

    // Erase affected pages
    emit updateProgressLabel(tr("Erasing flash pages..."));
    emit updateProgressDialog(0);
    for(uint32_t off = 0; off < (uint32_t)data.size(); off += flash_mem->pagesize)
    {
        flash->unlock();
        flash->erase_page(addr+off);
        do {
            emit updateProgressDialog((off*100)/data.size());
        } while(flash->is_busy());
        flash->lock();
    }

    // Write
    emit updateProgressLabel(tr("Writing data..."));
    emit updateProgressDialog(0);

    flash->write(chip, addr, data.data(), data.size());

    m_conn->c_write_reg(m_conn->c_read_debug32(addr), 13);   // Stack
    m_conn->c_write_reg(m_conn->c_read_debug32(addr+4), 15); // PC

    // Verify
    if(verifyMode == VERIFY_ONLY_NON_EMPTY || verifyMode == VERIFY_ALL_PAGES)
    {
        emit updateProgressLabel(tr("Verifying data..."));
        emit updateProgressDialog(0);

        QByteArray mem;
        int cmp, aligned;
        int block_size = flash_mem->pagesize > 0x1800 ? 0x1800 : flash_mem->pagesize;
        for(int off = 0; off < data.size(); off += cmp)
        {
            cmp = (std::min)(block_size, data.size() - off);
            aligned = cmp;
            if(aligned & (4 - 1))
                aligned = (cmp + 4) & ~(4 - 1);

            mem = m_conn->c_read_mem32(addr + off, aligned);
            if(memcmp(data.data()+off, mem.data(), cmp) != 0)
                throw tr("Verification failed at offset 0x%1!").arg(off, 0, 16);

            emit updateProgressDialog((off*100)/data.size());
        }
    }
}

void STM32Programmer::erase_device(chip_definition& chip)
{
    flash_ptr flash(STM32FlashController::getController(chip.getOption("flash_controller"), m_conn));

    if(flash->supports_mass_erase())
    {
        emit updateProgressLabel(tr("Waiting for flash operations to finish..."));
        while(flash->is_busy())
        {
            emit updateProgressDialog(0);
            Utils::msleep(50);
        }

        flash->unlock();
        flash->erase_mass();

        emit updateProgressLabel(tr("Erasing memory.."));
        while(flash->is_busy())
        {
            emit updateProgressDialog(50);
            Utils::msleep(50);
        }

        flash->lock();
        //FIXME: verify?
    }
    else
    {
        throw tr("This flash does not support mass erase and individual page erase is not yet implemented");
    }
}

ProgrammerCapabilities STM32Programmer::capabilities() const
{
    ProgrammerCapabilities sources;
    sources.flash = true;
    return sources;
}

//----------------------------------------------------------------------------

STM32FlashController *STM32FlashController::getController(const QString& name, const ConnectionPointer<STM32Connection> &conn)
{
    if(name.isEmpty())
        throw STM32Programmer::tr("Flash controller for this chip was not specified.");

    if(name == "stm32vl")
        return new STM32VLFlash(conn);

    throw STM32Programmer::tr("Unknown flash controller (\"%1\")").arg(name);
}

STM32FlashController::STM32FlashController(ConnectionPointer<STM32Connection> const & conn, QObject *parent) :
    QObject(parent), m_conn(conn)
{

}

STM32FlashController::~STM32FlashController()
{

}

/* stm32f FPEC flash controller interface, pm0063 manual */
// STM32F05x is identical, based on RM0091 (DM00031936, Doc ID 018940 Rev 2, August 2012)
#define FLASH_REGS_ADDR 0x40022000

#define FLASH_ACR (FLASH_REGS_ADDR + 0x00)
#define FLASH_KEYR (FLASH_REGS_ADDR + 0x04)
#define FLASH_SR (FLASH_REGS_ADDR + 0x0c)
#define FLASH_CR (FLASH_REGS_ADDR + 0x10)
#define FLASH_AR (FLASH_REGS_ADDR + 0x14)
#define FLASH_OBR (FLASH_REGS_ADDR + 0x1c)
#define FLASH_WRPR (FLASH_REGS_ADDR + 0x20)

#define FLASH_SR_BSY 0
#define FLASH_SR_EOP 5

#define FLASH_CR_PG 0
#define FLASH_CR_PER 1
#define FLASH_CR_MER 2
#define FLASH_CR_STRT 6
#define FLASH_CR_LOCK 7

// For STM32F05x, the RDPTR_KEY may be wrong, but as it is not used anywhere...
#define FLASH_RDPTR_KEY 0x00a5
#define FLASH_KEY1 0x45670123
#define FLASH_KEY2 0xcdef89ab

/* from openocd, contrib/loaders/flash/stm32.s */
static const uint8_t loader_code_stm32vl[] = {
    0x08, 0x4c, /* ldr	r4, STM32_FLASH_BASE */
    0x1c, 0x44, /* add	r4, r3 */
    /* write_half_word: */
    0x01, 0x23, /* movs	r3, #0x01 */
    0x23, 0x61, /* str	r3, [r4, #STM32_FLASH_CR_OFFSET] */
    0x30, 0xf8, 0x02, 0x3b, /* ldrh	r3, [r0], #0x02 */
    0x21, 0xf8, 0x02, 0x3b, /* strh	r3, [r1], #0x02 */
    /* busy: */
    0xe3, 0x68, /* ldr	r3, [r4, #STM32_FLASH_SR_OFFSET] */
    0x13, 0xf0, 0x01, 0x0f, /* tst	r3, #0x01 */
    0xfb, 0xd0, /* beq	busy */
    0x13, 0xf0, 0x14, 0x0f, /* tst	r3, #0x14 */
    0x01, 0xd1, /* bne	exit */
    0x01, 0x3a, /* subs	r2, r2, #0x01 */
    0xf0, 0xd1, /* bne	write_half_word */
    /* exit: */
    0x00, 0xbe, /* bkpt	#0x00 */
    0x00, 0x20, 0x02, 0x40, /* STM32_FLASH_BASE: .word 0x40022000 */
};

STM32VLFlash::STM32VLFlash(const ConnectionPointer<STM32Connection> &conn) : STM32FlashController(conn)
{
    m_lock_on_destroy = false;
}

STM32VLFlash::~STM32VLFlash()
{
    if(m_lock_on_destroy)
        lock();
}

uint32_t STM32VLFlash::read_sr()
{
    return m_conn->c_read_debug32(FLASH_SR);
}

uint32_t STM32VLFlash::read_cr()
{
    return m_conn->c_read_debug32(FLASH_CR);
}

void STM32VLFlash::add_cr_bit(uint8_t bit)
{
    m_conn->c_write_debug32(FLASH_CR, (this->read_cr() | (1 << bit)));
}

void STM32VLFlash::set_cr_bit(uint8_t bit)
{
    m_conn->c_write_debug32(FLASH_CR, (1 << bit));
}

void STM32VLFlash::erase_mass()
{
    /* set the mass erase bit */
    add_cr_bit(FLASH_CR_MER);

    /* start erase operation, reset by hw with bsy bit */
    add_cr_bit(FLASH_CR_STRT);
}

bool STM32VLFlash::is_busy()
{
    return (this->read_sr() & (1 << FLASH_SR_BSY));
}

void STM32VLFlash::unlock(bool guard)
{
    if(!is_locked())
        return;

    /* the unlock sequence consists of 2 write cycles where
       2 key values are written to the FLASH_KEYR register.
       an invalid sequence results in a definitive lock of
       the FPEC block until next reset.
     */
    m_conn->c_write_debug32(FLASH_KEYR, FLASH_KEY1);
    m_conn->c_write_debug32(FLASH_KEYR, FLASH_KEY2);

    if(is_locked())
        throw STM32Programmer::tr("Failed to unlock flash memory, try to reset the device!");

    if(guard)
        m_lock_on_destroy = true;
}

void STM32VLFlash::lock()
{
    /* write to 1 only. reset by hw at unlock sequence */
    add_cr_bit(FLASH_CR_LOCK);

    m_lock_on_destroy = false;
}

bool STM32VLFlash::is_locked()
{
    return (this->read_cr() & (1 << FLASH_CR_LOCK));
}

void STM32VLFlash::erase_page(uint32_t addr)
{
    set_cr_bit(FLASH_CR_PER);
    m_conn->c_write_debug32(FLASH_AR, addr);
    add_cr_bit(FLASH_CR_STRT);
}

void STM32VLFlash::write(chip_definition& chip, uint32_t addr, const char *data, int size)
{
    flash_loader loader;
    init_flash_loader(loader);

    int write_len = 0;
    int page_size = chip.getMemDef(MEM_FLASH)->pagesize;

    uint8_t *data_itr = (uint8_t*)data;
    for(int off = 0; off < size;)
    {
        write_len = (std::min)(page_size, size - off);

        unlock();
        set_cr_bit(FLASH_CR_PG);

        write_data_for_loader(loader, data_itr, write_len);
        run_flash_loader(loader, addr, write_len);

        lock();

        off += write_len;
        addr += write_len;
        data_itr += write_len;

        emit updateProgressDialog((off*100)/size);
    }
}

void STM32VLFlash::init_flash_loader(flash_loader &loader)
{
    loader.addr = STM32_SRAM_BASE;
    loader.buff_addr = loader.addr + sizeof(loader_code_stm32vl);
    m_conn->c_write_mem32(loader.addr, loader_code_stm32vl, sizeof(loader_code_stm32vl));
}

void STM32VLFlash::run_flash_loader(const flash_loader &loader, uint32_t target, int size)
{
    // setup the core
    size_t count = size / sizeof(uint16_t);
    if (size % sizeof(uint16_t))
        ++count;

    // fill registers for loader
    m_conn->c_write_reg(loader.buff_addr, 0); // source
    m_conn->c_write_reg(target, 1);           // target
    m_conn->c_write_reg(count, 2);            // count in 16 bit half words
    m_conn->c_write_reg(0, 3);                // flash bank 0 (input)
    m_conn->c_write_reg(loader.addr, 15);     // PC

    // run the loader
    m_conn->c_run();

    // wait until it is done (reaches breakpoint)
    int i;
    const int rounds = 10000;
    for(i = 0; i < rounds; ++i)
    {
        QApplication::processEvents();
        if(m_conn->is_core_halted())
            break;
    }

    if(i >= rounds)
        throw tr("Flash loader didn't finish in time!");

    // Check written count
    uint32_t reg = m_conn->c_read_reg(2);
    if(reg != 0)
        throw tr("Flash loader write error (count: %1)").arg(reg);
}

void STM32VLFlash::write_data_for_loader(const flash_loader& loader, const uint8_t *data, int len)
{
    size_t chunk = len & ~0x03;
    size_t rem = len & 0x03;

    if(chunk)
        m_conn->c_write_mem32(loader.buff_addr, data, chunk);

    if(rem)
        m_conn->c_write_mem8(loader.buff_addr+chunk, data+chunk, rem);
}
