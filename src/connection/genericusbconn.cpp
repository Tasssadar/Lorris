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

void GenericUsbConnection::setDevice(yb::usb_device const & dev, bool updateName)
{
    if (m_dev == dev)
        return;

    try
    {
        m_dev = dev;
        this->SetState(m_dev.empty()? st_removed: st_disconnected);

        std::vector<acm_id> acm_ids;
        if (!m_dev.empty())
        {
            m_selected_langid = m_dev.get_default_langid();

            yb::usb_device_descriptor desc = m_dev.descriptor();

            QString productName = fromUtf8(m_dev.product());
            QString manufacturerName = fromUtf8(m_dev.manufacturer());
            m_serialNumber = fromUtf8(m_dev.serial_number());

            if (updateName)
            {
                if (!productName.isEmpty())
                    this->setName(productName);
                else
                    this->setName(QString("USB %1:%2").arg(desc.idVendor, 4, 16, QChar('0')).arg(desc.idProduct, 4, 16, QChar('0')));
            }

            {
                QStringList res;
                if (!productName.isEmpty())
                    res.push_back(QString("USB %1:%2").arg(desc.idVendor, 4, 16, QChar('0')).arg(desc.idProduct, 4, 16, QChar('0')));
                if (!manufacturerName.isEmpty())
                    res.push_back(manufacturerName);
                if (!m_serialNumber.isEmpty())
                    res.push_back(m_serialNumber);
                m_details = res.join(", ");
            }

            yb::usb_config_descriptor config = m_dev.get_config_descriptor();
            for (size_t i = 0; i < config.interfaces.size(); ++i)
            {
                if (config.interfaces[i].altsettings.size() != 1)
                    continue;

                yb::usb_interface_descriptor & intf = config.interfaces[i].altsettings[0];
                if (intf.bInterfaceClass == 0xa && intf.bInterfaceSubClass == 0 && intf.bInterfaceProtocol == 0)
                {
                    // Lookup endpoints
                    bool ok = true;
                    uint8_t inep = 0, outep = 0;
                    for (size_t j = 0; ok && j < intf.endpoints.size(); ++j)
                    {
                        if (intf.endpoints[j].bEndpointAddress & 0x80)
                        {
                            if (inep)
                                ok = false;
                            else
                                inep = intf.endpoints[j].bEndpointAddress;
                        }
                        else
                        {
                            if (outep)
                                ok = false;
                            else
                                outep = intf.endpoints[j].bEndpointAddress;
                        }
                    }

                    if (!ok || (!inep && !outep))
                        continue;

                    assert(i == intf.bInterfaceNumber);

                    acm_id id;
                    id.intfno = i;
                    if (intf.iInterface)
                    { // XXX: use interface enumerator
                        try
                        {
                            id.intfname = m_dev.get_string_descriptor(intf.iInterface, m_selected_langid);
                        }
                        catch (...)
                        {
                        }
                    }
                    id.outep = outep;
                    id.inep = inep;
                    acm_ids.push_back(std::move(id));
                }
            }
        }

        m_acm_conns.update(acm_ids.begin(), acm_ids.end(), [this](acm_id const &) -> UsbAcmConnection2 * {
            ConnectionPointer<UsbAcmConnection2> conn(new UsbAcmConnection2(m_runner));
            sConMgr2.addConnection(conn.data());
            return conn.take();
        }, [this](acm_id const & id, UsbAcmConnection2 * conn) {
            conn->setup(m_dev, id.intfno, id.outep, id.inep);
            QString name;
            if (id.intfname.empty())
                name = QString("ACM%1 @ %2").arg(QString::number(id.intfno), this->name()) ;
            else
                name = QString("%1 @ %2").arg(QString::fromUtf8(id.intfname.data(), id.intfname.size()), this->name());
            conn->setName(name);
            conn->setRemovable(false);
            conn->setPersistent(!id.intfname.empty());
        }, [](UsbAcmConnection2 * conn) {
            conn->clear();
            conn->setRemovable(true);
        });
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

bool GenericUsbConnection::isFlipDevice() const
{
    return !m_dev.empty() && m_dev.vidpid() == 0x03eb2fe4;
}
