#include "genericusbconn.h"
#include "usbacmconn.h"
#include "connectionmgr2.h"
#include <libyb/async/sync_runner.hpp>

static struct
{
    uint16_t vid;
    uint16_t pid;
    char const * name;
} const g_deviceNames[] = {
    { 0x03EB, 0x2FD8, "ATxmega16C4 DFU" },
    { 0x03EB, 0x2FD9, "ATxmega32C4 DFU" },
    { 0x03EB, 0x2FD6, "ATxmega64C3 DFU" },
    { 0x03EB, 0x2FD7, "ATxmega128C3 DFU" },
    { 0x03EB, 0x2FDA, "ATxmega256C3 DFU" },
    { 0x03EB, 0x2FDB, "ATxmega384C3 DFU" },
    { 0x03EB, 0x2FE8, "ATxmega64A1U DFU" },
    { 0x03EB, 0x2FED, "ATxmega128A1U DFU" },
    { 0x03EB, 0x2FDD, "ATxmega64A4U DFU" },
    { 0x03EB, 0x2FDE, "ATxmega128A4U DFU" },
    { 0x03EB, 0x2FDF, "ATxmega64B3 DFU" },
    { 0x03EB, 0x2FE0, "ATxmega128B3 DFU" },
    { 0x03EB, 0x2FE1, "ATxmega64B1 DFU" },
    { 0x03EB, 0x2FEA, "ATxmega128B1 DFU" },
    { 0x03EB, 0x2FE2, "ATxmega256A3BU DFU" },
    { 0x03EB, 0x2FE3, "ATxmega16A4U DFU" },
    { 0x03EB, 0x2FE4, "ATxmega32A4U DFU" },
    { 0x03EB, 0x2FE5, "ATxmega64A4U DFU" },
    { 0x03EB, 0x2FE6, "ATxmega128A3U DFU" },
    { 0x03EB, 0x2FE7, "ATxmega192A3U DFU" },
    { 0x03EB, 0x2FDC, "UC3 L3/L4 DFU" },
    { 0x03EB, 0x2FE9, "ATUC3 D DFU" },
    { 0x03EB, 0x2FEB, "AT32UC3C DFU" },
    { 0x03EB, 0x2FEC, "ATxmega256A3U DFU" },
    { 0x03EB, 0x2FEE, "ATmega8U2 DFU" },
    { 0x03EB, 0x2FEF, "ATmega16U2 DFU" },
    { 0x03EB, 0x2FF0, "ATmega32U2 DFU" },
    { 0x03EB, 0x2FF1, "AT32UC3A3 DFU" },
    { 0x03EB, 0x2FF2, "ATmega32U6 DFU" },
    { 0x03EB, 0x2FF3, "ATmega16U4 DFU" },
    { 0x03EB, 0x2FF4, "ATmega32U4 DFU" },
    { 0x03EB, 0x2FF6, "AT32UC3B DFU" },
    { 0x03EB, 0x2FF7, "AT90USB82 DFU" },
    { 0x03EB, 0x2FF8, "AT32UC3A DFU" },
    { 0x03EB, 0x2FF9, "AT90USB64 DFU" },
    { 0x03EB, 0x2FFA, "AT90USB162 DFU" },
    { 0x03EB, 0x2FFB, "AT90USB128 DFU" },
    { 0x03EB, 0x2FFD, "AT89C5130/AT89C5131 DFU" },
    { 0x03EB, 0x2FFE, "AT8XC5122 DFU" },
    { 0x03EB, 0x2FFF, "AT89C5132/AT89C51SND1/AT89C51SND2 DFU" },
};

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

static QString formatDeviceName(yb::usb_device const & dev, bool & hasRealName)
{
    hasRealName = true;

    QString productName = fromUtf8(dev.product());
    if (!productName.isEmpty())
        return productName;

    yb::usb_device_descriptor const & desc = dev.descriptor();
    for (size_t i = 0; i < sizeof g_deviceNames / sizeof g_deviceNames[0]; ++i)
    {
        if (g_deviceNames[i].vid == desc.idVendor && g_deviceNames[i].pid == desc.idProduct)
            return QString::fromUtf8(g_deviceNames[i].name);
    }

    hasRealName = false;
    return QString("USB %1:%2").arg(desc.idVendor, 4, 16, QChar('0')).arg(desc.idProduct, 4, 16, QChar('0'));
}

QString GenericUsbConnection::formatDeviceName(yb::usb_device const & dev)
{
    bool hasRealName;
    return ::formatDeviceName(dev, hasRealName);
}

static QString formatDeviceDetails(yb::usb_device const & dev, bool hasRealName)
{
    yb::usb_device_descriptor const & desc = dev.descriptor();
    QString manufacturerName = fromUtf8(dev.manufacturer());
    QString serialNumber = fromUtf8(dev.serial_number());

    QStringList res;
    if (hasRealName)
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

            bool hasRealName;
            QString name = ::formatDeviceName(m_dev, hasRealName);

            if (updateName)
                this->setName(name);

            m_details = formatDeviceDetails(m_dev, hasRealName);
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
