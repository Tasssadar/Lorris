#include "atsamprogrammer.h"
#include "../../shared/defmgr.h"
#include "../../shared/hexfile.h"
#include <QEventLoop>
#include <QTimer>
#include <QTime>
#include <stdexcept>

AtsamProgrammer::AtsamProgrammer(ConnectionPointer<PortConnection> const & conn, ProgrammerLogSink * logsink)
    : Programmer(logsink), m_conn(conn)
{
    connect(m_conn.data(), SIGNAL(dataRead(QByteArray)), this, SLOT(dataRead(QByteArray)));
}

void AtsamProgrammer::stopAll(bool wait)
{
    (void)wait;
}

void AtsamProgrammer::switchToFlashMode(quint32 prog_speed_hz)
{
    (void)prog_speed_hz;
}

void AtsamProgrammer::switchToRunMode()
{
    m_conn->SendData("G80000#");
}

bool AtsamProgrammer::isInFlashMode()
{
    return true;
}

chip_definition AtsamProgrammer::readDeviceId()
{
    this->wait_eefc_ready();
    this->write_word(0x400E0804, 0x5A000000/*GETD*/);

    uint32_t desc[4];
    for (size_t i = 0; i < 4; ++i)
    {
        this->wait_eefc_ready();
        desc[i] = this->read_word(0x400E080C);
    }

    this->wait_eefc_ready();

    chip_definition cd;
    cd.setName("atsam");
    cd.setSign("atsam:");

    chip_definition::memorydef md;
    md.memid = 1;
    md.pagesize = desc[2];
    md.size = desc[1];
    cd.getMems()["flash"] = md;

    md.memid = 3;
    md.pagesize = 1;
    md.size = 1;
    cd.getMems()["fuses"] = md;

    chip_definition::fuse f;
    f.bits.push_back(0);
    f.name = tr("Security Bit");
    f.values.push_back(0);
    f.values.push_back(1);
    cd.getFuses().push_back(f);

    f.name = tr("Boot mode selection");
    f.bits[0] = 1;
    cd.getFuses().push_back(f);

    return cd;
}

QByteArray AtsamProgrammer::readMemory(const QString& mem, chip_definition &chip)
{
    QByteArray res;

    m_cancelled = false;    

    chip_definition::memorydef * md = chip.getMemDef(mem);
    if (!md)
        return res;

    uint32_t size = md->size;
    for (uint32_t addr = 0; !m_cancelled && addr < size; addr += 4)
    {
        uint32_t value = this->read_word(0x80000 + addr);
        res.append((char const *)&value, 4);
        emit updateProgressDialog((addr*100)/md->size);
    }
    emit updateProgressDialog(-1);

    return res;
}

void AtsamProgrammer::readFuses(std::vector<quint8>& data, chip_definition &chip)
{
    (void)chip;

    this->wait_eefc_ready();
    this->write_word(0x400E0804, 0x5A00000D/*GGPB*/);
    this->wait_eefc_ready();

    uint32_t value = this->read_word(0x400E080C);
    data.resize(1);
    data[0] = (uint8_t)value;
}

void AtsamProgrammer::writeFuses(std::vector<quint8>& data, chip_definition &chip, VerifyMode /*verifyMode*/)
{
    (void)chip;

    this->wait_eefc_ready();

    for (uint8_t i = 0; i < 2; ++i)
    {
        if (data[0] & (1<<i))
            this->write_word(0x400E0804, 0x5A00000B/*SGPB*/ | (i<<8));
        else
            this->write_word(0x400E0804, 0x5A00000C/*CGPB*/ | (i<<8));

        this->wait_eefc_ready();
    }
}

void AtsamProgrammer::flashRaw(HexFile& file, quint8 memId, chip_definition& chip, VerifyMode /*verifyMode*/)
{
    chip_definition::memorydef * md = chip.getMemDef(memId);
    if (!md)
        return;

    m_cancelled = false;
    for (uint32_t addr = 0; !m_cancelled && addr < md->size; addr += md->pagesize)
    {
        if (!file.intersects(addr, md->pagesize))
            continue;

        std::vector<uint8_t> page(md->pagesize, 0xff);
        file.getRange(addr, md->pagesize, page);

        this->wait_eefc_ready();

        for (size_t page_offset = 0; page_offset < md->pagesize; page_offset += 4)
        {
            uint32_t value
                = page[page_offset]
                | (page[page_offset+1] << 8)
                | (page[page_offset+2] << 16)
                | (page[page_offset+3] << 24);
            this->write_word(0x80000 + addr + page_offset, value);
        }

        this->write_word(0x400E0804, 0x5A000003/*EWP*/ | ((addr / md->pagesize) << 8));
        emit updateProgressDialog((addr*100)/md->size);

        this->wait_eefc_ready();
    }

    emit updateProgressDialog(-1);
}

void AtsamProgrammer::erase_device(chip_definition& chip)
{
    (void)chip;

    this->wait_eefc_ready();
    this->write_word(0x400E0804, 0x5A000005/*EA*/);
    this->wait_eefc_ready();
}

void AtsamProgrammer::wait_eefc_ready()
{
    QTime t;
    t.start();
    while ((this->read_word(0x400E0808) & (1<<0)) == 0)
    {
        if (t.elapsed() > 1000)
            throw tr("The chip faile to become ready.");
    }
}

int AtsamProgrammer::getType()
{
    return programmer_atsam;
}

QString AtsamProgrammer::transact(QString const & data)
{
    m_conn->SendData(data.toLatin1());

    QTimer t;

    connect(&t, SIGNAL(timeout()), &m_waitLoop, SLOT(quit()));

    t.setSingleShot(true);
    t.start(1000);

    m_recvBuffer.clear();
    m_waitLoop.exec();

    if (m_recvBuffer.isEmpty() || m_recvBuffer[m_recvBuffer.size() - 1] != '>')
        throw tr("Failed to get proper response from SAM-BA"); // XXX: should throw something derived from std::exception

    return QString::fromLatin1(m_recvBuffer.data(), m_recvBuffer.size() - 1);
}

void AtsamProgrammer::dataRead(QByteArray const & data)
{
    m_recvBuffer.append(data);
    if (data.contains('>'))
        m_waitLoop.quit();
}

uint32_t AtsamProgrammer::read_word(uint32_t address)
{
    QString result = transact(QString("w%1,#").arg(address, 0, 16));
    bool ok;
    uint32_t res = result.toUInt(&ok, 16);
    if (!ok)
        throw tr("invalid response from SAM-BA"); // XXX
    return res;
}

void AtsamProgrammer::write_word(uint32_t address, uint32_t data)
{
    this->transact(QString("W%1,%2#").arg(address, 0, 16).arg(data, 0, 16));
}

ProgrammerCapabilities AtsamProgrammer::capabilities() const
{
    ProgrammerCapabilities ps;
    ps.flash = true;
    ps.eeprom = true;
    ps.fuses = true;
    return ps;
}
