#ifndef USBSHUPITOCONN_H
#define USBSHUPITOCONN_H

#include "connection.h"
#include "shupitoconn.h"
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <libusby.hpp>

class UsbAcmConnection : public PortConnection
{
    Q_OBJECT

public:
    explicit UsbAcmConnection(libusby::context & ctx);
    ~UsbAcmConnection();

    QString details() const;

    QString manufacturer() const { return m_manufacturer; }
    QString product() const { return m_product; }
    QString serialNumber() const { return m_serialNumber; }

    libusby::device const & usbDevice() const { return m_dev; }
    bool setUsbDevice(libusby::device const & dev);

    void OpenConcurrent();
    void Close();

    static bool isDeviceSupported(libusby::device & dev);

protected:
    bool event(QEvent * ev);

public slots:
    virtual void SendData(const QByteArray & data);

private:
    bool openImpl();
    void closeImpl();
    bool readConfig(libusby_device_handle * handle);
    bool updateStrings();
    static void static_read_completed(libusby_transfer * t);
    static void static_write_completed(libusby_transfer * t);

    libusby::device m_dev;
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

class UsbShupitoConnection : public ShupitoConnection
{
    Q_OBJECT

public:
    explicit UsbShupitoConnection(libusby::context & ctx);
    ~UsbShupitoConnection();

    QString manufacturer() const { return m_acm_conn->manufacturer(); }
    QString product() const { return m_acm_conn->product(); }
    QString serialNumber() const { return m_acm_conn->serialNumber(); }

    libusby::device const & usbDevice() const { return m_acm_conn->usbDevice(); }
    bool setUsbDevice(libusby::device const & dev) { return m_acm_conn->setUsbDevice(dev); }

    QString details() const;
    void OpenConcurrent();
    void Close();

    void requestDesc();

    static bool isDeviceSupported(libusby::device & dev);

public slots:
    void sendPacket(ShupitoPacket const & packet);

private slots:
    void acmConnChanged();
    void shupitoConnStateChanged(ConnectionState state);

private:
    ConnectionPointer<UsbAcmConnection> m_acm_conn;
    ConnectionPointer<PortShupitoConnection> m_shupito_conn;
};

#endif // USBSHUPITOCONN_H
