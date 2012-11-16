#include "flipprogrammer.h"
#include "../../shared/defmgr.h"
#include <libyb/async/sync_runner.hpp>

/*    FLASH
    EEPROM
    SECURITY
    CONFIGURATION
    BOOTLOADER
    SIGNATURE
    USER
    INT_RAM*/

FlipProgrammer::FlipProgrammer(ConnectionPointer<FlipConnection> const & conn)
    : m_conn(conn)
{
    m_flip.open(m_conn->device());
}

void FlipProgrammer::stopAll(bool wait)
{
}

void FlipProgrammer::switchToFlashMode(quint32 prog_speed_hz)
{
    try_run(m_flip.clear_errors());
}

void FlipProgrammer::switchToRunMode()
{
    try_run(m_flip.start_application());
}

bool FlipProgrammer::isInFlashMode()
{
    return true;
}

chip_definition FlipProgrammer::readDeviceId()
{
    uint8_t buf[3];
    if (try_run(m_flip.read_memory(5, 0, buf, sizeof buf)).has_exception())
        return chip_definition();

    QString chipid = QString("flip:%1%2%3")
        .arg((int)buf[0], 2, 16, QChar('0'))
        .arg((int)buf[1], 2, 16, QChar('0'))
        .arg((int)buf[2], 2, 16, QChar('0'));

    return sDefMgr.findChipdef(chipid);
}

QByteArray FlipProgrammer::readMemory(const QString& mem, chip_definition &chip)
{
    QByteArray res;

    yb::flip2::memory_id_t memid;
    if (mem == "flash")
        memid = 0;
    else if (mem == "eeprom")
        memid = 1;
    else if (mem == "sig")
        memid = 5;
    else
        return res;

    chip_definition::memorydef * md = chip.getMemDef(mem);
    res.resize(md->size);

    if (try_run(m_flip.read_memory(memid, 0, (uint8_t *)res.data(), res.size())).has_exception())
        res.clear();
    return res;
}

void FlipProgrammer::readMemRange(quint8 memid, QByteArray& memory, quint32 address, quint32 size)
{
}

void FlipProgrammer::readFuses(std::vector<quint8>&, chip_definition &)
{
    // The fuse reading/writing is not available in DFU.
}

void FlipProgrammer::writeFuses(std::vector<quint8>&, chip_definition &, quint8)
{
    // The fuse reading/writing is not available in DFU.
}

void FlipProgrammer::flashRaw(HexFile& file, quint8 memId, chip_definition& chip, quint8 verifyMode)
{
    std::vector<page> pages;
    file.makePages(pages, memId, chip, 0);

    for (size_t i = 0; i < pages.size(); ++i)
        try_run(m_flip.write_memory(memId - 1, pages[i].address, pages[i].data.data(), pages[i].data.size()));
}

void FlipProgrammer::erase_device(chip_definition& chip)
{
    try_run(m_flip.chip_erase());
}
