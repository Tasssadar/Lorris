#ifndef USBACMCONN_H
#define USBACMCONN_H

#include "connection.h"
#include <libyb/async/async_runner.hpp>
#include <libyb/async/async_channel.hpp>
#include <libyb/usb/usb_device.hpp>

class UsbAcmConnection2
    : public PortConnection
{
    Q_OBJECT

public:
    UsbAcmConnection2(yb::async_runner & runner);
    ~UsbAcmConnection2();

    QString details() const { return m_details; }

    bool enumerated() const { return m_enumerated; }
    void setEnumerated(bool value) { m_enumerated = value; }

    int baudRate() const { return m_baudrate; }
    void setBaudRate(int value) { m_baudrate = value; emit changed(); }

    int vid() const { return m_vid; }
    int pid() const { return m_pid; }
    QString serialNumber() const { return m_serialNumber; }
    QString intfName() const { return m_intfName; }

    void setVid(int value) { m_vid = value; emit changed(); }
    void setPid(int value) { m_pid = value; emit changed(); }
    void setSerialNumber(QString const & value) { m_serialNumber = value; emit changed(); }
    void setIntfName(QString const & value) { m_intfName = value; emit changed(); }

    yb::usb_device_interface intf() const { return m_enumerated_intf; }
    void setIntf(yb::usb_device_interface const & intf);

    void clear();

    void OpenConcurrent();
    void Close();

    bool event(QEvent * ev);

    bool clonable() const { return true; }
    ConnectionPointer<Connection> clone();

    QHash<QString, QVariant> config() const;
    bool applyConfig(QHash<QString, QVariant> const & config);

public slots:
    void SendData(const QByteArray & data);

private:
    bool m_enumerated;

    int m_vid;
    int m_pid;
    QString m_serialNumber;
    QString m_intfName;

    int m_baudrate;

    yb::usb_device_interface m_enumerated_intf;
    yb::usb_device_interface m_intf;

    QString m_details;

    bool m_configurable;

    yb::async_runner & m_runner;
    yb::async_future<void> m_receive_worker;
    yb::async_future<void> m_send_worker;

    uint8_t m_read_buffer[128];

    yb::async_channel<uint8_t> m_send_channel;
    std::vector<uint8_t> m_write_buffer;
    size_t m_sent;

    yb::task<void> write_loop(int outep);
    yb::task<void> send_loop(int outep);
    void cleanupWorkers();
};

#endif // USBACMCONN_H
