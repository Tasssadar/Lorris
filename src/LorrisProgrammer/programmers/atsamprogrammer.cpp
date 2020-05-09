#include "atsamprogrammer.h"
#include "../../shared/defmgr.h"
#include "../../shared/hexfile.h"
#include <QEventLoop>
#include <QTimer>
#include <QTime>
#include <QCoreApplication>
#include <QDir>
#include <stdexcept>
#include <fstream>
#include <string>

AtsamProgrammer::AtsamProgrammer(ConnectionPointer<PortConnection> const & conn, ProgrammerLogSink * logsink)
    : Programmer(logsink), m_cancelled(false), m_flash_mode(false), m_tunnel_enabled(false), m_applet_address(0), m_chipdef(nullptr), m_conn(conn)
{
    connect(m_conn.data(), SIGNAL(dataRead(QByteArray)), this, SLOT(dataRead(QByteArray)));
}

AtsamProgrammer::~AtsamProgrammer()
{
    if(m_chipdef != nullptr)
        delete m_chipdef;
}

void AtsamProgrammer::stopAll(bool /*wait*/)
{

}

void AtsamProgrammer::switchToFlashMode(quint32 /*prog_speed_hz*/)
{
    if(m_flash_mode)
        return;

    m_tunnel_enabled = false;

    static const char stopCmd[4] = { 0x74, 0x7E, 0x7A, 0x33 };
    m_conn->SendData(QByteArray::fromRawData(stopCmd, 4));

    QTimer t;

    connect(&t, SIGNAL(timeout()), &m_waitLoop, SLOT(quit()));

    t.setSingleShot(true);
    t.start(100);

    m_waitLoop.exec();

    transact("v#");

    m_flash_mode = true;

    this->readDeviceId();
    if(m_chipdef != nullptr)
    {
        QFile f(":/SAM-BA_loader.bin");
        if(f.open(QIODevice::ReadOnly))
        {
            QByteArray loader = f.readAll();
            if(loader.length() % 128)
                loader += QByteArray(128 - (loader.length() % 128), 0);

            uint32_t loader_address = m_chipdef->getMems()["sram"].start_addr + 2048 + 32 + m_chipdef->getMems()["flash"].pagesize;
            this->write_file(loader_address, loader);
            m_applet_address = loader_address;
        }
    }
}

void AtsamProgrammer::switchToRunMode()
{
    if(m_chipdef == nullptr)
    {
        m_flash_mode = false;
        this->switchToFlashMode(0);
    }
    if(m_applet_address == 0)
    {
        QString cmd = QString("G%1#").arg(m_chipdef->getMems()["flash"].start_addr, 0, 16);
        m_conn->SendData(cmd.toLatin1());
        this->debug_output("<-", cmd);
    }
    else
    {
        this->write_word(m_applet_address - m_chipdef->getMems()["flash"].pagesize - 12, 2); // reboot to flash cmd
        QString cmd = QString("G%1#").arg(m_applet_address, 0, 16);
        m_conn->SendData(cmd.toLatin1());
        this->debug_output("<-", cmd);
    }
    m_flash_mode = false;
    m_tunnel_enabled = true;
    m_applet_address = 0;
}

bool AtsamProgrammer::isInFlashMode()
{
    return m_flash_mode;
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

    uint32_t chipid = this->read_word(0x400E0740);

    if(m_chipdef != nullptr)
        delete m_chipdef;
    m_chipdef = new  chip_definition;

    chip_definition& cd = *m_chipdef;
    cd.setName("atsam");
    cd.setSign("atsam:");

    chip_definition::memorydef md;
    md.memid = MEM_FLASH;
    md.pagesize = desc[2];
    md.size = desc[1];
    md.start_addr = 0x00080000;
    cd.getMems()["flash"] = md;

    md.memid = MEM_FUSES;
    md.pagesize = 1;
    md.size = 1;
    cd.getMems()["fuses"] = md;

    static const uint32_t sram_size[16] =
        {  49152,   1024,   2048,   6144,  24576,   4096,  81920, 163840,
            8192,  16384,  32768,  65536, 131072, 262144,  98304, 524288 };

    md.memid = MEM_SDRAM;
    md.pagesize = 1;
    md.size = sram_size[(chipid & 0x000F0000)>>16];
    md.start_addr = 0x2007C000; /*SRAM as contignous block on ATSAM3U2C*/
    cd.getMems()["sram"] = md;

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

QByteArray AtsamProgrammer::readMemory(const QString& mem, chip_definition &chip) // FIX ME: should use xmodem protocol
{
    QByteArray res;

    m_cancelled = false;    

    chip_definition::memorydef * md = chip.getMemDef(mem == "eeprom" ? "sram" : mem);
    if (!md)
        return res;

    uint32_t size = md->size;
    for (uint32_t addr = 0; !m_cancelled && addr < size; addr += 4)
    {
        uint32_t value = this->read_word(md->start_addr + addr);
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

    std::vector<page> pages;
    std::set<quint32> skip;

    file.makePages(pages, memId, chip, &skip);

    int max = pages.size()-skip.size();

    uint32_t source_address = 0;
    if(m_applet_address != 0)
    {
        source_address = m_applet_address - chip.getMems()["flash"].pagesize;
        this->write_word(source_address - 32, source_address); // source address
        this->write_word(source_address - 28, md->start_addr); // destination address
        this->write_word(source_address - 24, 0); // start page
        this->write_word(source_address - 20, 1); // pages count
        this->write_word(source_address - 16, md->pagesize); // page size
        this->write_word(source_address - 12, 1); // write cmd
    }

    m_cancelled = false;
    for (uint32_t addr = 0; !m_cancelled && addr < pages.size(); ++addr)
    {

        if(skip.find(addr) != skip.end())
            continue;

        if(m_applet_address != 0)
        {
            this->write_file(m_applet_address - md->pagesize, QByteArray((char *)(&pages[addr].data[0]), pages[addr].data.size()));
            this->transact(QString("G%1#").arg(m_applet_address, 0, 16));
        }
        else
        {
            this->wait_eefc_ready();

            for (size_t page_offset = 0; page_offset < md->pagesize; page_offset += 4)
            {
                uint32_t value
                    = (pages[addr].data[page_offset+0])
                    | (pages[addr].data[page_offset+1] << 8)
                    | (pages[addr].data[page_offset+2] << 16)
                    | (pages[addr].data[page_offset+3] << 24);
                this->write_word(md->start_addr + addr * md->pagesize + page_offset, value);
            }
            this->wait_eefc_ready();
            this->write_word(0x400E0804, 0x5A000003/*EWP*/ | (addr << 8));
        }

        emit updateProgressDialog((addr*100)/max);
    }

    emit updateProgressDialog(-1);
}

quint16 AtsamProgrammer::crc16(QByteArray const & data, quint32 crc = 0)
{
    crc &= 0xFFFF;
    for(int i = 0; i != data.size(); ++i)
    {
        crc ^= (data.at(i) << 8);
        for(int j = 0; j != 8; ++j)
        {
            crc <<= 1;
            if(crc & 0x10000)
                crc ^= 0x11021;
        }
    }
    return quint16(crc);
}

void AtsamProgrammer::write_file(const uint32_t& address, QByteArray const & data)
{
    int attempts = 0;
    this->transact(QString("S%1,#").arg(address, 0, 16), "C");
    for(int i = 0; i != data.size() / 128 + 1; ++i)
    {
        QByteArray packet;
        if(i != data.size() / 128)
        {
            packet.append('\x01'); // SOH
            char num = (i + 1) & 0xFF;
            packet.append(num);
            packet.append(~num);
            QByteArray payload = data.mid(128 * i, 128);
            quint16 crc = this->crc16(payload);
            packet.append(payload);
            packet.append(char(crc>>8));
            packet.append(char(crc&0xFF));
        }
        else
        {
            packet.append('\x04'); // EOT
        }
        QString res = this->transact(packet, "\x06\x15>", true);
        if(res.size() == 0)
            res = "\x00";
        switch(res.at(0).toLatin1())
        {
        case '\x06': // ACK
            attempts = 0;
            this->debug_output(">>", "ACK");
            break;
        case '\x15': // NACK
            this->debug_output(">>", "NACK");
            if(++attempts == 3)
                throw tr("Unable to send packet");
            else
                --i;
            continue;
        case '>':
            this->debug_output(">>", ">");
            throw tr("SAM-BA timeout");
            break;
        default:
            this->debug_output(">>", res.toLatin1().toHex());
            break;
        }
    }
    while(!(m_recvBuffer.contains('>') || m_recvBuffer1.contains('>')))
    {
        QTimer t;
        connect(&t, SIGNAL(timeout()), &m_waitLoop, SLOT(quit()));
        t.setSingleShot(true);
        t.start(1000);
        m_waitLoop.exec();
    }
    if(!(m_recvBuffer.contains('>') || m_recvBuffer1.contains('>')))
        throw tr("Failed to get proper response from SAM-BA (write_file)"); // XXX: should throw something derived from std::exception
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
            throw tr("The chip failed to become ready.");
    }
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

QString AtsamProgrammer::transact(const QString & data, const QString & delimiter)
{
    this->debug_output("<-", data);
    transact(data.toLatin1(), delimiter, false);
    QString res = QString::fromLatin1(m_recvBuffer.data(), m_recvBuffer.size() - 1);
    this->debug_output("->", res);
    return res;
}

QByteArray AtsamProgrammer::transact(const QByteArray & data, const QString & delimiter, const bool& hex_debug)
{
    m_recvDelimiter = delimiter.toLatin1();
    m_recvBuffer.clear();
    if(hex_debug)
        this->debug_output("<<", data.toHex());

    m_conn->SendData(data);

    QTimer t;
    connect(&t, SIGNAL(timeout()), &m_waitLoop, SLOT(quit()));
    t.setSingleShot(true);
    t.start(1000);
    m_waitLoop.exec();

    if (m_recvBuffer.isEmpty() || !m_recvDelimiter.contains(m_recvBuffer[m_recvBuffer.size() - 1]))
        throw tr("Failed to get proper response from SAM-BA (transact)"); // XXX: should throw something derived from std::exception

    if(hex_debug)
        this->debug_output("->", m_recvBuffer.toHex());
    return m_recvBuffer;
}

void AtsamProgrammer::debug_output(QString const & /*dir*/, QString /*data*/)
{
    //this->log(QString("%2 %1 \"%3\"\n").arg(dir).arg(QTime::currentTime().toString("hh:mm:ss.zzz")).arg(data.replace("\n", "\\n").replace("\r", "\\r")).toLatin1());
}

void AtsamProgrammer::dataRead(const QByteArray & data)
{
    if(m_tunnel_enabled)
        emit tunnelData(data);
    else
    {
        this->debug_output("!>", data.toHex());
        for(int i = 0; i != data.size(); ++i)
        {
            m_recvBuffer.append(data[i]);
            if(m_recvDelimiter.contains(data[i]))
            {
                m_recvBuffer1 = data.mid(i + 1);
                m_waitLoop.quit();
                break;
            }
        }
    }
}

void AtsamProgrammer::sendTunnelData(const QString & data)
{
    if(m_tunnel_enabled)
        m_conn->SendData(data.toUtf8());
}

int AtsamProgrammer::getType()
{
    return programmer_atsam;
}

ProgrammerCapabilities AtsamProgrammer::capabilities() const
{
    ProgrammerCapabilities ps;
    ps.flash = true;
    ps.terminal = true;
    ps.fuses = true;
    return ps;
}
