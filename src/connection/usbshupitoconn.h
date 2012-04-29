#ifndef USBSHUPITOCONN_H
#define USBSHUPITOCONN_H

#include "connection.h"
#include "shupitoconn.h"

struct usb_device;
struct libusb0_methods;
struct usb_dev_handle;

class UsbShupitoConnection : public ShupitoConnection
{
public:
    explicit UsbShupitoConnection(libusb0_methods * um);

    QString details() const;

    QString manufacturer() const { return m_manufacturer; }
    QString product() const { return m_product; }
    QString serialNumber() const { return m_serialNumber; }

    usb_device * usbDevice() const { return m_dev; }
    bool setUsbDevice(usb_device * dev);

    void OpenConcurrent();
    void Close();

    static bool isDeviceSupported(usb_device * dev);

public slots:
    virtual void sendPacket(ShupitoPacket const & packet);

private:
    bool updateStrings();

    libusb0_methods * m_um;
    usb_device * m_dev;
    usb_dev_handle * m_handle;

    QString m_manufacturer;
    QString m_product;
    QString m_serialNumber;
};

#endif // USBSHUPITOCONN_H
