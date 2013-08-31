/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QEventLoop>
#include <QTimer>

#include "avr232bootprogrammer.h"
#include "../../shared/defmgr.h"
#include "../../shared/hexfile.h"

#define EEPROM_READ_PAGE 128

avr232bootProgrammer::avr232bootProgrammer(ConnectionPointer<PortConnection> const & conn, ProgrammerLogSink * logsink)
    : Programmer(logsink), m_conn(conn)
{
    m_wait_act = WAIT_NONE;
    m_flash_mode = false;
    m_cancel_requested = false;

    connect(m_conn.data(), SIGNAL(dataRead(QByteArray)), this, SLOT(dataRead(QByteArray)));
}

int avr232bootProgrammer::getType()
{
    return programmer_avr232boot;
}

void avr232bootProgrammer::stopAll(bool /*wait*/)
{
}

void avr232bootProgrammer::switchToFlashMode(quint32 /*prog_speed_hz*/)
{
    if(m_flash_mode)
        return;

    static const char stopCmd[4] = { 0x74, 0x7E, 0x7A, 0x33 };

    m_conn->SendData(QByteArray::fromRawData(stopCmd, 4));
    // First sequence restarts chip to bootloader,
    // but I won't get the ack byte. But when chip is already stopped,
    // the first ack is sent, so we have to enter event loop to receive it
    waitForAct(WAIT_ACK, 100);

    m_conn->SendData(QByteArray::fromRawData(stopCmd, 4));
    if(!waitForAct(WAIT_ACK))
        throw tr("Failed to switch to flash mode (timeout).");

    m_flash_mode = true;
}

void avr232bootProgrammer::switchToRunMode()
{
    m_conn->SendData(QByteArray(1, 0x11));
    m_flash_mode = false;
}

bool avr232bootProgrammer::isInFlashMode()
{
    return m_flash_mode;
}

chip_definition avr232bootProgrammer::readDeviceId()
{
    m_rec_buff.clear();

    m_conn->SendData(QByteArray(1, 0x12));
    waitForAct(WAIT_DEV_ID);

    if(m_rec_buff.size() != 4)
        throw tr("Failed to read device id (timeout)");

    return sDefMgr.findChipdef("avr232boot:" + QString(m_rec_buff));
}

QByteArray avr232bootProgrammer::readMemory(const QString& mem, chip_definition &chip)
{
    if(mem != "eeprom")
        throw tr("avr232boot supports read only for EEPROM memory");

    chip_definition::memorydef *md = chip.getMemDef(mem);
    if(!md)
        throw tr("Chip %1 does not have memory type: %2").arg(chip.getName()).arg(mem);

    m_rec_buff.clear();
    m_cancel_requested = false;

    quint16 itr = 0;
    do
    {
        if((quint32)m_rec_buff.size() >= md->size)
            goto success;

        QByteArray cmd(1, 0x13);
        m_conn->SendData(cmd);

        cmd[0] = (itr >> 8);
        cmd[1] = quint8(itr);
        cmd[2] = EEPROM_READ_PAGE;
        m_conn->SendData(cmd);

        itr += EEPROM_READ_PAGE;
        emit updateProgressDialog((itr*100)/md->size);
    } while(!m_cancel_requested && waitForAct(WAIT_EEPROM_READ));

    emit updateProgressDialog(-1);

    if(!m_cancel_requested)
        throw tr("Failed to read EEPROM (timeout)");
    m_cancel_requested = false;

success:
    QByteArray res = m_rec_buff;
    m_rec_buff.clear();
    return res;
}

void avr232bootProgrammer::readFuses(std::vector<quint8>&, chip_definition &)
{
    // The fuse reading/writing is not available in DFU.
}

void avr232bootProgrammer::writeFuses(std::vector<quint8>&, chip_definition &, VerifyMode)
{
    // The fuse reading/writing is not available in DFU.
}

void avr232bootProgrammer::flashRaw(HexFile& file, quint8 memId, chip_definition& chip, VerifyMode /*verifyMode*/)
{
    if(memId != MEM_FLASH && memId != MEM_EEPROM)
        throw tr("avr232boot can only write to flash and EEPROM");

    std::vector<page> pages;
    std::set<quint32> skip;

    file.makePages(pages, memId, chip, memId == MEM_FLASH ? &skip : NULL);

    m_cancel_requested = false;

    int max = pages.size()-skip.size();
    for (size_t i = 0; i < pages.size(); ++i)
    {
        if(skip.find(i) != skip.end())
            continue;

        switch(memId)
        {
            case MEM_FLASH:
                writeFlashPage(pages[i]);
                break;
            case MEM_EEPROM:
                writeEEPROMPage(pages[i]);
                break;
        }

        if(!waitForAct(WAIT_ACK))
            throw tr("Failed to write page (timeout)");

        if(m_cancel_requested)
        {
            emit updateProgressDialog(-1);
            m_cancel_requested = false;
            return;
        }

        emit updateProgressDialog((i*100)/max);
    }
}

void avr232bootProgrammer::writeFlashPage(page& p)
{
    QByteArray cmd(1, 0x10);
    m_conn->SendData(cmd);

    cmd[0] = quint8(p.address >> 8);
    cmd[1] = quint8(p.address);
    m_conn->SendData(cmd);

    for(quint16 i = 0; i < p.data.size(); ++i)
        cmd[i] = p.data[i];
    m_conn->SendData(cmd);
}

void avr232bootProgrammer::writeEEPROMPage(page& p)
{
    QByteArray cmd(1, 0x14);
    m_conn->SendData(cmd);

    cmd[0] = quint8(p.address >> 8);
    cmd[1] = quint8(p.address);
    cmd[2] = p.data.size();
    m_conn->SendData(cmd);

    cmd.clear();
    for(quint8 i = 0; i < p.data.size(); ++i)
        cmd[i] = p.data[i];
    m_conn->SendData(cmd);
}

void avr232bootProgrammer::erase_device(chip_definition& /*chip*/)
{
    // Not available in avr232boot
    throw tr("Erasing not supported in avr232boot.");
}

void avr232bootProgrammer::cancelRequested()
{
    m_cancel_requested = true;
}

bool avr232bootProgrammer::waitForAct(int waitAct, int timeout)
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

void avr232bootProgrammer::dataRead(const QByteArray &data)
{
    switch(m_wait_act)
    {
        case WAIT_NONE:
            break;
        case WAIT_ACK:
            if(data.indexOf(char(20)) != -1)
                emit waitActDone();
            return;
        case WAIT_DEV_ID:
            m_rec_buff.append(data);
            if(m_rec_buff.size() >= 4)
            {
                m_rec_buff.resize(4);
                emit waitActDone();
            }
            return;
        case WAIT_EEPROM_READ:
            m_rec_buff.append(data);
            if(m_rec_buff.size()%EEPROM_READ_PAGE == 0)
                emit waitActDone();
            return;
    }

    emit tunnelData(data);
}

void avr232bootProgrammer::sendTunnelData(QString const & data)
{
    m_conn->SendData(data.toUtf8());
}

ProgrammerCapabilities avr232bootProgrammer::capabilities() const
{
    ProgrammerCapabilities ps;
    ps.terminal = true;
    ps.flash = true;
    ps.eeprom = true;
    return ps;
}
