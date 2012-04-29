#include "usbshupitoconn.h"
#include "libusb/lusb0_usb_dyn.h"
#include <QStringBuilder>
#include <QStringList>

UsbShupitoConnection::UsbShupitoConnection(libusb0_methods * um)
    : ShupitoConnection(CONNECTION_USB_SHUPITO), m_um(um), m_dev(0), m_handle(0)
{
    this->Close();
}

void UsbShupitoConnection::OpenConcurrent()
{
    if (!m_dev)
        return;

    m_handle = m_um->usb_open(m_dev);
    if (m_handle)
        this->SetState(st_connected);
}

void UsbShupitoConnection::Close()
{
    if (!m_handle)
        return;
    m_um->usb_close(m_handle);
    m_handle = 0;
    this->SetState(st_disconnected);
}

bool UsbShupitoConnection::setUsbDevice(struct usb_device * dev)
{
    if (this->state() == st_connected)
        this->Close();

    Q_ASSERT(this->state() == st_disconnected);
    m_dev = dev;
    return this->updateStrings();
}

void UsbShupitoConnection::sendPacket(ShupitoPacket const & packet)
{
    // XXX
}

QString UsbShupitoConnection::details() const
{
    QStringList list;

    QString res = ShupitoConnection::details();
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

bool UsbShupitoConnection::updateStrings()
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

bool UsbShupitoConnection::isDeviceSupported(struct usb_device * dev)
{
    Q_ASSERT(dev);
    return dev->descriptor.idVendor == 0x4a61 && dev->descriptor.idProduct == 0x679a;
}
