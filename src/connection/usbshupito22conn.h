#ifndef USBSHUPITOCONN_H
#define USBSHUPITOCONN_H

#include "connection.h"
#include "usbacmconn.h"
#include "shupitoconn.h"
#include <libyb/usb/usb_device.hpp>

class UsbShupito22Connection : public ShupitoConnection
{
    Q_OBJECT

public:
    explicit UsbShupito22Connection(yb::async_runner & runner);
    ~UsbShupito22Connection();

/*    QString manufacturer() const { return m_acm_conn->manufacturer(); }
    QString product() const { return m_acm_conn->product(); }
    QString serialNumber() const { return m_acm_conn->serialNumber(); }*/

    void setup(yb::usb_device_interface const & intf);
    void clear();

    QString details() const;

    void requestDesc();

public slots:
    void sendPacket(ShupitoPacket const & packet);

protected:
    void doOpen();
    void doClose();

private slots:
    void acmConnChanged();
    void shupitoConnStateChanged(ConnectionState state);

private:
    ConnectionPointer<UsbAcmConnection2> m_acm_conn;
    ConnectionPointer<PortShupitoConnection> m_shupito_conn;
};

#endif // USBSHUPITOCONN_H
