#include "usbacmconn.h"
#include "genericusbconn.h"
#include "connectionmgr2.h"
#include <libyb/async/sync_runner.hpp>
#include <QEvent>
#include <QCoreApplication>
#include <QString>

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
    : PortConnection(CONNECTION_USB_ACM2), m_runner(runner), m_enumerated(false), m_vid(0), m_pid(0), m_baudrate(115200)
{
}

static QString fromUtf8(std::string const & s)
{
    return QString::fromUtf8(s.data(), s.size());
}

static void extractEndpoints(yb::usb_interface const & idesc, int & inep, int & outep)
{
    inep = 0;
    outep = 0;

    if (idesc.altsettings.empty())
        return;

    yb::usb_interface_descriptor desc = idesc.altsettings[0];
    for (size_t i = 0; i < desc.endpoints.size(); ++i)
    {
        if (inep == 0 && desc.endpoints[i].is_input())
            inep = desc.endpoints[i].bEndpointAddress;
        if (outep == 0 && desc.endpoints[i].is_output())
            outep = desc.endpoints[i].bEndpointAddress;
    }
}

void UsbAcmConnection2::setIntf(yb::usb_device_interface const & intf)
{
    m_enumerated_intf = intf;

    if (!m_enumerated_intf.empty())
    {
        yb::usb_device const & dev = m_enumerated_intf.device();

        yb::usb_device_descriptor const & desc = dev.descriptor();
        m_vid = desc.idVendor;
        m_pid = desc.idProduct;
        m_serialNumber = fromUtf8(dev.serial_number());
        m_intfName = fromUtf8(m_enumerated_intf.name());
        if (m_intfName.isEmpty())
            m_intfName = QString("#%1").arg(m_enumerated_intf.interface_index());
        emit changed();

        this->SetState(st_disconnected);

        QString productName = fromUtf8(dev.product());
        QString manufacturerName = fromUtf8(dev.manufacturer());

        int inep, outep;
        extractEndpoints(m_enumerated_intf.descriptor(), inep, outep);

        QStringList res;
        res.push_back(QString("USB ACM %1.%2, %3:%4").arg(intf.config_value()).arg(intf.interface_index()).arg(outep, 2, 16, QChar('0')).arg(inep, 2, 16, QChar('0')));
        if (!productName.isEmpty())
            res.push_back(QString("%1:%2").arg(desc.idVendor, 4, 16, QChar('0')).arg(desc.idProduct, 4, 16, QChar('0')));
        if (!manufacturerName.isEmpty())
            res.push_back(manufacturerName);
        if (!m_serialNumber.isEmpty())
            res.push_back(m_serialNumber);
        m_details = res.join(", ");
    }
}

void UsbAcmConnection2::clear()
{
    this->cleanupWorkers();
    m_intf.clear();
    if (m_enumerated)
        this->SetState(st_removed);
}

UsbAcmConnection2::~UsbAcmConnection2()
{
    this->Close();
}

yb::task<void> UsbAcmConnection2::write_loop(int outep)
{
    m_sent = 0;
    return yb::loop<size_t>(yb::async::value((size_t)0), [this, outep](size_t r, yb::cancel_level cl) -> yb::task<size_t> {
        m_sent += r;
        if (cl >= yb::cl_quit || m_sent == m_write_buffer.size())
            return yb::nulltask;
        return m_intf.device().bulk_write(outep, m_write_buffer.data() + m_sent, m_write_buffer.size() - m_sent);
    });
}

yb::task<void> UsbAcmConnection2::send_loop(int outep)
{
    return m_send_channel.receive(m_write_buffer).then([this, outep]() {
        return this->write_loop(outep);
    });
}

struct line_coding_struct
{
public:
    explicit line_coding_struct(uint32_t dwDTERate, uint8_t bCharFormat = 0, uint8_t bParityType = 0, uint8_t bDataBits = 8)
    {
        m_buffer[0] = dwDTERate;
        m_buffer[1] = dwDTERate >> 8;
        m_buffer[2] = dwDTERate >> 16;
        m_buffer[3] = dwDTERate >> 24;

        m_buffer[4] = bCharFormat;
        m_buffer[5] = bParityType;
        m_buffer[6] = bDataBits;
    }

    uint8_t const * data() const { return m_buffer; }
    size_t size() const { return sizeof m_buffer; }

private:
    uint8_t m_buffer[7];
};

void UsbAcmConnection2::OpenConcurrent()
{
    if (this->state() != st_removed && this->state() != st_connected)
    {
        m_intf = m_enumerated_intf;
        if (m_intf.empty())
        {
            m_intf = sConMgr2.lookupUsbAcmConn(m_vid, m_pid, m_serialNumber, m_intfName);
            if (m_intf.empty())
                return Utils::showErrorBox(tr("Cannot find the USB interface."), 0);
        }

        yb::usb_interface_descriptor const & desc = m_intf.descriptor().altsettings[0];

        int inep, outep;
        extractEndpoints(m_enumerated_intf.descriptor(), inep, outep);

        if (!m_intf.device().claim_interface(m_intf.interface_index()))
            return Utils::showErrorBox(tr("Cannot open the USB interface."), 0);

        this->SetState(st_connected);

        assert(m_receive_worker.empty());
        assert(m_send_worker.empty());

        if (inep)
        {
            m_receive_worker = m_runner.post(yb::loop<size_t>(yb::async::value((size_t)0), [this, inep](size_t r, yb::cancel_level cl) -> yb::task<size_t> {
                if (r > 0)
                {
                    QEvent * ev = new RecvEvent(yb::buffer_ref(m_read_buffer, r));
                    QCoreApplication::instance()->postEvent(this, ev);
                }
                return cl >= yb::cl_quit? yb::nulltask: m_intf.device().bulk_read(inep, m_read_buffer, sizeof m_read_buffer);
            }));
        }

        if (outep)
        {
            m_send_worker = m_runner.post(yb::loop([this, outep](yb::cancel_level cl) -> yb::task<void> {
                return cl >= yb::cl_quit? yb::nulltask: this->send_loop(outep);
            }));
        }

        // {ea5c3c23-ea74-f841-bfa2-8e1983e796be}
        static uint8_t const sig[] = { 0xea, 0x5c, 0x3c, 0x23, 0xea, 0x74, 0xf8, 0x41, 0xbf, 0xa2, 0x8e, 0x19, 0x83, 0xe7, 0x96, 0xbe };
        std::vector<uint8_t> extra_desc = desc.lookup_extra_descriptor(75, yb::buffer_ref(sig, sig + sizeof sig));
        m_configurable = !extra_desc.empty();

        if (m_configurable)
        {
            line_coding_struct payload(m_baudrate);
            yb::usb_control_code_t set_line_coding = { 0x21, 0x20 };
            m_runner.try_run(m_intf.device().control_write(set_line_coding, 0, m_intf.interface_index(), payload.data(), payload.size()));
        }
    }
}

void UsbAcmConnection2::Close()
{
    this->cleanupWorkers();
    if (this->state() == st_connected)
    {
        if (m_configurable)
        {
            yb::usb_control_code_t set_control_line_state = { 0x21, 0x22 };
            m_runner.try_run(m_intf.device().control_write(set_control_line_state, 0, m_intf.interface_index(), 0, 0));
        }

        m_intf.device().release_interface(m_intf.interface_index());
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
    if (!m_send_worker.empty() && this->state() == st_connected)
        m_send_channel.send(yb::buffer_ref((uint8_t const *)data.data(), data.size()));
}

ConnectionPointer<Connection> UsbAcmConnection2::clone()
{
    ConnectionPointer<UsbAcmConnection2> conn(new UsbAcmConnection2(m_runner));
    conn->setName(tr("Clone of ") + this->name());
    conn->setVid(this->vid());
    conn->setPid(this->pid());
    conn->setSerialNumber(this->serialNumber());
    conn->setIntfName(this->intfName());
    conn->setBaudRate(this->baudRate());
    return conn;
}

QHash<QString, QVariant> UsbAcmConnection2::config() const
{
    QHash<QString, QVariant> res = this->Connection::config();
    res["vid"] = this->vid();
    res["pid"] = this->pid();
    res["serial_number"] = this->serialNumber();
    res["intf_name"] = this->intfName();
    res["baud_rate"] = this->baudRate();
    return res;
}

bool UsbAcmConnection2::applyConfig(QHash<QString, QVariant> const & config)
{
    this->setVid(config.value("vid", 0).toInt());
    this->setPid(config.value("pid", 0).toInt());
    this->setSerialNumber(config.value("serial_number").toString());
    this->setIntfName(config.value("intf_name").toString());
    this->setBaudRate(config.value("baud_rate", 115200).toInt());
    return this->Connection::applyConfig(config);
}
