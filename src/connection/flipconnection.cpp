#include "flipconnection.h"

FlipConnection::FlipConnection()
    : Connection(CONNECTION_FLIP)
{
    this->setName("DFU: atxmega32a4u");
    this->SetState(st_removed);
}

void FlipConnection::OpenConcurrent()
{
    if (!m_dev.empty())
        this->SetState(st_connected);
}

void FlipConnection::Close()
{
    if (!m_dev.empty())
        this->SetState(st_disconnected);
}

void FlipConnection::setDevice(yb::usb_device const & dev)
{
    if (m_dev != dev)
    {
        m_dev = dev;
        this->SetState(m_dev.empty()? st_removed: st_disconnected);
    }
}

void FlipConnection::clearDevice()
{
    m_dev.clear();
    this->SetState(st_removed);
}

yb::usb_device FlipConnection::device() const
{
    return m_dev;
}

bool FlipConnection::isDeviceSupported(yb::usb_device const & dev)
{
    yb::usb_device_descriptor desc = dev.descriptor();
    return desc.idVendor == 0x03eb && desc.idProduct == 0x2fe4;
}
