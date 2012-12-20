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
    QString serialNumber() const { return m_serialNumber; }

    int baudRate() const { return m_baudrate; }
    void setBaudRate(int value) { m_baudrate = value; emit changed(); }

    void setup(yb::usb_device_interface const & intf, uint8_t outep, uint8_t inep);
    void clear();

    void OpenConcurrent();
    void Close();

    bool event(QEvent * ev);

public slots:
    void SendData(const QByteArray & data);

private:
    QString m_serialNumber;
    QString m_details;

    yb::usb_device_interface m_intf;
    uint8_t m_outep;
    uint8_t m_inep;

    bool m_configurable;
    int m_baudrate;

    yb::async_runner & m_runner;
    yb::async_future<void> m_receive_worker;
    yb::async_future<void> m_send_worker;

    uint8_t m_read_buffer[128];

    yb::async_channel<uint8_t> m_send_channel;
    std::vector<uint8_t> m_write_buffer;
    size_t m_sent;

    yb::task<void> write_loop();
    yb::task<void> send_loop();
    void cleanupWorkers();
};

#endif // USBACMCONN_H
