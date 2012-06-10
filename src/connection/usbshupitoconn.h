#ifndef USBSHUPITOCONN_H
#define USBSHUPITOCONN_H

#include "connection.h"
#include "shupitoconn.h"
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <libusby.h>

class UsbAcmConnection : public PortConnection
{
public:
    explicit UsbAcmConnection(libusby_context * ctx);
    ~UsbAcmConnection();

    QString details() const;

    QString manufacturer() const { return m_manufacturer; }
    QString product() const { return m_product; }
    QString serialNumber() const { return m_serialNumber; }

    libusby_device * usbDevice() const { return m_dev; }
    bool setUsbDevice(libusby_device * dev);

    void OpenConcurrent();
    void Close();

    static bool isDeviceSupported(libusby_device * dev);

protected:
    bool event(QEvent * ev);

public slots:
    virtual void SendData(const QByteArray & data);

private:
    bool openImpl();
    bool readConfig(libusby_device_handle * handle);
    bool updateStrings();
    static void static_read_completed(libusby_transfer * t);
    static void static_write_completed(libusby_transfer * t);

    libusby_device * m_dev;
    libusby_device_handle * m_handle;
    libusby_transfer * m_read_transfer;
    libusby_transfer * m_write_transfer;

    QMutex m_write_mutex;
    QByteArray m_send_data;
    QByteArray m_write_buffer;
    int m_write_buffer_pos;

    int m_write_ep;
    int m_read_ep;

    unsigned char m_read_buffer[1024];

    QString m_manufacturer;
    QString m_product;
    QString m_serialNumber;
};

#endif // USBSHUPITOCONN_H
