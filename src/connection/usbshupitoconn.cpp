#include "usbshupitoconn.h"
#include <QStringBuilder>
#include <QStringList>
#include <QEvent>
#include <QApplication>
#include <stdexcept>

struct ScopedConfigDescriptor
{
    explicit ScopedConfigDescriptor(libusby_config_descriptor * desc)
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
            libusby_free_config_descriptor(m_desc);
            m_desc = 0;
        }
    }

    libusby_config_descriptor * m_desc;
};

UsbAcmConnection::UsbAcmConnection(libusby_context * ctx)
    : PortConnection(CONNECTION_USB_ACM), m_dev(0), m_handle(0), m_read_transfer(0), m_write_buffer_pos(0)
{
    m_read_transfer = libusby_alloc_transfer(ctx, 0);
    if (!m_read_transfer)
        throw std::runtime_error("Failed to allocate a USB transfer.");

    m_write_transfer = libusby_alloc_transfer(ctx, 0);
    if (!m_write_transfer)
    {
        libusby_free_transfer(m_read_transfer);
        throw std::runtime_error("Failed to allocate a USB transfer.");
    }
}

UsbAcmConnection::~UsbAcmConnection()
{
    this->Close();
    this->setUsbDevice(0);

    libusby_free_transfer(m_read_transfer);
    libusby_free_transfer(m_write_transfer);
}

void UsbAcmConnection::OpenConcurrent()
{
    if (this->state() != st_disconnected)
        return;

    this->SetState(st_connecting);
    bool success = this->openImpl();
    this->SetState(success? st_connected: st_disconnected);
}

void UsbAcmConnection::static_read_completed(libusby_transfer * t)
{
    QEvent * ev = new QEvent(QEvent::User);
    QCoreApplication::instance()->postEvent((UsbAcmConnection *)t->user_data, ev);
}

bool UsbAcmConnection::event(QEvent * ev)
{
    if (ev->type() == QEvent::User)
    {
        if (m_read_transfer && m_read_transfer->status == LIBUSBY_TRANSFER_COMPLETED && m_read_transfer->actual_length != 0)
            emit dataRead(QByteArray((char *)m_read_buffer, m_read_transfer->actual_length));

        if (m_handle)
        {
            libusby_fill_bulk_transfer(m_read_transfer, m_handle, m_read_ep, m_read_buffer, sizeof m_read_buffer, &static_read_completed, this, 0);
            libusby_submit_transfer(m_read_transfer);
        }

        return true;
    }
    else
    {
        return PortConnection::event(ev);
    }
}

bool UsbAcmConnection::readConfig(libusby_device_handle * handle)
{
    libusby_config_descriptor * cdesc = 0;
    if (libusby_get_active_config_descriptor(handle, &cdesc) < 0)
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

    if ((m_write_ep & LIBUSBY_ENDPOINT_DIR_MASK) == LIBUSBY_ENDPOINT_IN)
        std::swap(m_write_ep, m_read_ep);

    if ((m_write_ep & LIBUSBY_ENDPOINT_DIR_MASK) != LIBUSBY_ENDPOINT_OUT
            || (m_read_ep & LIBUSBY_ENDPOINT_DIR_MASK) != LIBUSBY_ENDPOINT_IN)
        return false;

    return true;
}

bool UsbAcmConnection::openImpl()
{
    if (!m_dev || !isDeviceSupported(m_dev))
        return false;

    Q_ASSERT(m_handle == 0);
    if (libusby_open(m_dev, &m_handle) < 0)
        return false;

    if (!this->readConfig(m_handle))
    {
        libusby_close(m_handle);
        m_handle = 0;
        return false;
    }

    int res = libusby_claim_interface(m_handle, 1);
    if (res < 0)
    {
        libusby_close(m_handle);
        m_handle = 0;
        return false;
    }

    libusby_fill_bulk_transfer(m_read_transfer, m_handle, m_read_ep, m_read_buffer, sizeof m_read_buffer, &static_read_completed, this, 0);
    libusby_submit_transfer(m_read_transfer);
    return true;
}

void UsbAcmConnection::Close()
{
    if (m_handle)
    {
        {
            QMutexLocker write_lock(&m_write_mutex);
            m_send_data.clear();
        }

        libusby_cancel_transfer(m_read_transfer);
        libusby_cancel_transfer(m_write_transfer);

        libusby_wait_for_transfer(m_read_transfer);
        libusby_wait_for_transfer(m_write_transfer);
        libusby_release_interface(m_handle, 1);
        libusby_close(m_handle);
        m_handle = 0;
    }

    this->SetState(st_disconnected);
}

bool UsbAcmConnection::setUsbDevice(libusby_device * dev)
{
    if (this->state() == st_connected)
        this->Close();

    Q_ASSERT(this->state() == st_disconnected);
    if (m_dev)
        libusby_unref_device(m_dev);
    m_dev = dev;
    if (m_dev)
    {
        libusby_ref_device(m_dev);
        return this->updateStrings();
    }

    return true;
}

void UsbAcmConnection::static_write_completed(libusby_transfer * t)
{
    UsbAcmConnection * self = (UsbAcmConnection *)t->user_data;

    QMutexLocker write_lock(&self->m_write_mutex);
    if (t->status != LIBUSBY_TRANSFER_COMPLETED || self->m_write_buffer_pos + t->actual_length == self->m_write_buffer.size())
    {
        self->m_write_buffer_pos = 0;
        self->m_write_buffer = self->m_send_data;
        self->m_send_data.clear();
    }
    else
    {
        Q_ASSERT(self->m_write_buffer_pos + t->actual_length < self->m_write_buffer.size());
        self->m_write_buffer_pos += t->actual_length;
    }

    if (!self->m_write_buffer.isEmpty())
    {
        libusby_fill_bulk_transfer(self->m_write_transfer, self->m_handle, self->m_write_ep,
            (uint8_t *)self->m_write_buffer.data() + self->m_write_buffer_pos,
            self->m_write_buffer.size() + self->m_write_buffer_pos,
            &static_write_completed, self, 0);
        libusby_submit_transfer(self->m_write_transfer);
    }
}

void UsbAcmConnection::SendData(const QByteArray & data)
{
    if (!m_handle || data.isEmpty())
        return;

    QMutexLocker write_lock(&m_write_mutex);
    if (m_write_buffer.isEmpty())
    {
        m_write_buffer = data;
        m_write_buffer_pos = 0;
        libusby_fill_bulk_transfer(m_write_transfer, m_handle, m_write_ep, (uint8_t *)m_write_buffer.data(), m_write_buffer.size(), &static_write_completed, this, 0);
        libusby_submit_transfer(m_write_transfer);
    }
    else
    {
        m_send_data.append(data);
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

static QString getUsbString(libusby_device_handle * h, int index, int langid)
{
    unsigned char buf[1024];
    int len = libusby_get_string_descriptor(h, index, langid, buf, sizeof buf - 2);
    if (len < 2 || len % 2 != 0)
        return QString();
    buf[len] = 0;
    buf[len+1] = 0;
    return QString::fromUtf16((ushort const *)(buf + 2));
}

bool UsbAcmConnection::updateStrings()
{
    libusby_device_descriptor desc;
    if (libusby_get_device_descriptor_cached(m_dev, &desc) < 0)
        return false;

    if (desc.iProduct == 0)
        return false;

    libusby_device_handle * h = 0;
    if (libusby_open(m_dev, &h) < 0)
        return false;

    unsigned char buf[64];
    int len = libusby_get_descriptor(h, 3, 0, buf, sizeof buf);
    if (len < 0)
    {
        libusby_close(h);
        return false;
    }

    quint16 langid = (buf[3] << 8) | buf[2];
    m_product = getUsbString(h, desc.iProduct, langid);
    if (m_product.isNull())
    {
        libusby_close(h);
        return false;
    }

    if (desc.iManufacturer)
        m_manufacturer = getUsbString(h, desc.iManufacturer, langid);

    if (desc.iSerialNumber)
        m_serialNumber = getUsbString(h, desc.iSerialNumber, langid);

    libusby_close(h);
    return true;
}

bool UsbAcmConnection::isDeviceSupported(libusby_device * dev)
{
    Q_ASSERT(dev);

    libusby_device_descriptor desc;
    if (libusby_get_device_descriptor_cached(dev, &desc) < 0)
        return false;

    if (desc.idVendor == 0x4a61 && (desc.idProduct == 0x679a || desc.idProduct == 0x679b))
    {
        // FIXME: check that the descriptor is consistent
        return true;
    }

    return false;
}
