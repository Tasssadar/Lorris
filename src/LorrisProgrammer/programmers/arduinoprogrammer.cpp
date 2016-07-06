#include <QTimer>
#include <QEventLoop>

#include "arduinoprogrammer.h"
#include "../../shared/defmgr.h"
#include "../../misc/utils.h"

// http://baldwisdom.com/bootloading/
enum Commands : char {
    Sync_CRC_EOP = 0x20,

    STK_GET_SYNCH = 0x30,
    STK_GET_PARAMETER = 0x41,
    STK_SW_MAJOR = (char)0x81,
    STK_ENTER_PROGMODE = 0x50,
    STK_READ_SIGN = 0x75,
    STK_LOAD_ADDRESS = 0x55,
    STK_READ_PAGE = 0x74,
    STK_PROGRAM_PAGE = 0x64,
    STK_CHIP_ERASE = 0x52,

    STK_INSYNC = 0x14,
    STK_OK = 0x10,
};

#define READ_PAGE_SIZE 256

ArduinoProgrammer::ArduinoProgrammer(ConnectionPointer<SerialPort> const & conn, ProgrammerLogSink * logsink) : Programmer(logsink), m_stay_in_bl_timer(this) {
    m_conn = conn;
    m_flash_mode = false;
    m_ignore_incoming = false;
    m_wait_act = WAIT_NONE;
    m_cancel_requested = false;

    connect(m_conn.data(), SIGNAL(dataRead(QByteArray)), this, SLOT(dataRead(QByteArray)));
    connect(&m_stay_in_bl_timer, SIGNAL(timeout()), this, SLOT(stayInBootloader()));
}

ArduinoProgrammer::~ArduinoProgrammer() {

}

void ArduinoProgrammer::stopAll(bool /*wait*/) {

}

void ArduinoProgrammer::switchToFlashMode(quint32 prog_speed_hz) {
    if(m_flash_mode) {
        return;
    }

    if(m_wait_act != WAIT_NONE) {
        throw tr("Already stopping!");
    }

    m_conn->setRts(false);
    m_conn->setDtr(false);
    waitForAct(WAIT_RTS_DTR, 250);
    m_conn->setRts(true);
    m_conn->setDtr(true);
    waitForAct(WAIT_RTS_DTR, 50);

    static const char stopCmd[] = { STK_GET_SYNCH, Sync_CRC_EOP };

    m_ignore_incoming = true;

    m_conn->SendData(QByteArray::fromRawData(stopCmd, sizeof(stopCmd)));
    m_conn->SendData(QByteArray::fromRawData(stopCmd, sizeof(stopCmd)));

    bool synced = false;
    for(int i = 0; i < 10; ++i) {
        m_conn->SendData(QByteArray::fromRawData(stopCmd, 2));
        if(waitForAct(WAIT_SYNC, 100)) {
            synced = true;
            break;
        }
    }

    m_ignore_incoming = false;
    if(!synced) {
        throw tr("Failed to switch to flash mode (timeout).");
    }

    // TODO: Get protocol version should be here

    static const char enterProgMode[] = { STK_ENTER_PROGMODE, Sync_CRC_EOP };
    m_conn->SendData(QByteArray::fromRawData(enterProgMode, 2));
    if(!waitForAct(WAIT_SYNC, 100)) {
        throw tr("Failed to switch to flash mode (timeout).");
    }

    m_flash_mode = true;
    setStayInBootloaderTimer(true);
}

// The bootloader times out after about 1s and runs the main program
// if no data are received, because fuck you.
void ArduinoProgrammer::setStayInBootloaderTimer(bool run) {
    if(run == m_stay_in_bl_timer.isActive())
        return;

    if(run) {
        m_stay_in_bl_timer.start(500);
    } else {
        m_stay_in_bl_timer.stop();
    }
}

void ArduinoProgrammer::stayInBootloader() {
    static const char syncCmd[] = { STK_GET_SYNCH, Sync_CRC_EOP };
    m_conn->SendData(QByteArray::fromRawData(syncCmd, sizeof(syncCmd)));
    waitForAct(WAIT_SYNC, 50);
}

void ArduinoProgrammer::switchToRunMode() {
    if(!m_flash_mode)
        return;

    m_flash_mode = false;
    setStayInBootloaderTimer(false);

    m_conn->setRts(false);
    m_conn->setDtr(false);
    waitForAct(WAIT_RTS_DTR, 250);
    m_conn->setRts(true);
    m_conn->setDtr(true);
    waitForAct(WAIT_RTS_DTR, 50);
}

chip_definition ArduinoProgrammer::readDeviceId() {
    m_rec_buff.clear();

    setStayInBootloaderTimer(false);

    static const char readSign[] = { STK_READ_SIGN, Sync_CRC_EOP };
    m_conn->SendData(QByteArray::fromRawData(readSign, 2));

    if(!waitForAct(WAIT_DEVICE_ID, 200)) {
        throw tr("Failed to read device id (timeout)");
    }

    setStayInBootloaderTimer(true);

    QString sign("avr:");
    sign.append(Utils::toBase16((quint8*)m_rec_buff.data()+1, (quint8*)m_rec_buff.data()+4));
    return sDefMgr.findChipdef(sign);
}

QByteArray ArduinoProgrammer::readMemory(const QString& mem, chip_definition &chip) { 
    if(mem != "eeprom" && mem != "flash") {
        throw tr("Arduino bootloader can only read eeprom and flash memories.");
    }

    setStayInBootloaderTimer(false);
    m_cancel_requested = false;

    /*QByteArray cmd(4, '\0');
    cmd[0] = STK_LOAD_ADDRESS;
    cmd[1] = 0x00;
    cmd[2] = 0x00;
    cmd[3] = Sync_CRC_EOP;
    m_conn->SendData(cmd);

    if(!waitForAct(WAIT_SYNC, 100)) {
        throw tr("The load address command has failed!");
    }*/

    QByteArray cmd_load_addr(4, '\0');
    cmd_load_addr[0] = STK_LOAD_ADDRESS;
    // addr
    cmd_load_addr[3] = Sync_CRC_EOP;

    const quint32 pagesize = READ_PAGE_SIZE;

    QByteArray cmd(5, '\0');
    cmd[0] = STK_READ_PAGE;
    cmd[1] = (pagesize >> 8) & 0xFF;
    cmd[2] = (pagesize & 0xFF);
    cmd[3] = mem == "flash" ? 'F' : 'E';
    cmd[4] = Sync_CRC_EOP;

    auto memdef = chip.getMemDef(mem);

    m_cur_page_size = pagesize;

    QByteArray res;
    for(quint32 read = 0; !m_cancel_requested && read < memdef->size; ) {
        // Bootloader should have autoincrement. It's lying.
        cmd_load_addr[1] = ((read/2) & 0xFF);
        cmd_load_addr[2] = (((read/2) >> 8) & 0xFF);
        m_conn->SendData(cmd_load_addr);

        if(!waitForAct(WAIT_SYNC, 1000)) {
            throw tr("The load address command has failed!");
        }

        read += pagesize;

        m_rec_buff.clear();
        m_conn->SendData(cmd);
        if(!waitForAct(WAIT_PAGE_READ, 1000)) {
            emit updateProgressDialog(-1);
            setStayInBootloaderTimer(true);
            throw tr("Timeout while reading memory!");
        }

        if(m_rec_buff.at(0) != STK_INSYNC || m_rec_buff.at(1+pagesize) != STK_OK) {
            emit updateProgressDialog(-1);
            setStayInBootloaderTimer(true);
            throw tr("Invalid response while reading memory!");
        }
        res.append(m_rec_buff.data() + 1, pagesize);

        emit updateProgressDialog((read*100)/memdef->size);
    }

    emit updateProgressDialog(-1);

    setStayInBootloaderTimer(true);
    return res;
}

void ArduinoProgrammer::readFuses(std::vector<quint8>& data, chip_definition &chip) {

}

void ArduinoProgrammer::writeFuses(std::vector<quint8>& data, chip_definition &chip, VerifyMode verifyMode) {

}

void ArduinoProgrammer::flashRaw(HexFile& file, quint8 memId, chip_definition& chip, VerifyMode verifyMode) {
    if(memId != MEM_FLASH && memId != MEM_EEPROM)
        throw tr("arduino can only write to flash and EEPROM");

    std::vector<page> pages;
    std::set<quint32> skip;

    auto *memdef = chip.getMemDef(memId);

    file.makePages(pages, memId, chip, memId == MEM_FLASH ? &skip : NULL);

    m_cancel_requested = false;

    QByteArray cmd_load_address(4, '\0');
    cmd_load_address[0] = STK_LOAD_ADDRESS;
    // addr
    cmd_load_address[3] = Sync_CRC_EOP;

    const quint16 pagesize = memdef->pagesize;
    QByteArray cmd_program_page(1 + 2 + 1 + pagesize + 1, '\0');
    cmd_program_page[0] = STK_PROGRAM_PAGE;
    cmd_program_page[1] = (pagesize >> 8) & 0xFF;
    cmd_program_page[2] = (pagesize & 0xFF);
    cmd_program_page[3] = memId == MEM_FLASH ? 'F' : 'E';
    // data
    cmd_program_page[cmd_program_page.size()-1] = Sync_CRC_EOP;

    setStayInBootloaderTimer(false);

    int max = pages.size() - skip.size();
    int prog = 0;
    for (size_t i = 0; i < pages.size(); ++i)
    {
        if(skip.find(i) != skip.end())
            continue;

        const page &p = pages[i];

        // divide by 2 for some reason, not important enough
        // to be mentioned in the docs
        cmd_load_address[1] = ((p.address/2) & 0xFF);
        cmd_load_address[2] = ((p.address/2) >> 8) & 0xFF;

        m_conn->SendData(cmd_load_address);
        if(!waitForAct(WAIT_SYNC, 2000))
            throw tr("Failed to write page (timeout during address set)");

        memcpy(cmd_program_page.data() + 4, p.data.data(), p.data.size());
        m_conn->SendData(cmd_program_page);
        if(!waitForAct(WAIT_SYNC, 2000))
            throw tr("Failed to write page (timeout during address set)");

        if(m_cancel_requested)
        {
            emit updateProgressDialog(-1);
            setStayInBootloaderTimer(false);
            m_cancel_requested = false;
            return;
        }

        emit updateProgressDialog((++prog*100)/max);
    }

    if(memId == MEM_EEPROM && verifyMode == VERIFY_ONLY_NON_EMPTY)
        verifyMode = VERIFY_ALL_PAGES;

    switch(verifyMode) {
    case VERIFY_NONE:
        break;
    case VERIFY_ALL_PAGES: {
        QByteArray mem = readMemory(chip.memIdToName(memId), chip);
        for (size_t i = 0; i < pages.size(); ++i) {
            const page &p = pages[i];
            if(p.address + p.data.size() > mem.size()) {
                throw tr("Verification failed!");
            }

            if(memcmp(mem.data() + p.address, p.data.data(), p.data.size()) != 0) {
                throw tr("Verification failed!");
            }
        }
        break;
    }
    case VERIFY_ONLY_NON_EMPTY: {
        QByteArray block;
        prog = 0;
        for(size_t i = 0; i < pages.size() && !m_cancel_requested; ++i)
        {
            if(skip.find(i) != skip.end())
                continue;

            block.clear();
            const page& p = pages[i];
            try {
                block = readPage(p.address, p.data.size(), memId);
            } catch(QString ex) {
                qWarning() << ex;
            }

            //qWarning() << p.address << " " << Utils::toBase16((quint8*)block.data(), (quint8*)block.data() + block.size());
            if ((size_t)block.size() != p.data.size() ||
                !std::equal(p.data.data(), p.data.data()+p.data.size(), (quint8*)block.data()))
            {
                setStayInBootloaderTimer(true);
                throw tr("Verification failed!");
            }
            emit updateProgressDialog(((++prog)*100)/max);
        }
        break;
    }
    }

    setStayInBootloaderTimer(true);
}

QByteArray ArduinoProgrammer::readPage(quint16 address, quint16 pagesize, quint8 memId) {
    QByteArray cmd(4, '\0');
    cmd[0] = STK_LOAD_ADDRESS;
    cmd[1] = (address/2) & 0xFF;
    cmd[2] = ((address/2) >> 8) & 0xFF;
    cmd[3] = Sync_CRC_EOP;
    m_conn->SendData(cmd);

    if(!waitForAct(WAIT_SYNC, 1000)) {
        throw tr("The load address command has failed!");
    }

    cmd.resize(5);
    cmd[0] = STK_READ_PAGE;
    cmd[1] = (pagesize >> 8) & 0xFF;
    cmd[2] = (pagesize & 0xFF);
    cmd[3] = memId == MEM_FLASH ? 'F' : 'E';
    cmd[4] = Sync_CRC_EOP;

    m_rec_buff.clear();
    m_cur_page_size = pagesize;

    m_conn->SendData(cmd);
    if(!waitForAct(WAIT_PAGE_READ, 1000)) {
        throw tr("Timeout while reading memory!");
    }

    if(m_rec_buff.at(0) != STK_INSYNC || m_rec_buff.at(1+pagesize) != STK_OK) {
        throw tr("Invalid response while reading memory!");
    }

    return QByteArray(m_rec_buff.data() + 1, pagesize);
}

void ArduinoProgrammer::erase_device(chip_definition& chip) {
    throw tr("Arduino bootloader does not support chip erase.");
    // Arduino ignores this.
    /*static const char eraseCmd[] = { STK_CHIP_ERASE, Sync_CRC_EOP };
    m_conn->SendData(QByteArray::fromRawData(eraseCmd, 2));

    if(!waitForAct(WAIT_SYNC, 1000)) {
        throw tr("Timeout during chip erase");
    }*/
}

int ArduinoProgrammer::getType() {
    return programmer_arduino;
}

ProgrammerCapabilities ArduinoProgrammer::capabilities() const {
    ProgrammerCapabilities ps;
    ps.terminal = true;
    ps.flash = true;
    ps.eeprom = true;
    return ps;
}

void ArduinoProgrammer::cancelRequested() {
    m_cancel_requested = true;
}

bool ArduinoProgrammer::waitForAct(int waitAct, int timeout)
{
    if(m_wait_act != WAIT_NONE)
    {
        Q_ASSERT(false);
        return false;
    }

    m_wait_act = waitAct;

    QEventLoop ev;
    QTimer t;

    connect(&t,   SIGNAL(timeout()), &ev, SLOT(quit()));
    connect(this, SIGNAL(waitActDone()),  &ev, SLOT(quit()));

    t.setSingleShot(true);
    t.start(timeout);

    ev.exec();

    m_wait_act = WAIT_NONE;

    return t.isActive();
}

void ArduinoProgrammer::dataRead(const QByteArray &data) {
    //qWarning() << Utils::toBase16((quint8*)data.data(), (quint8*)data.data() +data.size());
    switch(m_wait_act) {
        case WAIT_SYNC: {
            const int idx = data.indexOf(STK_INSYNC);
            if(idx != -1 && idx+1 < data.size() && data[idx+1] == STK_OK)
                emit waitActDone();
            return;
        }
        case WAIT_DEVICE_ID: {
            m_rec_buff.append(data.data(), std::min(data.size(), 5-m_rec_buff.size()));
            if(m_rec_buff.size() == 5 && m_rec_buff.at(0) == STK_INSYNC && m_rec_buff.at(4) == STK_OK) {
                emit waitActDone();
            }
            return;
        }
        case WAIT_PAGE_READ: {
            m_rec_buff.append(data);
            if(m_rec_buff.size() >= m_cur_page_size + 2) {
                m_rec_buff.resize(m_cur_page_size + 2);
                emit waitActDone();
            }
            return;
        }
    }

    if(!m_ignore_incoming)
        emit tunnelData(data);
}

void ArduinoProgrammer::sendTunnelData(QString const & data)
{
    m_conn->SendData(data.toUtf8());
}
