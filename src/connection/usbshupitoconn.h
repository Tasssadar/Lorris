#ifndef USBSHUPITOCONN_H
#define USBSHUPITOCONN_H

#include "connection.h"
#include "shupitoconn.h"
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <libusb.h>

class UsbAcmConnection : public PortConnection
{
public:
    UsbAcmConnection();
    ~UsbAcmConnection();

    QString details() const;

    QString manufacturer() const { return m_manufacturer; }
    QString product() const { return m_product; }
    QString serialNumber() const { return m_serialNumber; }

    libusb_device * usbDevice() const { return m_dev; }
    bool setUsbDevice(libusb_device * dev);

    void OpenConcurrent();
    void Close();

    static bool isDeviceSupported(libusb_device * dev);

public slots:
    virtual void SendData(const QByteArray & data);

private:
    bool openImpl();
    bool updateStrings();
    static void LIBUSB_CALL static_read_completed(libusb_transfer * t);
    void read_completed(libusb_transfer * t);
    void start_read_transfer();

    libusb_device * m_dev;
    libusb_device_handle * m_handle;
    libusb_transfer * m_read_transfer;

    int m_write_ep;
    int m_read_ep;

    unsigned char m_read_buffer[1024];
    bool m_stop_read;
    bool m_read_stopped;
    QMutex m_read_stopped_mutex;
    QWaitCondition m_read_stopped_cond;

    QString m_manufacturer;
    QString m_product;
    QString m_serialNumber;
};

#endif // USBSHUPITOCONN_H
