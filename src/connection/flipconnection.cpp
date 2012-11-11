#include "flipconnection.h"

FlipConnection::FlipConnection()
    : Connection(CONNECTION_FLIP)
{
    this->setName("DFU: atxmega32a4u");
}

void FlipConnection::OpenConcurrent()
{
    this->SetState(st_connected);
}

void FlipConnection::Close()
{
    this->SetState(st_disconnected);
}

bool FlipConnection::present() const
{
    return !m_dev.empty();
}

void FlipConnection::setDevice(yb::usb_device const & dev)
{
    m_dev = dev;
    emit this->changed();
}

void FlipConnection::clearDevice()
{
    m_dev.clear();
    emit this->changed();
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
