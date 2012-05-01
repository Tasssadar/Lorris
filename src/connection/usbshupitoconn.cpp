#include "usbshupitoconn.h"
#include "libusb/lusb0_usb_dyn.h"
#include <QStringBuilder>
#include <QStringList>

namespace {

class ReadThread : public QThread
{
public:
    void run()
    {

    }
};

}

struct ScopedUsbHandle
{
    explicit ScopedUsbHandle(libusb0_methods * um, usb_dev_handle * h = 0)
        : m_um(um), h(h)
    {
    }

    ~ScopedUsbHandle()
    {
        if (h)
            m_um->usb_close(h);
    }

    usb_dev_handle * take()
    {
        usb_dev_handle * res = h;
        h = 0;
        return res;
    }

    libusb0_methods * m_um;
    usb_dev_handle * h;
};

UsbAcmConnection::UsbAcmConnection(libusb0_methods * um)
    : PortConnection(CONNECTION_USB_ACM), m_um(um), m_dev(0), m_handle(0)
{
    this->Close();
}

void UsbAcmConnection::OpenConcurrent()
{
    this->SetState(st_connecting);
    bool success = this->openImpl();
    this->SetState(success? st_connected: st_disconnected);
}

bool UsbAcmConnection::openImpl()
{
    if (!m_dev || !isDeviceSupported(m_dev))
        return false;

    ScopedUsbHandle h(m_um, m_um->usb_open(m_dev));
    if (!h.h)
        return false;

    if (m_um->usb_claim_interface(h.h, 1) < 0)
        return false;

    m_readThread = new ReadThread();

    m_write_ep = m_dev->config->interface[1].altsetting[0].endpoint[0].bEndpointAddress;
    m_read_ep = m_dev->config->interface[1].altsetting[0].endpoint[1].bEndpointAddress;
    m_handle = h.take();
    return true;
}

void UsbAcmConnection::Close()
{
    if (!m_handle)
        return;
    m_um->usb_close(m_handle);
    m_handle = 0;

    m_readThread->wait();
    this->SetState(st_disconnected);
}

bool UsbAcmConnection::setUsbDevice(struct usb_device * dev)
{
    if (this->state() == st_connected)
        this->Close();

    Q_ASSERT(this->state() == st_disconnected);
    m_dev = dev;
    return this->updateStrings();
}

void UsbAcmConnection::SendData(const QByteArray & data)
{
    if (!m_handle)
        return;

    int res = m_um->usb_bulk_write(m_handle, m_write_ep, data.data(), data.size(), 0);
}

QString UsbAcmConnection::details() const
{
    QStringList list;

    QString res = PortConnection::details();
    if (!res.isEmpty())
        list << res;
    if (!m_manufacturer.isEmpty())
        list << m_manufacturer;
    if (!m_serialNumber.isEmpty())
        list << m_serialNumber;

    return list.join(", ");
}

static QString getUsbString(libusb0_methods * um, struct usb_dev_handle * h, int index, int langid)
{
    char buf[1024];
    int len = um->usb_get_string(h, index, langid, buf, sizeof buf - 2);
    if (len > 2 && len % 2 == 0)
    {
        buf[len] = 0;
        buf[len+1] = 0;
        return QString::fromUtf16((ushort const *)buf + 1);
    }

    return QString();
}

bool UsbAcmConnection::updateStrings()
{
    if (!m_dev)
        return false;

    usb_dev_handle * h = m_um->usb_open(m_dev);
    if (!h)
        return false;

    char buf[128];
    int len = m_um->usb_get_descriptor(h, USB_DT_STRING, 0, buf, sizeof buf);
    if (len < 4)
    {
        m_um->usb_close(h);
        return false;
    }

    int langid = (buf[3] << 8) | buf[2];
    if (m_dev->descriptor.iManufacturer)
        m_manufacturer = getUsbString(m_um, h, m_dev->descriptor.iManufacturer, langid);

    if (m_dev->descriptor.iProduct)
        m_product = getUsbString(m_um, h, m_dev->descriptor.iProduct, langid);

    if (m_dev->descriptor.iSerialNumber)
        m_serialNumber = getUsbString(m_um, h, m_dev->descriptor.iSerialNumber, langid);

    m_um->usb_close(h);
    return true;
}

bool UsbAcmConnection::isDeviceSupported(struct usb_device * dev)
{
    Q_ASSERT(dev);
    if (dev->descriptor.idVendor == 0x4a61 && dev->descriptor.idProduct == 0x679a)
    {
        // FIXME: check that the descriptor is consistent
        if (dev->config->bNumInterfaces == 2)
            return true;
    }

    return false;
}
