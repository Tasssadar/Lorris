/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "shupitojtag.h"
#include "../shupito.h"
#include "../../misc/utils.h"
#include <sstream>
#include <cassert>

ShupitoJtag::ShupitoJtag(Shupito *shupito)
    : ShupitoMode(shupito)
{
}

ProgrammerCapabilities ShupitoJtag::capabilities() const
{
    ProgrammerCapabilities caps;
    caps.svf = true;
    return caps;
}

ShupitoDesc::config const * ShupitoJtag::getModeCfg()
{
    return m_shupito->getDesc()->getConfig("fe047e35-dec8-48ab-b194-e3762c8f6b66");
}

void ShupitoJtag::switchToFlashMode(quint32 speed_hz)
{
    m_flash_mode = false;

    ShupitoDesc::config const *prog_cfg = getModeCfg();
    if (!prog_cfg)
        throw QString(QObject::tr("The device can't program these types of chips."));

    m_shupito->sendPacket(prog_cfg->getStateChangeCmd(true));

    m_freq_base = 0;
    if (prog_cfg->data.size() >= 5 && prog_cfg->data[0] == 1)
        m_freq_base = deserialize_le<uint32_t>(prog_cfg->data.data() + 1);

    uint32_t max_freq = speed_hz;
    if (prog_cfg->data.size() >= 9 && prog_cfg->data[0] == 1)
        max_freq = deserialize_le<uint32_t>(prog_cfg->data.data() + 5);

    m_prog_cmd_base = prog_cfg->cmd;

    m_max_freq_hz = (std::min)(speed_hz, max_freq);
    this->cmd_frequency(m_max_freq_hz);

    m_flash_mode = true;
}

void ShupitoJtag::switchToRunMode()
{
    if (!m_flash_mode)
        return;

    ShupitoDesc::config const *prog_cfg = getModeCfg();
    Q_ASSERT(prog_cfg != 0);
    m_shupito->sendPacket(prog_cfg->getStateChangeCmd(false));
    m_flash_mode = false;
}

chip_definition ShupitoJtag::readDeviceId()
{
    chip_definition cd("jtag:");
    cd.setName("jtag");
    return cd;
}

void ShupitoJtag::erase_device(chip_definition& chip)
{
}

void ShupitoJtag::flashPage(chip_definition::memorydef *memdef, std::vector<quint8>& memory, quint32 address)
{
}

void ShupitoJtag::readMemRange(quint8 memid, QByteArray& memory, quint32 address, quint32 size)
{
}

struct ShupitoJtag::cost_visitor
{
    explicit cost_visitor(ShupitoJtag & parent)
        : current_bit_period(1.0 / parent.m_max_freq_hz), min_bit_period(current_bit_period), total_cost(0)
    {
    }

    void operator()(yb::svf_tms_path const & stmt)
    {
        total_cost += stmt.length * current_bit_period;
    }

    void operator()(yb::svf_xxr const & stmt)
    {
        total_cost += stmt.length * current_bit_period;
    }

    void operator()(yb::svf_frequency const & stmt)
    {
        current_bit_period = (std::max)(1.0 / stmt.cycles_hz, min_bit_period);
    }

    void operator()(yb::svf_runtest const & stmt)
    {
        total_cost += (std::min)(stmt.max_time, (std::max)(stmt.run_count * current_bit_period, stmt.min_time));
    }

    template <typename T>
    void operator()(T const &)
    {
    }

    double current_bit_period;
    double min_bit_period;
    double total_cost;
};

struct ShupitoJtag::play_visitor
{
    explicit play_visitor(ShupitoJtag & parent, double total_cost)
        : parent(parent), current_bit_period(1.0 / parent.m_max_freq_hz), min_bit_period(current_bit_period),
        current_cost(0), total_cost(total_cost)
    {
    }

    void operator()(yb::svf_frequency const & stmt)
    {
        current_bit_period = (std::max)(1.0 / stmt.cycles_hz, min_bit_period);

        uint32_t cycles_hz = (std::min)((uint32_t)stmt.cycles_hz, parent.m_max_freq_hz);
        parent.cmd_frequency(cycles_hz);
    }

    void operator()(yb::svf_xxr const & stmt)
    {
        size_t ms = parent.m_shupito->maxPacketSize();

        size_t length_bits = stmt.length;
        uint8_t const * tdi = stmt.tdi.data();
        uint8_t const * tdo = stmt.tdo.data();
        uint8_t const * mask = stmt.mask.data();
        while (length_bits && !parent.m_cancel_requested)
        {
            size_t chunk_bits = (std::min)(length_bits, (ms - 1) * 8);
            size_t chunk_bytes = (chunk_bits + 7) / 8;

            bool verify = !stmt.tdo.empty();

            ShupitoPacket pkt;
            pkt.push_back(parent.m_prog_cmd_base + 1);
            pkt.push_back(chunk_bits & 0x07);
            if (!verify)
                pkt.back() |= 0x10;
            pkt.insert(pkt.end(), tdi, tdi + chunk_bytes);

            pkt = parent.m_shupito->waitForPacket(pkt, parent.m_prog_cmd_base + 1);
            if (pkt.size() != (!verify? 2: chunk_bytes + 2) || pkt[1] != 0)
                throw QObject::tr("Invalid response received from Shupito");

            if (verify)
            {
                if (chunk_bits % 8)
                    pkt.back() >>= (8-(chunk_bits%8));

                for (size_t i = 0; i < chunk_bytes; ++i)
                {
                    if ((pkt[i+2] & mask[i]) != tdo[i])
                        throw QObject::tr("Verification failed!");
                }

                tdo += chunk_bytes;
                mask += chunk_bytes;
            }

            length_bits -= chunk_bits;
            tdi += chunk_bytes;

            current_cost += chunk_bits * current_bit_period;
            emit parent.updateProgressDialog((int)(current_cost * 100 / total_cost));
        }
    }

    void operator()(yb::svf_tms_path const & stmt)
    {
        size_t ms = parent.m_shupito->maxPacketSize();

        uint8_t const * p = stmt.path.data();
        size_t length_bits = stmt.length;
        while (length_bits && !parent.m_cancel_requested)
        {
            size_t chunk_bits = (std::min)(length_bits, (ms - 1) * 8);
            if (chunk_bits > 248)
                chunk_bits = 248;

            size_t chunk_bytes = (chunk_bits + 7) / 8;

            ShupitoPacket pkt;
            pkt.push_back(parent.m_prog_cmd_base);
            pkt.push_back(chunk_bits);
            pkt.insert(pkt.end(), p, p + chunk_bytes);

            parent.m_shupito->waitForPacket(pkt, parent.m_prog_cmd_base);

            length_bits -= chunk_bits;
            p += chunk_bytes;

            current_cost += chunk_bits * current_bit_period;
            emit parent.updateProgressDialog((int)(current_cost * 100 / total_cost));
        }
    }

    void operator()(yb::svf_runtest const & stmt)
    {
        uint32_t clocks = (uint32_t)(std::min)(stmt.max_time / current_bit_period, (std::max)((double)stmt.run_count, stmt.min_time / current_bit_period));
        uint32_t max_chunk = 1 / current_bit_period;

        while (clocks && !parent.m_cancel_requested)
        {
            uint32_t chunk = (std::min)(clocks, max_chunk);
            clocks -= chunk;

            uint8_t pkt[5] = { parent.m_prog_cmd_base + 3 };
            serialize_le(pkt + 1, chunk);

            ShupitoPacket resp = parent.m_shupito->waitForPacket(ShupitoPacket(pkt, pkt + sizeof pkt), pkt[0]);
            for (;;)
            {
                if (resp.size() == 2)
                {
                    uint8_t error = resp[1];
                    if (error)
                        throw QObject::tr("Something went wrong while executing RUNTEST command.");
                    break;
                }
                else if (resp.size() == 5)
                {
                    uint32_t remaining_clocks = deserialize_le<uint32_t>(resp.data() + 1);
                    emit parent.updateProgressDialog((int)((current_cost + (chunk - remaining_clocks) * current_bit_period) * 100 / total_cost));
                }
                else
                {
                    throw QObject::tr("Invalid response");
                }

                resp = parent.m_shupito->waitForPacket(pkt[0]);
            }

            current_cost += (std::min)(chunk * current_bit_period, (std::max)(stmt.run_count * current_bit_period, stmt.min_time));
        }
    }

    void operator()(yb::svf_trst const & stmt)
    {
        ShupitoPacket resp = parent.m_shupito->waitForPacket(makeShupitoPacket(parent.m_prog_cmd_base + 4, 1, stmt.mode), parent.m_prog_cmd_base + 4);
        if (resp.size() != 2 || resp[1] != 0)
            throw QObject::tr("Something went wrong while executing TRST command.");
    }

#if 0
    void operator()(yb::svf_comment const & stmt)
    {
        if (stmt.comment_kind == yb::svf_comment::ck_bang)
        {
            QString text = QString::fromStdString(stmt.text.substr(1)).trimmed();
            if (!text.isEmpty())
                emit parent.updateProgressLabel(text);
        }
    }

#endif

    template <typename T>
    void operator()(T const &)
    {
    }

    ShupitoJtag & parent;
    double current_bit_period;
    double min_bit_period;
    double current_cost;
    double total_cost;
};

void ShupitoJtag::executeText(QByteArray const & data, quint8 memId, chip_definition & chip)
{
    std::istringstream ss((std::string(data.data(), data.size())));
    yb::svf_file doc = yb::svf_parse(ss);

    doc = yb::svf_lower(doc);

    cost_visitor cv(*this);
    yb::svf_visit(doc, cv);

    emit updateProgressDialog(0);
    play_visitor pv(*this, cv.total_cost);
    try
    {
        m_cancel_requested = false;
        for (size_t i = 0; !m_cancel_requested && i < doc.size(); ++i)
            svf_visit(doc[i].get(), pv);
        emit updateProgressDialog(-1);
    }
    catch (QString const & e)
    {
        emit updateProgressDialog(-1);
        Utils::showErrorBox(e);
    }
}

void ShupitoJtag::cmd_frequency(uint32_t speed_hz)
{
    if (m_freq_base == 0)
        return;

    uint32_t period = (m_freq_base + speed_hz - 1) / speed_hz;

    uint8_t packet[5] = { m_prog_cmd_base + 2 };
    serialize_le(packet + 1, period);
    m_shupito->waitForPacket(ShupitoPacket(packet, packet + sizeof packet), packet[0]);
}
