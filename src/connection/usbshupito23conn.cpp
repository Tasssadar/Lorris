#include "usbshupito23conn.h"
#include "../misc/utils.h"
#include <stdexcept>
#include <QEvent>
#include <QCoreApplication>

// XXX: st_removed

namespace {

class RecvEvent
    : public QEvent
{
public:
    static const QEvent::Type recvType = static_cast<QEvent::Type>(2000);

    RecvEvent(yb::buffer_ref const & buf)
        : QEvent(recvType), m_packet(buf.begin(), buf.end())
    {
    }

    ShupitoPacket m_packet;
};

}

UsbShupito23Connection::UsbShupito23Connection(yb::async_runner & runner)
    : ShupitoConnection(CONNECTION_SHUPITO23), m_runner(runner)
{
    this->SetState(st_removed);
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
            m_out_ep = desc.endpoints[i].bEndpointAddress;
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

    this->SetState(st_disconnected);
}

void UsbShupito23Connection::clear()
{
    m_intf_guard.release();
    m_intf.clear();
    this->SetState(st_removed);
}

void UsbShupito23Connection::OpenConcurrent()
{
    yb::usb_device dev = m_intf.device();
    if (!m_intf_guard.claim(dev, m_intf.interface_index()))
        return Utils::showErrorBox("Cannot claim the interface");

    m_write_loop = m_runner.post(yb::loop([this](yb::cancel_level cl) -> yb::task<void> {
        return cl < yb::cl_quit? this->write_loop(): yb::nulltask;
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

void UsbShupito23Connection::Close()
{
    if (this->state() == st_connected)
    {
        m_write_loop.wait(yb::cl_abort);
        for (size_t i = 0; i < m_in_eps.size(); ++i)
            m_read_loops[i].read_loop.wait(yb::cl_abort);
        m_read_loops.reset();
    }

    m_intf_guard.release();
    this->SetState(st_disconnected);
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
    return m_write_channel.receive(m_write_loop_ctx.packets).then([this]() -> yb::task<void> {
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
        return m_intf.device().bulk_write(m_out_ep, packet.data(), packet.size()).ignore_result();
    });
}

yb::task<void> UsbShupito23Connection::read_loop(uint8_t i)
{
    return m_intf.device().bulk_read(m_in_eps[i], m_read_loops[i].read_buffer, sizeof m_read_loops[i].read_buffer).then([this, i](size_t r) -> yb::task<void> {
        RecvEvent * ev = new RecvEvent(yb::buffer_ref(m_read_loops[i].read_buffer, r));
        QCoreApplication::instance()->postEvent(this, ev);
        return yb::async::value();
    });
}

bool UsbShupito23Connection::event(QEvent * ev)
{
    if (ev->type() == RecvEvent::recvType)
    {
        emit this->packetRead(static_cast<RecvEvent *>(ev)->m_packet);
        return true;
    }
    else
    {
        return this->ShupitoConnection::event(ev);
    }
}
