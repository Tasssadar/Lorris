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

void UsbShupitoConnection::setUsbDevice(struct usb_device * dev)
{
    if (this->state() == st_connected)
        this->Close();

    Q_ASSERT(this->state() == st_disconnected);
    m_dev = dev;
    this->updateStrings();
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

void UsbShupitoConnection::updateStrings()
{
    if (!m_dev)
        return;

    usb_dev_handle * h = m_um->usb_open(m_dev);
    if (!h)
        return;

    char buf[1024];
    int len = m_um->usb_get_descriptor(h, USB_DT_STRING, 0, buf, sizeof buf);
    if (len < 0 || len < 4)
    {
        m_um->usb_close(h);
        return;
    }

    int langid = (buf[3] << 8) | buf[2];
    if (m_dev->descriptor.iManufacturer)
    {
        len = m_um->usb_get_string(h, m_dev->descriptor.iManufacturer, langid, buf, sizeof buf);
        if (len > 2 && len % 2 == 0)
            m_manufacturer = QString::fromUtf16((ushort const *)buf + 1, len / 2 - 1);
    }

    if (m_dev->descriptor.iProduct)
    {
        len = m_um->usb_get_string(h, m_dev->descriptor.iProduct, langid, buf, sizeof buf);
        if (len > 0 && len % 2 == 0)
            m_product = QString::fromUtf16((ushort const *)buf + 1, len / 2 - 1);
    }

    if (m_dev->descriptor.iSerialNumber)
    {
        len = m_um->usb_get_string(h, m_dev->descriptor.iSerialNumber, langid, buf, sizeof buf);
        if (len > 0 && len % 2 == 0)
            m_serialNumber = QString::fromUtf16((ushort const *)buf + 1, len / 2 - 1);
    }

    m_um->usb_close(h);
}

bool UsbShupitoConnection::isDeviceSupported(struct usb_device * dev)
{
    Q_ASSERT(dev);
    return dev->descriptor.idVendor == 0x4a61 && dev->descriptor.idProduct == 0x679a;
}
