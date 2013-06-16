#include "usbshupito23conn.h"
#include "../misc/utils.h"
#include <stdexcept>
#include <QEvent>
#include <QCoreApplication>

UsbShupito23Connection::UsbShupito23Connection(yb::async_runner & runner)
    : ShupitoConnection(CONNECTION_SHUPITO23), m_runner(runner)
{
    connect(&m_incomingPackets, SIGNAL(dataReceived()), this, SLOT(incomingPacketsReceived()));
    connect(&m_sendCompleted, SIGNAL(dataReceived()), this, SLOT(sendCompleted()));
    this->markMissing();
}

static QString fromUtf8(std::string const & s)
{
    return QString::fromUtf8(s.data(), s.size());
}

static QString formatDeviceDetails(yb::usb_device const & dev)
{
    return QString("SN %1").arg(fromUtf8(dev.serial_number()));
}

QString UsbShupito23Connection::details() const
{
    return m_details;
}

void UsbShupito23Connection::setup(yb::usb_device_interface const & intf)
{
    assert(!intf.empty());
    yb::usb_interface const & idesc = intf.descriptor();
    assert(idesc.altsettings.size() == 1);

    yb::usb_interface_descriptor const & desc = idesc.altsettings[0];

    m_intf = intf;
    m_out_ep = 0;
    m_in_eps.clear();

    for (size_t i = 0; i < desc.endpoints.size(); ++i)
    {
        if (desc.endpoints[i].is_output())
        {
            m_out_ep = desc.endpoints[i].bEndpointAddress;
            m_out_ep_size = desc.endpoints[i].wMaxPacketSize;
        }
        else
            m_in_eps.push_back(desc.endpoints[i].bEndpointAddress);
    }

    m_details = formatDeviceDetails(intf.device());

    QByteArray raw_desc;
    for (size_t i = 0; i < desc.extra_descriptors.size(); ++i)
    {
        if (desc.extra_descriptors[i].size() < 2)
            continue;

        if (desc.extra_descriptors[i][1] == 75)
            raw_desc.append((char const *)desc.extra_descriptors[i].data() + 2, desc.extra_descriptors[i].size() - 2);
    }

    m_desc.Clear();
    m_desc.AddData(raw_desc);
    emit descRead(m_desc);

    this->markPresent();
}

void UsbShupito23Connection::clear()
{
    this->closeImpl();
    m_intf.clear();
    this->markMissing();
}

void UsbShupito23Connection::closeImpl()
{
    if (this->state() == st_connected)
    {
        m_write_loop.wait(yb::cl_abort);
        for (size_t i = 0; i < m_in_eps.size(); ++i)
            m_read_loops[i].read_loop.wait(yb::cl_abort);
        m_read_loops.reset();
    }

    m_intf_guard.release();
}

void UsbShupito23Connection::doOpen()
{
    yb::usb_device dev = m_intf.device();
    if (!m_intf_guard.claim(dev, m_intf.interface_index()))
        return Utils::showErrorBox("Cannot claim the interface");

    m_write_loop = m_runner.post(yb::loop([this](yb::cancel_level cl) -> yb::task<void> {
        if (cl >= yb::cl_abort || (cl >= yb::cl_quit && m_write_channel.empty()))
        {
            m_sendCompleted.send();
            return yb::nulltask;
        }
        return this->write_loop();
    }));

    m_read_loops.reset(new read_loop_ctx[m_in_eps.size()]);
    for (size_t i = 0; i < m_in_eps.size(); ++i)
    {
        m_read_loops[i].read_loop = m_runner.post(yb::loop([this, i](yb::cancel_level cl) -> yb::task<void> {
            return cl < yb::cl_quit? this->read_loop(i): yb::nulltask;
        }));
    }

    this->SetState(st_connected);
}

void UsbShupito23Connection::doClose()
{
    if (this->state() == st_disconnecting)
    {
        this->closeImpl();
        this->SetState(st_disconnected);
    }
    else
    {
        emit disconnecting();
        this->SetState(st_disconnecting);
        m_write_loop.cancel(yb::cl_quit);
    }
}

void UsbShupito23Connection::sendPacket(ShupitoPacket const & packet)
{
    m_write_channel.send(packet);
}

void UsbShupito23Connection::requestDesc()
{
    emit descRead(m_desc);
}

yb::task<void> UsbShupito23Connection::write_loop()
{
    m_write_loop_ctx.packet_index = 0;
    m_write_loop_ctx.packets.clear();
    return m_write_channel.receive(m_write_loop_ctx.packets).finish_on(yb::cl_quit).then([this]() -> yb::task<void> {
        return this->write_packets();
    });
}

yb::task<void> UsbShupito23Connection::write_packets()
{
    return yb::loop([this](yb::cancel_level cl) -> yb::task<void> {
        if (cl >= yb::cl_quit || m_write_loop_ctx.packet_index >= m_write_loop_ctx.packets.size())
        {
            m_write_loop_ctx.packets.clear();
            return yb::nulltask;
        }

        ShupitoPacket & packet = m_write_loop_ctx.packets[m_write_loop_ctx.packet_index++];
        if (packet.size() == 256)
            return m_intf.device().bulk_write(m_out_ep, packet.data(), packet.size()).ignore_result();
        else
            return m_intf.device().bulk_write_zlp(m_out_ep, packet.data(), packet.size(), m_out_ep_size).ignore_result();
    });
}

yb::task<void> UsbShupito23Connection::read_loop(uint8_t i)
{
    return m_intf.device().bulk_read(m_in_eps[i], m_read_loops[i].read_buffer, sizeof m_read_loops[i].read_buffer).then([this, i](size_t r) -> yb::task<void> {
        if (r != 0)
            m_incomingPackets.send(ShupitoPacket(m_read_loops[i].read_buffer, m_read_loops[i].read_buffer + r));
        return yb::async::value();
    });
}

void UsbShupito23Connection::incomingPacketsReceived()
{
    std::vector<ShupitoPacket> packets;
    m_incomingPackets.receive(packets);
    for (size_t i = 0; i < packets.size(); ++i)
        emit this->packetRead(packets[i]);
}

bool UsbShupito23Connection::getFirmwareDetails(ShupitoFirmwareDetails & details) const
{
    if (ShupitoDesc::config const * c = m_desc.getConfig("c49124d9-4629-4aef-ae35-ddc32c21b279"))
    {
        if (c->data.size() != 15 || c->data[0] != 1)
            return false;

        details.hw_major = c->data[1];
        details.hw_minor = c->data[2];
        uint32_t fw_timestamp = c->data[3] | (c->data[4] << 8) | (c->data[5] << 16) | (c->data[6] << 24);
        details.fw_timestamp.setTimeSpec(Qt::UTC);
        details.fw_timestamp.setTime_t(fw_timestamp);
        details.fw_zone_offset = (int16_t)(c->data[7] | (c->data[8] << 8));
        details.fw_timestamp = details.fw_timestamp.addSecs(details.fw_zone_offset * 60);

        QByteArray fw_revision((char const *)c->data.data() + 9, 6);
        details.fw_revision = fw_revision.toHex();
        return true;
    }

    return false;
}

void UsbShupito23Connection::sendCompleted()
{
    if (this->state() == st_disconnecting)
        this->Close();
}

int UsbShupito23Connection::vid() const
{
    if(m_intf.empty())
        return 0;
    return m_intf.device().vidpid() >> 16;
}

int UsbShupito23Connection::pid() const
{
    if(m_intf.empty())
        return 0;
    return m_intf.device().vidpid() & 0xFFFF;
}

QString UsbShupito23Connection::serialNumber() const
{
    if(m_intf.empty())
        return QString();
    return QString::fromUtf8(m_intf.device().serial_number().c_str());
}

QString UsbShupito23Connection::intfName() const
{
    if(m_intf.empty())
        return QString();

    QString name = QString::fromUtf8(m_intf.name().c_str());
    if (name.isEmpty())
        name = QString("#%1").arg(m_intf.interface_index());
    return name;
}

QHash<QString, QVariant> UsbShupito23Connection::config() const
{
    QHash<QString, QVariant> cfg = ShupitoConnection::config();
    cfg["vid"] = this->vid();
    cfg["pid"] = this->pid();
    cfg["serial_number"] = this->serialNumber();
    cfg["intf_name"] = this->intfName();
    return cfg;
}
