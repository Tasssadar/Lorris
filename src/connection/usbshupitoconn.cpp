#include "usbshupitoconn.h"
#include <QStringBuilder>
#include <QStringList>
#include <libusb.h>

struct ScopedConfigDescriptor
{
    explicit ScopedConfigDescriptor(libusb_config_descriptor * desc)
        : m_desc(desc)
    {
    }

    ~ScopedConfigDescriptor()
    {
        this->clear();
    }

    void clear()
    {
        if (m_desc)
        {
            libusb_free_config_descriptor(m_desc);
            m_desc = 0;
        }
    }

    libusb_config_descriptor * m_desc;
};

UsbAcmConnection::UsbAcmConnection()
    : PortConnection(CONNECTION_USB_ACM), m_dev(0), m_handle(0), m_read_transfer(0), m_stop_read(false), m_read_stopped(false)
{
    m_read_transfer = libusb_alloc_transfer(0);
}

UsbAcmConnection::~UsbAcmConnection()
{
    if (m_read_transfer)
        libusb_free_transfer(m_read_transfer);
    this->Close();
    this->setUsbDevice(0);
}

void UsbAcmConnection::OpenConcurrent()
{
    if (this->state() != st_disconnected)
        return;

    this->SetState(st_connecting);
    bool success = this->openImpl();
    this->SetState(success? st_connected: st_disconnected);
}

void UsbAcmConnection::static_read_completed(libusb_transfer * t)
{
    ((UsbAcmConnection *)t->user_data)->read_completed(t);
}

void UsbAcmConnection::read_completed(libusb_transfer * t)
{
    if (t->status == LIBUSB_TRANSFER_COMPLETED && t->actual_length != 0)
        emit dataRead(QByteArray((char *)m_read_buffer, t->actual_length));

    QMutexLocker l(&m_read_stopped_mutex);
    if (m_stop_read)
    {
        m_read_stopped = true;
        m_read_stopped_cond.wakeAll();
    }
    else
    {
        this->start_read_transfer();
    }
}

void UsbAcmConnection::start_read_transfer()
{
    libusb_fill_bulk_transfer(m_read_transfer, m_handle, m_read_ep, m_read_buffer, sizeof m_read_buffer, &static_read_completed, this, 0);
    libusb_submit_transfer(m_read_transfer);
}

bool UsbAcmConnection::openImpl()
{
    if (!m_dev || !isDeviceSupported(m_dev))
        return false;

    {
        libusb_config_descriptor * cdesc = 0;
        if (libusb_get_config_descriptor(m_dev, 0, &cdesc) < 0)
            return false;
        ScopedConfigDescriptor cdesc_guard(cdesc);

        if (cdesc->bNumInterfaces != 2)
            return false;

        if (cdesc->interface[1].num_altsetting != 1)
            return false;

        if (cdesc->interface[1].altsetting[0].bNumEndpoints != 2)
            return false;

        m_write_ep = cdesc->interface[1].altsetting[0].endpoint[0].bEndpointAddress;
        m_read_ep = cdesc->interface[1].altsetting[0].endpoint[1].bEndpointAddress;

        if ((m_write_ep & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_IN)
            std::swap(m_write_ep, m_read_ep);

        if ((m_write_ep & LIBUSB_ENDPOINT_DIR_MASK) != LIBUSB_ENDPOINT_OUT
                || (m_read_ep & LIBUSB_ENDPOINT_DIR_MASK) != LIBUSB_ENDPOINT_IN)
            return false;
    }

    Q_ASSERT(m_handle == 0);
    if (libusb_open(m_dev, &m_handle) < 0)
        return false;

    int res = libusb_claim_interface(m_handle, 1);
    if (res < 0)
    {
        libusb_close(m_handle);
        m_handle = 0;
        return false;
    }

    m_stop_read = false;
    m_read_stopped = false;
    this->start_read_transfer();

    return true;
}

void UsbAcmConnection::Close()
{
    if (m_handle)
    {
        {
            QMutexLocker l(&m_read_stopped_mutex);
            m_stop_read = true;
            libusb_cancel_transfer(m_read_transfer);

            while (!m_read_stopped)
            {
                m_read_stopped_cond.wait(&m_read_stopped_mutex);
            }
        }

        libusb_release_interface(m_handle, 1);
        libusb_close(m_handle);
        m_handle = 0;
    }

    this->SetState(st_disconnected);
}

bool UsbAcmConnection::setUsbDevice(libusb_device * dev)
{
    if (this->state() == st_connected)
        this->Close();

    Q_ASSERT(this->state() == st_disconnected);
    if (m_dev)
        libusb_unref_device(m_dev);
    m_dev = dev;
    if (m_dev)
    {
        libusb_ref_device(m_dev);
        return this->updateStrings();
    }

    return true;
}

void UsbAcmConnection::SendData(const QByteArray & data)
{
    if (!m_handle)
        return;

    unsigned char * ptr = (unsigned char *)data.data();
    int len = data.size();

    while (len)
    {
        int transferred;
        int res = libusb_bulk_transfer(m_handle, m_write_ep, ptr, len, &transferred, 0);
        if (res < 0)
            return;

        ptr += transferred;
        len -= transferred;
    }
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

static QString getUsbString(libusb_device_handle * h, int index, int langid)
{
    unsigned char buf[1024];
    int len = libusb_get_string_descriptor(h, index, langid, buf, sizeof buf - 2);
    if (len < 2 || len % 2 != 0)
        return QString();
    buf[len] = 0;
    buf[len+1] = 0;
    return QString::fromUtf16((ushort const *)(buf + 2));
}

bool UsbAcmConnection::updateStrings()
{
    libusb_device_descriptor desc;
    if (libusb_get_device_descriptor(m_dev, &desc) < 0)
        return false;

    if (desc.iProduct == 0)
        return false;

    libusb_device_handle * h = 0;
    if (libusb_open(m_dev, &h) < 0)
        return false;

    unsigned char buf[64];
    int len = libusb_get_descriptor(h, 3, 0, buf, sizeof buf);
    if (len < 0)
    {
        libusb_close(h);
        return false;
    }

    quint16 langid = (buf[3] << 8) | buf[2];
    m_product = getUsbString(h, desc.iProduct, langid);
    if (m_product.isNull())
    {
        libusb_close(h);
        return false;
    }

    if (desc.iManufacturer)
        m_manufacturer = getUsbString(h, desc.iManufacturer, langid);

    if (desc.iSerialNumber)
        m_serialNumber = getUsbString(h, desc.iSerialNumber, langid);

    libusb_close(h);
    return true;
}

bool UsbAcmConnection::isDeviceSupported(libusb_device * dev)
{
    Q_ASSERT(dev);

    libusb_device_descriptor desc;
    if (libusb_get_device_descriptor(dev, &desc) < 0)
        return false;

    if (desc.idVendor == 0x4a61 && desc.idProduct == 0x679a)
    {
        // FIXME: check that the descriptor is consistent
        return true;
    }

    return false;
}
