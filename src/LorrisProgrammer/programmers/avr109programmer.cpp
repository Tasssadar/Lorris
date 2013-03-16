/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QTimer>
#include <QEventLoop>

#include "avr109programmer.h"
#include "../../shared/defmgr.h"
#include "../../misc/config.h"
#include "../../misc/utils.h"
#include "../../shared/hexfile.h"

static const char SUPPORTED_DEVS = 't';
static const char HAS_BLOCK = 'b';
static const char BOOTLOADER_EXIT = 'E';
static const char ID_REQ = 's';
static const char HAS_AUTOINCREMENT = 'a';
static const char SET_ADDR = 'A';
static const char SET_ADDR_BIG = 'H';
static const char READ_FLASH = 'R';
static const char WRITE_FLASH_LOW = 'c';
static const char WRITE_FLASH_HIGH = 'C';
static const char READ_EEPROM = 'd';
static const char WRITE_EEPROM = 'D';
static const char READ_BLOCK = 'g';
static const char BLOCK_FLASH = 'F';
static const char BLOCK_EEPROM = 'E';
static const char ERASE_CHIP = 'e';
static const char WRITE_PAGE = 'm';
static const char FLASH_BLOCK = 'B';

avr109Programmer::avr109Programmer(const ConnectionPointer<PortConnection> &conn, ProgrammerLogSink *logsink) :
    Programmer(logsink)
{
    m_conn = conn;
    m_bootseq = sConfig.get(CFG_STRING_AVR109_BOOTSEQ);
    m_wait_act = WAIT_NONE;
    m_flash_mode = false;

    connect(m_conn.data(), SIGNAL(dataRead(QByteArray)), this, SLOT(dataRead(QByteArray)));
}

int avr109Programmer::getType()
{
    return programmer_avr109;
}

QString avr109Programmer::getBootseq() const
{
    return m_bootseq;
}

void avr109Programmer::setBootseq(const QString &seq)
{
    m_bootseq = seq;
    sConfig.set(CFG_STRING_AVR109_BOOTSEQ, seq);
}

void avr109Programmer::stopAll(bool /*wait*/)
{
}

void avr109Programmer::switchToFlashMode(quint32 /*prog_speed_hz*/)
{
    QByteArray bootseq = Utils::convertByteStr(m_bootseq);
    if(bootseq.isEmpty())
        throw tr("Empty bootloader sequence!");

    m_conn->SendData(bootseq);

    waitForAct(WAIT_EMPTY, 100);

    m_conn->SendData(QByteArray::fromRawData(&SUPPORTED_DEVS, 1));

    if(!waitForAct(WAIT_SUPPORTED))
        throw tr("Failed to switch to flash mode (timeout).");

    m_flash_mode = true;
}

void avr109Programmer::switchToRunMode()
{
    m_conn->SendData(QByteArray::fromRawData(&BOOTLOADER_EXIT, 1));
    m_flash_mode = false;
}

bool avr109Programmer::isInFlashMode()
{
    return m_flash_mode;
}

chip_definition avr109Programmer::readDeviceId()
{
    m_rec_buff.clear();
    m_conn->SendData(QByteArray::fromRawData(&ID_REQ, 1));

    waitForAct(WAIT_CHAR3);

    if(m_rec_buff.size() != 3)
        throw tr("Failed to read device id (timeout)");

    QString id = "avr:";
    for(int i = m_rec_buff.size(); i > 0; )
    {
        static const char* hex = "0123456789abcdef";

        quint8 c = m_rec_buff[--i];
        id.append(hex[c >> 4]);
        id.append(hex[c & 0x0F]);
    }

    return sDefMgr.findChipdef(id);
}

QByteArray avr109Programmer::readMemory(const QString& mem, chip_definition &chip)
{
    if(mem != "flash" && mem != "eeprom")
        throw tr("Unsupported memory type: %1").arg(mem);

    quint32 size = chip.getMemDef(mem)->size;
    quint8 id = chip_definition::memNameToId(mem);
    return readMem(id, 0, size);
}

void avr109Programmer::readFuses(std::vector<quint8>&, chip_definition &)
{
    // NYI
    throw tr("Fuses are not yet implemented");
}

void avr109Programmer::writeFuses(std::vector<quint8>&, chip_definition &, VerifyMode)
{
    // NYI
    throw tr("Fuses are not yet implemented");
}

void avr109Programmer::flashRaw(HexFile& file, quint8 memId, chip_definition& chip, VerifyMode verifyMode)
{
    if(memId != MEM_FLASH && memId != MEM_EEPROM)
        throw tr("Unsupported memory type: %1").arg(memId);

    int block_size = 0;

    bool has_block = checkBlockSupport(block_size);
    bool autoincrement = checkAutoIncrement();

    std::vector<page> pages;
    std::set<quint32> skip;

    file.makePages(pages, memId, chip, &skip);

    switch(memId)
    {
        case MEM_FLASH:
            // avr109 needs to erase chip before flashing!
            erase_device(chip);

            if(has_block)
                writeFlashMemBlock(pages, skip);
            else
                writeFlashMem(pages, skip, autoincrement);
            break;
        case MEM_EEPROM:
            if(has_block)
                writeEEPROMBlock(pages);
            else
                writeEEPROM(pages, autoincrement);
            break;
    }

    if(verifyMode == VERIFY_NONE)
        return;

    emit updateProgressLabel(QObject::tr("Verifying data"));

    if(verifyMode == VERIFY_ALL_PAGES || memId == MEM_EEPROM)
        skip.clear();

    quint32 verCnt = pages.size() - skip.size();

    QByteArray block;
    for(size_t i = 0; i < pages.size() && !m_cancel_requested; ++i)
    {
        if(skip.find(i) != skip.end())
            continue;

        const page& p = pages[i];

        block.clear();

        try {
            block = readMem(memId, p.address, p.data.size());
        } catch(QString) {}

        if (block.size() != p.data.size() ||
            !std::equal(p.data.data(), p.data.data()+p.data.size(), (quint8*)block.data()))
        {
            throw tr("Verification failed!");
        }

        emit updateProgressDialog((i*100)/verCnt);
    }
}

void avr109Programmer::erase_device(chip_definition& /*chip*/)
{
    m_conn->SendData(QByteArray::fromRawData(&ERASE_CHIP, 1));
    waitForAct(WAIT_CHAR1);
    if(m_rec_buff.size() != 1 || m_rec_buff[0] != '\r')
        throw tr("Failed to erase chip!");
}

void avr109Programmer::cancelRequested()
{
    m_cancel_requested = true;
}

bool avr109Programmer::checkBlockSupport(int& block_size)
{
    m_conn->SendData(QByteArray::fromRawData(&HAS_BLOCK, 1));
    waitForAct(WAIT_CHAR3);
    if(m_rec_buff.size() != 3)
        throw tr("Failed to check for block support");

    block_size = quint8(m_rec_buff[1]) << 8;
    block_size |= quint8(m_rec_buff[2]);

    return (m_rec_buff[0] == 'Y');
}

bool avr109Programmer::checkAutoIncrement()
{
    m_conn->SendData(QByteArray::fromRawData(&HAS_AUTOINCREMENT, 1));
    waitForAct(WAIT_CHAR1);
    if(m_rec_buff.size() != 1)
        throw tr("Failed to check for autoincrement support");

    return (m_rec_buff[0] == 'Y');
}

void avr109Programmer::setAddress(quint32 address)
{
    QByteArray cmd;
    if(address < 0x10000)
    {
        cmd[0] = SET_ADDR;
        cmd[1] = (address >> 8) & 0xFF;
        cmd[2] = address & 0xFF;
    }
    else
    {
        cmd[0] = SET_ADDR_BIG;
        cmd[1] = (address >> 16) & 0xFF;
        cmd[2] = (address >> 8) & 0xFF;
        cmd[3] = address & 0xFF;
    }

    m_conn->SendData(cmd);

    waitForAct(WAIT_CHAR1);
    if(m_rec_buff.size() != 1 || m_rec_buff[0] != '\r')
        throw tr("Could not set address!");
}

QByteArray avr109Programmer::readMem(quint8 id, quint32 start, quint32 size)
{
    int block_size = 0;
    bool has_block = checkBlockSupport(block_size);
    bool autoincrement = checkAutoIncrement();

    switch(id)
    {
        case MEM_FLASH:
            if(has_block)
                return readFlashMemBlock(start, size, block_size);
            else
                return readFlashMem(start, size, autoincrement);
        case MEM_EEPROM:
            if(has_block)
                return readEEPROMBlock(start, size, block_size);
            else
                return readEEPROM(start, size, autoincrement);
    }
    return QByteArray();
}

QByteArray avr109Programmer::readFlashMem(quint32 start, quint32 size, bool autoincrement)
{
    Q_ASSERT((start % 2) == 0);

    quint32 address = start;
    if(autoincrement)
        setAddress(address >> 1);

    QByteArray res;
    res.reserve(size);

    QByteArray cmd = QByteArray::fromRawData(&READ_FLASH, 1);

    m_cancel_requested = false;
    while(address < size && !m_cancel_requested)
    {
        if(!autoincrement)
            setAddress(address >> 1);

        m_conn->SendData(cmd);
        waitForAct(WAIT_CHAR2);
        if(m_rec_buff.size() != 2)
            throw tr("Failed to read memory page (timeout)");

        res.append(m_rec_buff[2]); // high
        res.append(m_rec_buff[1]); // low
        address += 2;

        emit updateProgressDialog((address*100)/size);
    }

    return res;
}

QByteArray avr109Programmer::readFlashMemBlock(quint32 start, quint32 size, int block_size)
{
    Q_ASSERT((start % block_size) == 0);

    quint32 address = 0;
    setAddress(address >> 1);

    QByteArray res;
    res.reserve(size);

    QByteArray cmd(4, 0);
    cmd[0] = READ_BLOCK;
    cmd[3] = BLOCK_FLASH;

    m_cancel_requested = false;
    while(address < size && !m_cancel_requested)
    {
        setAddress(address >> 1);

        m_block_size = std::min(block_size, int(size-address));
        cmd[1] = (m_block_size >> 8) & 0xFF;
        cmd[2] = m_block_size & 0xFF;

        m_conn->SendData(cmd);

        waitForAct(WAIT_BLOCK);
        if(m_rec_buff.size() != m_block_size)
            throw tr("Failed to read mem block (timeout)");

        res.append(m_rec_buff);

        address += m_block_size;

        emit updateProgressDialog((address*100)/size);
    }

    return res;
}

QByteArray avr109Programmer::readEEPROM(quint32 start, quint32 size, bool autoincrement)
{
    Q_ASSERT((start % 2) == 0);

    quint32 address = start;
    if(autoincrement)
        setAddress(address);

    QByteArray res;
    res.reserve(size);

    QByteArray cmd = QByteArray::fromRawData(&READ_EEPROM, 1);

    m_cancel_requested = false;
    while(address < size && !m_cancel_requested)
    {
        if(!autoincrement)
            setAddress(address);

        m_conn->SendData(cmd);
        waitForAct(WAIT_CHAR1);
        if(m_rec_buff.size() != 1)
            throw tr("Failed to read memory page (timeout)");

        res.append(m_rec_buff);
        ++address;

        emit updateProgressDialog((address*100)/size);
    }
    return res;
}

QByteArray avr109Programmer::readEEPROMBlock(quint32 start, quint32 size, int block_size)
{
    Q_ASSERT((start % block_size) == 0);

    quint32 address = start;
    setAddress(address);

    QByteArray res;
    res.reserve(size);

    QByteArray cmd(4, 0);
    cmd[0] = READ_BLOCK;
    cmd[3] = BLOCK_EEPROM;

    m_cancel_requested = false;
    while(address < size && !m_cancel_requested)
    {
        setAddress(address);

        m_block_size = std::min(block_size, int(size-address));
        cmd[1] = (m_block_size >> 8) & 0xFF;
        cmd[2] = m_block_size & 0xFF;

        m_conn->SendData(cmd);

        waitForAct(WAIT_BLOCK);
        if(m_rec_buff.size() != m_block_size)
            throw tr("Failed to read mem block (timeout)");

        res.append(m_rec_buff);

        address += m_block_size;

        emit updateProgressDialog((address*100)/size);
    }
    return res;
}

void avr109Programmer::writeFlashMem(const std::vector<page> &pages, const std::set<quint32> &skip, bool autoincrement)
{
    quint32 address = 0;

    QByteArray cmd(4, 0);
    cmd[0] = WRITE_FLASH_LOW;
    cmd[2] = WRITE_FLASH_HIGH;

    QByteArray writePage = QByteArray::fromRawData(&WRITE_PAGE, 1);

    quint32 cntNoSkip = pages.size() - skip.size();

    m_cancel_requested = false;
    for(size_t i = 0; i < pages.size() && !m_cancel_requested; ++i)
    {
        if(skip.find(i) != skip.end())
            continue;

        const page& p = pages[i];
        address = p.address;

        setAddress(address >> 1);

        for(size_t x = 0; x < p.data.size(); address+=2)
        {
            if(!autoincrement)
                setAddress(address >> 1);

            cmd[1] = p.data[x++];
            cmd[3] = p.data[x++];

            m_conn->SendData(cmd);
            waitForAct(WAIT_CHAR2);
            if(m_rec_buff.size() != 2 || m_rec_buff[0] != '\r' || m_rec_buff[1] != '\r')
                throw tr("Failed to write memory page (timeout)");
        }

        m_conn->SendData(writePage);
        waitForAct(WAIT_CHAR1);
        if(m_rec_buff.size() != 1 || m_rec_buff[0] != '\r')
            throw tr("Failed to write memory page (timeout)");

        emit updateProgressDialog((i*100)/cntNoSkip);
    }
}

void avr109Programmer::writeFlashMemBlock(const std::vector<page> &pages, const std::set<quint32> &skip)
{
    quint32 address = 0;

    QByteArray cmd(4, 0);
    cmd[0] = FLASH_BLOCK;
    cmd[3] = BLOCK_FLASH;

    quint32 cntNoSkip = pages.size() - skip.size();

    m_cancel_requested = false;
    for(size_t i = 0; i < pages.size() && !m_cancel_requested; ++i)
    {
        if(skip.find(i) != skip.end())
            continue;

        const page& p = pages[i];
        address = p.address;

        setAddress(address >> 1);

        cmd[1] = (p.data.size() >> 8) & 0xFF;
        cmd[2] = p.data.size() & 0xFF;

        m_conn->SendData(cmd);
        m_conn->SendData(QByteArray::fromRawData((char*)p.data.data(), p.data.size()));

        waitForAct(WAIT_CHAR1);
        if(m_rec_buff.size() != 1 || m_rec_buff[0] != '\r')
            throw tr("Failed to write memory block (timeout)");

        emit updateProgressDialog((i*100)/cntNoSkip);
    }
}

void avr109Programmer::writeEEPROM(const std::vector<page> &pages, bool autoincrement)
{
    quint32 address = 0;

    QByteArray cmd(2, 0);
    cmd[0] = WRITE_EEPROM;

    m_cancel_requested = false;
    for(size_t i = 0; i < pages.size() && !m_cancel_requested; ++i)
    {
        const page& p = pages[i];
        address = p.address;

        setAddress(address);

        for(size_t x = 0; x < p.data.size(); ++address, ++x)
        {
            if(!autoincrement)
                setAddress(address);

            cmd[1] = p.data[x];
            m_conn->SendData(cmd);

            // FIXME: xboot has bug, it does not return ACK
            //waitForAct(WAIT_CHAR1);
            //if(m_rec_buff.size() != 1 || m_rec_buff[0] != '\r')
                //throw tr("Failed to write memory page (timeout)");
            waitForAct(WAIT_EMPTY, 30);
        }

        emit updateProgressDialog((i*100)/pages.size());
    }
}

void avr109Programmer::writeEEPROMBlock(const std::vector<page> &pages)
{
    quint32 address = 0;

    QByteArray cmd(4, 0);
    cmd[0] = FLASH_BLOCK;
    cmd[3] = BLOCK_EEPROM;

    m_cancel_requested = false;

    for(size_t i = 0; i < pages.size() && !m_cancel_requested; ++i)
    {
        const page& p = pages[i];
        address = p.address;

        setAddress(address);

        cmd[1] = (p.data.size() >> 8) & 0xFF;
        cmd[2] = p.data.size() & 0xFF;

        m_conn->SendData(cmd);
        m_conn->SendData(QByteArray::fromRawData((char*)p.data.data(), p.data.size()));

        waitForAct(WAIT_CHAR1);
        if(m_rec_buff.size() != 1 || m_rec_buff[0] != '\r')
            throw tr("Failed to write memory block (timeout)");

        emit updateProgressDialog((i*100)/pages.size());
    }
}

bool avr109Programmer::waitForAct(int waitAct, int timeout)
{
    if(m_wait_act != WAIT_NONE)
    {
        Q_ASSERT(false);
        return false;
    }

    m_rec_buff.clear();
    m_wait_act = waitAct;

    QEventLoop ev(this);
    QTimer t(this);

    connect(&t,   SIGNAL(timeout()), &ev, SLOT(quit()));
    connect(this, SIGNAL(waitActDone()),  &ev, SLOT(quit()));

    t.setSingleShot(true);
    t.start(timeout);

    ev.exec();

    m_wait_act = WAIT_NONE;

    return t.isActive();
}

void avr109Programmer::dataRead(const QByteArray &data)
{
    switch(m_wait_act)
    {
        case WAIT_NONE:
            emit tunnelData(data);
            break;
        case WAIT_EMPTY:
            return;
        case WAIT_SUPPORTED:
            if(data.indexOf(char(0)) != -1)
                emit waitActDone();
            return;
        case WAIT_CHAR1:
        case WAIT_CHAR2:
        case WAIT_CHAR3:
        {
            m_rec_buff.append(data);

            int n = m_wait_act - WAIT_CHAR1 + 1;
            if(m_rec_buff.size() >= n)
            {
                m_rec_buff.resize(n);
                emit waitActDone();
            }
            return;
        }
        case WAIT_BLOCK:
        {
            m_rec_buff.append(data);

            if(m_rec_buff.size() >= m_block_size)
            {
                m_rec_buff.resize(m_block_size);
                emit waitActDone();
            }
            return;
        }
    }
}

void avr109Programmer::sendTunnelData(QString const & data)
{
    m_conn->SendData(data.toUtf8());
}
