/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "../shupito.h"
#include "shupitods89c.h"
#include "../../shared/defmgr.h"
#include "../../misc/utils.h"

ShupitoDs89c::ShupitoDs89c(Shupito *shupito)
    : ShupitoMode(shupito), m_flash_mode(false)
{
}

static QString getFirstToken(QString str)
{
    int first = 0;
    for (; first < str.size(); ++first)
    {
        if (str[first] != ' ' && str[first] != '\r' && str[first] != '\n' && str[first] != '\t')
            break;
    }

    int last = first;
    for (; last < str.size(); ++last)
    {
        if (str[last] == ' ' || str[last] == '\r' || str[last] == '\n' || str[last] == '\t')
            break;
    }

    return QString(str.data() + first, last - first);
}

void ShupitoDs89c::switchToFlashMode(quint32 speed_hz)
{
    m_flash_mode = false;

    ShupitoDesc::config const *prog_cfg = getModeCfg();
    if (!prog_cfg)
        throw QString(QObject::tr("The device can't program these types of chips."));

    m_shupito->sendPacket(prog_cfg->getStateChangeCmd(true));
    m_prog_cmd_base = prog_cfg->cmd;

    uint8_t pck[5];
    pck[0] = m_prog_cmd_base;
    serialize_le(pck + 1, speed_hz);
    m_shupito->sendPacket(ShupitoPacket(pck, pck + sizeof pck));

    ShupitoPacket resp = m_shupito->waitForPacket(m_prog_cmd_base);
    if (resp.empty() || resp[1] != 0)
        throw QString(QObject::tr("Failed to initialize USART mode."));

    m_shupito->sendPacket(makeShupitoPacket(m_prog_cmd_base + 4, 2, 0, 1));

    m_capture.data.clear();
    m_shupito->registerCapture(m_prog_cmd_base + 3, m_capture);

    m_shupito->sendPacket(makeShupitoPacket(m_prog_cmd_base + 2, 1, 13));
    resp = m_shupito->waitForPacket(m_prog_cmd_base + 2);
    if (resp.empty() || resp[1] != 0)
        throw QString(QObject::tr("Failed to transfer data."));

    m_banner = this->waitForPrompt();
    m_banner = getFirstToken(m_banner).toLower();
    m_flash_mode = true;
}

void ShupitoDs89c::switchToRunMode()
{
    if (!m_flash_mode)
        return;

    m_shupito->unregisterCapture(m_prog_cmd_base + 3);

    ShupitoDesc::config const *prog_cfg = getModeCfg();
    Q_ASSERT(prog_cfg != 0);
    m_shupito->sendPacket(prog_cfg->getStateChangeCmd(false));
    m_flash_mode = false;
}

QString ShupitoDs89c::read()
{
    ShupitoPacket pck = m_shupito->waitForPacket(m_prog_cmd_base + 3);
    if (pck.size() < 2)
        throw QString(QObject::tr("Invalid response from the chip"));

    QString res = m_capture.data;
    m_capture.data.clear();
    return res;
}

QString ShupitoDs89c::waitForPrompt()
{
    while (!m_capture.data.endsWith("\r\n> "))
    {
        ShupitoPacket pck = m_shupito->waitForPacket(m_prog_cmd_base + 3);
        if (pck.size() < 2)
            throw QString(QObject::tr("Invalid response from the chip"));
    }

    m_capture.data.chop(4);

    QString res = m_capture.data;
    m_capture.data.clear();

    int pos = res.indexOf('\n');
    if (pos >= 0)
        res = QString(res.data() + pos + 1, res.size() - pos);

    return res;
}

void ShupitoDs89c::waitForNewLine()
{
    while (!m_capture.data.endsWith("\r\n"))
    {
        ShupitoPacket pck = m_shupito->waitForPacket(m_prog_cmd_base + 3);
        if (pck.size() < 2)
            throw QString(QObject::tr("Invalid response from the chip"));
    }

    m_capture.data.clear();
}

chip_definition ShupitoDs89c::readDeviceId()
{
    chip_definition cd;
    cd.setSign("ds89c:" + m_banner);

    chip_definition::memorydef md;
    md.memid = 3;
    md.size = 2;
    cd.getMems()["fuses"] = md;

    sDefMgr.update(cd);
    return cd;
}

ShupitoDesc::config const * ShupitoDs89c::getModeCfg()
{
    return m_shupito->getDesc()->getConfig("b1a28e62-6d13-44b5-8894-0b9f7a3061c9");
}

ProgrammerCapabilities ShupitoDs89c::capabilities() const
{
    ProgrammerCapabilities ps;
    ps.flash = true;
    ps.fuses = true;
    return ps;
}

void ShupitoDs89c::flashPage(chip_definition::memorydef *memdef, std::vector<quint8>& memory, quint32 address)
{
    if (memdef->memid != 1)
        throw QString("Unsupported");

    HexFile hf;
    hf.addRegion(address, memory.data(), memory.data() + memory.size(), 0);

    this->write("L\r");
    this->waitForNewLine();

    QList<QByteArray> lines = hf.SaveToArray();
    for (int i = 0; i < lines.size(); ++i)
    {
        this->write(lines[i]);

        if (i != lines.size() - 1)
        {
            QString resp = this->read();
            if (resp != "G")
                throw QString(QObject::tr("An error response received: ") + resp);
        }
        else
        {
            this->waitForPrompt();
        }
    }
}

void ShupitoDs89c::write(QByteArray const & line)
{
    size_t maxSize = m_shupito->maxPacketSize();

    int offs = 0;
    std::vector<uint8_t> pck;
    pck.push_back(m_prog_cmd_base + 2);
    while (offs < line.size())
    {
        size_t chunk = line.size();
        if (chunk > maxSize)
            chunk = maxSize;

        pck.resize(chunk + 1);
        std::copy(line.data() + offs, line.data() + offs + chunk, pck.data() + 1);
        m_shupito->sendPacket(pck);

        offs += chunk;
    }
}

void ShupitoDs89c::readMemRange(quint8 /*memid*/, QByteArray& memory, quint32 address, quint32 size)
{
    QByteArray cmd = QString("D %1 %2\r").arg(address, 0, 16).arg(address + size - 1, 0, 16).toUpper().toUtf8();

    ShupitoPacket pck;
    pck.push_back(m_prog_cmd_base + 2);
    pck.insert(pck.end(), cmd.begin(), cmd.end());

    m_shupito->sendPacket(pck);
    m_shupito->waitForPacket(m_prog_cmd_base + 2);

    QString resp = this->waitForPrompt();

    HexFile hf;
    hf.DecodeFromString(resp.toUtf8());

    memory.resize(memory.size() + size);
    std::fill(memory.data() + memory.size() - size, memory.data() + memory.size(), 0xff);

    hf.getRange(address, size, (quint8 *)memory.data() + memory.size() - size);
}

void ShupitoDs89c::erase_device(chip_definition&)
{
    m_shupito->sendPacket(makeShupitoPacket(m_prog_cmd_base + 2, 2, 'K', '\r'));
    m_shupito->waitForPacket(m_prog_cmd_base + 2);
    this->waitForPrompt();
}

void ShupitoDs89c::readFuses(std::vector<quint8>& data, chip_definition &)
{
    m_shupito->sendPacket(makeShupitoPacket(m_prog_cmd_base + 2, 2, 'R', '\r'));
    m_shupito->waitForPacket(m_prog_cmd_base + 2);

    QString resp = this->waitForPrompt();

    data.resize(2);
    data[0] = 0;
    data[1] = 0xff;

    QStringList l = resp.split(' ');
    for (int i = 0; i < l.size(); ++i)
    {
        QStringList comps = l[i].split(':');
        if (comps.size() != 2)
            throw QString(QObject::tr("Invalid response from the chip"));
        if (comps[0] == "LB")
            data[0] = comps[1].toUInt(0, 16);
        if (comps[0] == "OCR")
            data[1] = comps[1].toUInt(0, 16);
    }
}
