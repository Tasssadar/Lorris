#include "usbacmconn.h"
#include "genericusbconn.h"
#include "connectionmgr2.h"
#include <libyb/async/sync_runner.hpp>
#include <libyb/async/double_buffer.hpp>
#include <QEvent>
#include <QCoreApplication>
#include <QString>

UsbAcmConnection2::UsbAcmConnection2(yb::async_runner & runner)
    : PortConnection(CONNECTION_USB_ACM2), m_runner(runner), m_enumerated(false), m_vid(0), m_pid(0), m_baudrate(115200)
{
    connect(&m_incomingDataChannel, SIGNAL(dataReceived()), this, SLOT(incomingDataReady()));
    this->markMissing();
}

static QString fromUtf8(std::string const & s)
{
    return QString::fromUtf8(s.data(), s.size());
}

static void extractEndpoints(yb::usb_interface const & idesc, int & inep, size_t & inepsize, int & outep)
{
    inep = 0;
    outep = 0;

    if (idesc.altsettings.empty())
        return;

    yb::usb_interface_descriptor desc = idesc.altsettings[0];
    for (size_t i = 0; i < desc.endpoints.size(); ++i)
    {
        if (inep == 0 && desc.endpoints[i].is_input())
        {
            inep = desc.endpoints[i].bEndpointAddress;
            inepsize = desc.endpoints[i].wMaxPacketSize;
        }

        if (outep == 0 && desc.endpoints[i].is_output())
            outep = desc.endpoints[i].bEndpointAddress;
    }
}

void UsbAcmConnection2::setEnumeratedIntf(yb::usb_device_interface const & intf)
{
    Q_ASSERT(!intf.empty());
    m_enumerated = true;

    yb::usb_device const & dev = intf.device();
    yb::usb_device_descriptor const & desc = dev.descriptor();
    m_vid = desc.idVendor;
    m_pid = desc.idProduct;
    m_serialNumber = fromUtf8(dev.serial_number());
    m_intfName = formatIntfName(intf);
    emit changed();

    this->setIntf(intf);
}

void UsbAcmConnection2::clearEnumeratedIntf()
{
    this->setIntf(yb::usb_device_interface());
}

void UsbAcmConnection2::setIntf(yb::usb_device_interface const & intf)
{
    m_intf = intf;

    if (!m_intf.empty())
    {
        yb::usb_device const & dev = m_intf.device();

        yb::usb_device_descriptor const & desc = dev.descriptor();

        QString productName = fromUtf8(dev.product());
        QString manufacturerName = fromUtf8(dev.manufacturer());

        int inep, outep;
        size_t inepsize;
        extractEndpoints(m_intf.descriptor(), inep, inepsize, outep);

        QStringList res;
        res.push_back(QString("USB ACM %1.%2, %3:%4").arg(intf.config_value()).arg(intf.interface_index()).arg(outep, 2, 16, QChar('0')).arg(inep, 2, 16, QChar('0')));
        if (!productName.isEmpty())
            res.push_back(QString("%1:%2").arg(desc.idVendor, 4, 16, QChar('0')).arg(desc.idProduct, 4, 16, QChar('0')));
        if (!manufacturerName.isEmpty())
            res.push_back(manufacturerName);
        if (!m_serialNumber.isEmpty())
            res.push_back(m_serialNumber);

        m_details = res.join(", ");
        this->markPresent();
    }
    else
    {
        this->cleanupWorkers();
        this->markMissing();
    }
}

void UsbAcmConnection2::notifyIntfPlugin(yb::usb_device_interface const & intf)
{
    if (!m_enumerated && this->isMissing())
    {
        yb::usb_device const & dev = intf.device();
        yb::usb_device_descriptor const & desc = dev.descriptor();

        if (desc.idVendor == m_vid && desc.idProduct == m_pid
            && QString::fromUtf8(intf.device().serial_number().c_str()) == m_serialNumber
            && formatIntfName(intf) == m_intfName)
        {
            this->setIntf(intf);
        }
    }
}

void UsbAcmConnection2::notifyIntfUnplug(yb::usb_device_interface const & intf)
{
    if (!m_enumerated && m_intf == intf)
        this->setIntf(yb::usb_device_interface());
}

QString UsbAcmConnection2::formatIntfName(yb::usb_device_interface const & intf)
{
    QString name = QString::fromUtf8(intf.name().c_str());
    if (name.isEmpty())
        name = QString("#%1").arg(intf.interface_index());
    return name;
}

void UsbAcmConnection2::clear()
{
    this->cleanupWorkers();
    m_intf.clear();
    if (m_enumerated)
        this->markMissing();
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

void UsbAcmConnection2::doOpen()
{
    if (m_intf.empty())
    {
        m_intf = sConMgr2.lookupUsbAcmConn(m_vid, m_pid, m_serialNumber, m_intfName);
        if (m_intf.empty())
            return Utils::showErrorBox(tr("Cannot find the USB interface."), 0);
    }

    yb::usb_interface_descriptor const & desc = m_intf.descriptor().altsettings[0];

    int inep, outep;
    size_t inepsize;
    extractEndpoints(m_intf.descriptor(), inep, inepsize, outep);

    Q_ASSERT(inepsize <= sizeof m_read_buffers[0]);

    if (!m_intf.device().claim_interface(m_intf.interface_index()))
        return Utils::showErrorBox(tr("Cannot open the USB interface."), 0);

    assert(m_receive_worker.empty());
    assert(m_send_worker.empty());

    if (inep)
    {
#if 1
        // Note that double buffering seems to work, but
        // quadruple buffering will sometimes kill the driver (a bug perhaps?)
        // so that no more transactions on the pipe go through
        // until the device is reconnected.
        m_receive_worker = m_runner.post(yb::double_buffer<size_t>([this, inep, inepsize](size_t i) {
            return m_intf.device().bulk_read(inep, m_read_buffers[i], inepsize);
        }, [this](size_t i, size_t r) {
            if (r > 0)
                m_incomingDataChannel.send(m_read_buffers[i], m_read_buffers[i] + r);
        }, read_buffer_count));
#else
        m_receive_worker = m_runner.post(yb::loop<size_t>(yb::async::value((size_t)0), [this, inep, inepsize](size_t r, yb::cancel_level cl) -> yb::task<size_t> {
            if (r > 0)
                m_incomingDataChannel.send(m_read_buffers[0], m_read_buffers[0] + r);
            return cl >= yb::cl_quit? yb::nulltask: m_intf.device().bulk_read(inep, m_read_buffers[0], inepsize);
        }));
#endif
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

    this->SetState(st_connected);
}

void UsbAcmConnection2::doClose()
{
    Q_ASSERT(this->state() == st_connected);
    this->cleanupWorkers();
    if (m_configurable)
    {
        yb::usb_control_code_t set_control_line_state = { 0x21, 0x22 };
        m_runner.try_run(m_intf.device().control_write(set_control_line_state, 0, m_intf.interface_index(), 0, 0));
    }

    m_intf.device().release_interface(m_intf.interface_index());
    this->SetState(st_disconnected);

    if (!m_enumerated)
        m_intf.clear();
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

void UsbAcmConnection2::incomingDataReady()
{
    std::vector<uint8_t> data;
    m_incomingDataChannel.receive(data);
    emit this->dataRead(QByteArray((char const *)data.data(), data.size()));
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

void UsbAcmConnection2::updateIntf()
{
    Q_ASSERT(!m_enumerated && (this->state() == st_disconnected || this->state() == st_missing));
    emit changed();

    yb::usb_device_interface intf = sConMgr2.lookupUsbAcmConn(m_vid, m_pid, m_serialNumber, m_intfName);
    if (!intf.empty())
        this->markPresent();
    else
        this->markMissing();
}

void UsbAcmConnection2::setBaudRate(int value)
{
    if (m_baudrate != value)
    {
        m_baudrate = value;
        emit changed();

        if (m_configurable && this->state() == st_connected)
        {
            line_coding_struct payload(m_baudrate);
            yb::usb_control_code_t set_line_coding = { 0x21, 0x20 };
            m_runner.try_run(m_intf.device().control_write(set_line_coding, 0, m_intf.interface_index(), payload.data(), payload.size()));
        }
    }
}
