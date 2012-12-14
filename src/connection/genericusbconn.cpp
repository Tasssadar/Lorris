#include "genericusbconn.h"
#include "usbacmconn.h"
#include "connectionmgr2.h"
#include <libyb/async/sync_runner.hpp>

GenericUsbConnection::GenericUsbConnection(yb::async_runner & runner, yb::usb_device const & dev)
    : Connection(CONNECTION_LIBYB_USB), m_runner(runner)
{
    this->setDevice(dev, /*updateName=*/true);
}

yb::async_runner & GenericUsbConnection::runner() const
{
    return m_runner;
}

void GenericUsbConnection::OpenConcurrent()
{
    if (!m_dev.empty())
        this->SetState(st_connected);
}

void GenericUsbConnection::Close()
{
    if (!m_dev.empty())
        this->SetState(st_disconnected);
}

static QString fromUtf8(std::string const & s)
{
    return QString::fromUtf8(s.data(), s.size());
}

QString GenericUsbConnection::formatDeviceName(yb::usb_device const & dev)
{
    QString productName = fromUtf8(dev.product());
    if (!productName.isEmpty())
        return productName;

    yb::usb_device_descriptor const & desc = dev.descriptor();
    return QString("USB %1:%2").arg(desc.idVendor, 4, 16, QChar('0')).arg(desc.idProduct, 4, 16, QChar('0'));
}

QString GenericUsbConnection::formatDeviceDetails(yb::usb_device const & dev)
{
    yb::usb_device_descriptor const & desc = dev.descriptor();
    QString productName = fromUtf8(dev.product());
    QString manufacturerName = fromUtf8(dev.manufacturer());
    QString serialNumber = fromUtf8(dev.serial_number());

    QStringList res;
    if (!productName.isEmpty())
        res.push_back(QString("USB %1:%2").arg(desc.idVendor, 4, 16, QChar('0')).arg(desc.idProduct, 4, 16, QChar('0')));
    if (!manufacturerName.isEmpty())
        res.push_back(manufacturerName);
    if (!serialNumber.isEmpty())
        res.push_back(serialNumber);
    return res.join(", ");
}

void GenericUsbConnection::setDevice(yb::usb_device const & dev, bool updateName)
{
    if (m_dev == dev)
        return;

    try
    {
        m_dev = dev;
        this->SetState(m_dev.empty()? st_removed: st_disconnected);

        if (!m_dev.empty())
        {
            m_selected_langid = m_dev.get_default_langid();

            QString productName = fromUtf8(m_dev.product());
            QString manufacturerName = fromUtf8(m_dev.manufacturer());
            m_serialNumber = fromUtf8(m_dev.serial_number());

            if (updateName)
                this->setName(formatDeviceName(m_dev));

            m_details = formatDeviceDetails(m_dev);
        }
    }
    catch (...)
    {
        m_dev.clear();
        this->SetState(st_removed);
    }
}

void GenericUsbConnection::clearDevice()
{
    this->setDevice(yb::usb_device());
}

yb::usb_device GenericUsbConnection::device() const
{
    return m_dev;
}

bool GenericUsbConnection::isShupito20Device(yb::usb_device const & dev)
{
    return dev.vidpid() == 0x4a61679a;
}

bool GenericUsbConnection::isFlipDevice(yb::usb_device const & dev)
{
    return !dev.empty() && dev.vidpid() == 0x03eb2fe4;
}

bool GenericUsbConnection::isFlipDevice() const
{
    return isFlipDevice(m_dev);
}
