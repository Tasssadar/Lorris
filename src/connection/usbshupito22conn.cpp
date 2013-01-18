#include "usbshupito22conn.h"
#include <stdexcept>

UsbShupito22Connection::UsbShupito22Connection(yb::async_runner & runner)
    : ShupitoConnection(CONNECTION_USB_SHUPITO)
{
    m_acm_conn.reset(new UsbAcmConnection2(runner));
    m_shupito_conn.reset(new PortShupitoConnection());
    m_shupito_conn->setPort(m_acm_conn);
    connect(m_shupito_conn.data(), SIGNAL(packetRead(ShupitoPacket)), this, SIGNAL(packetRead(ShupitoPacket)));
    connect(m_shupito_conn.data(), SIGNAL(descRead(ShupitoDesc)), this, SIGNAL(descRead(ShupitoDesc)));
    connect(m_shupito_conn.data(), SIGNAL(stateChanged(ConnectionState)), this, SLOT(shupitoConnStateChanged(ConnectionState)));
    connect(m_acm_conn.data(), SIGNAL(changed()), this, SLOT(acmConnChanged()));
    this->shupitoConnStateChanged(m_shupito_conn->state());
}

UsbShupito22Connection::~UsbShupito22Connection()
{
}

QString UsbShupito22Connection::details() const
{
    return m_acm_conn->details();
}

void UsbShupito22Connection::doOpen()
{
    m_shupito_conn->OpenConcurrent();
}

void UsbShupito22Connection::doClose()
{
    emit disconnecting();
    m_shupito_conn->Close();
}

void UsbShupito22Connection::sendPacket(ShupitoPacket const & packet)
{
    m_shupito_conn->sendPacket(packet);
}

void UsbShupito22Connection::setup(yb::usb_device_interface const & intf)
{
    m_acm_conn->setEnumeratedIntf(intf);
}

void UsbShupito22Connection::clear()
{
    m_acm_conn->clearEnumeratedIntf();
}

void UsbShupito22Connection::shupitoConnStateChanged(ConnectionState state)
{
    this->SetState(state);
}

void UsbShupito22Connection::acmConnChanged()
{
    emit changed();
}

void UsbShupito22Connection::requestDesc()
{
    m_shupito_conn->requestDesc();
}
