#ifndef USBSHUPITOCONN_H
#define USBSHUPITOCONN_H

#include "connection.h"
#include "shupitoconn.h"
#include <QThread>

struct usb_device;
struct libusb0_methods;
struct usb_dev_handle;

class UsbAcmConnection : public PortConnection
{
public:
    explicit UsbAcmConnection(libusb0_methods * um);

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
    virtual void SendData(const QByteArray & data);

private:
    bool openImpl();
    bool updateStrings();

    libusb0_methods * m_um;
    usb_device * m_dev;
    usb_dev_handle * m_handle;
    int m_write_ep;
    int m_read_ep;

    QString m_manufacturer;
    QString m_product;
    QString m_serialNumber;

    QThread * m_readThread;
};

#endif // USBSHUPITOCONN_H
