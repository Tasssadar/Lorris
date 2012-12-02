#include "usbacmconn.h"
#include <libyb/async/sync_runner.hpp>
#include <QEvent>
#include <QCoreApplication>

namespace {

class RecvEvent
    : public QEvent
{
public:
    static const QEvent::Type recvType = static_cast<QEvent::Type>(2000);

    RecvEvent(yb::buffer_ref const & buf)
        : QEvent(recvType), m_data((char const *)buf.data(), buf.size())
    {
    }

    QByteArray m_data;
};

}

UsbAcmConnection2::UsbAcmConnection2(yb::async_runner & runner)
    : PortConnection(CONNECTION_USB_ACM2), m_runner(runner)
{
    this->SetState(st_removed);
}

void UsbAcmConnection2::setup(yb::usb_device const & dev, uint8_t intf, uint8_t outep, uint8_t inep)
{
    m_dev = dev;
    m_intf = intf;
    m_outep = outep;
    m_inep = inep;
    this->SetState(st_disconnected);
}

void UsbAcmConnection2::clear()
{
    this->cleanupWorkers();
    m_dev.clear();
    this->SetState(st_removed);
}

UsbAcmConnection2::~UsbAcmConnection2()
{
    this->Close();
}

yb::task<void> UsbAcmConnection2::write_loop()
{
    m_sent = 0;
    return yb::loop<size_t>(yb::async::value((size_t)0), [this](size_t r, yb::cancel_level cl) -> yb::task<size_t> {
        m_sent += r;
        if (cl >= yb::cl_quit || m_sent == m_write_buffer.size())
            return yb::nulltask;
        return m_dev.bulk_write(m_outep, m_write_buffer.data() + m_sent, m_write_buffer.size() - m_sent);
    });
}

yb::task<void> UsbAcmConnection2::send_loop()
{
    return m_send_channel.receive(m_write_buffer).then([this]() {
        return this->write_loop();
    });
}

void UsbAcmConnection2::OpenConcurrent()
{
    if (this->state() != st_connected)
    {
        if (!m_dev.claim_interface(m_intf))
            return Utils::showErrorBox(tr("Cannot open the USB device."), 0);

        this->SetState(st_connected);

        assert(m_receive_worker.empty());
        assert(m_send_worker.empty());

        if (m_inep)
        {
            m_receive_worker = m_runner.post(yb::loop<size_t>(yb::async::value((size_t)0), [this](size_t r, yb::cancel_level cl) -> yb::task<size_t> {
                if (r > 0)
                {
                    QEvent * ev = new RecvEvent(yb::buffer_ref(m_read_buffer, r));
                    QCoreApplication::instance()->postEvent(this, ev);
                }
                return cl >= yb::cl_quit? yb::nulltask: m_dev.bulk_read(m_inep | 0x80, m_read_buffer, sizeof m_read_buffer);
            }));
        }

        if (m_outep)
        {
            m_send_worker = m_runner.post(yb::loop([this](yb::cancel_level cl) -> yb::task<void> {
                return cl >= yb::cl_quit? yb::nulltask: this->send_loop();
            }));
        }
    }
}

void UsbAcmConnection2::Close()
{
    this->cleanupWorkers();
    if (this->state() == st_connected)
    {
        m_dev.release_interface(m_intf);
        this->SetState(st_disconnected);
    }
}

void UsbAcmConnection2::cleanupWorkers()
{
    if (!m_receive_worker.empty())
    {
        m_receive_worker.cancel(yb::cl_abort);
        m_receive_worker.try_get();
    }

    if (!m_send_worker.empty())
    {
        m_send_worker.cancel(yb::cl_abort);
        m_send_worker.try_get();
    }

    m_send_channel.clear();
    m_write_buffer.clear();
}

bool UsbAcmConnection2::event(QEvent * ev)
{
    if (ev->type() == RecvEvent::recvType)
    {
        emit this->dataRead(static_cast<RecvEvent *>(ev)->m_data);
        return true;
    }
    else
    {
        return this->PortConnection::event(ev);
    }
}

void UsbAcmConnection2::SendData(const QByteArray & data)
{
    if (m_outep && this->state() == st_connected)
        m_send_channel.send(yb::buffer_ref((uint8_t const *)data.data(), data.size()));
}
