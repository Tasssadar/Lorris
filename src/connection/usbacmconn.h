#ifndef USBACMCONN_H
#define USBACMCONN_H

#include "connection.h"
#include "../misc/threadchannel.h"
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

    int baudRate() const { return m_baudrate; }
    void setBaudRate(int value) { m_baudrate = value; emit changed(); }

    int vid() const { return m_vid; }
    int pid() const { return m_pid; }
    QString serialNumber() const { return m_serialNumber; }
    QString intfName() const { return m_intfName; }

    void setVid(int value) { m_vid = value; this->updateIntf(); }
    void setPid(int value) { m_pid = value; this->updateIntf(); }
    void setSerialNumber(QString const & value) { m_serialNumber = value; this->updateIntf(); }
    void setIntfName(QString const & value) { m_intfName = value; this->updateIntf(); }

    yb::usb_device_interface intf() const { return m_intf; }
    bool enumerated() const { return m_enumerated; }
    void setEnumeratedIntf(yb::usb_device_interface const & intf);
    void clearEnumeratedIntf();

    void notifyIntfPlugin(yb::usb_device_interface const & intf);
    void notifyIntfUnplug(yb::usb_device_interface const & intf);

    void clear();

    bool clonable() const { return true; }
    ConnectionPointer<Connection> clone();

    QHash<QString, QVariant> config() const;
    bool applyConfig(QHash<QString, QVariant> const & config);

    static QString formatIntfName(yb::usb_device_interface const & intf);

public slots:
    void SendData(const QByteArray & data);

protected:
    void doOpen();
    void doClose();

private slots:
    void incomingDataReady();

private:
    void setIntf(yb::usb_device_interface const & intf);
    void updateIntf();

    bool m_enumerated;

    int m_vid;
    int m_pid;
    QString m_serialNumber;
    QString m_intfName;

    int m_baudrate;

    yb::usb_device_interface m_intf;

    QString m_details;

    bool m_configurable;

    yb::async_runner & m_runner;
    yb::async_future<void> m_receive_worker;
    yb::async_future<void> m_send_worker;

    uint8_t m_read_buffer[64];

    yb::async_channel<uint8_t> m_send_channel;
    std::vector<uint8_t> m_write_buffer;
    size_t m_sent;

    yb::task<void> write_loop(int outep);
    yb::task<void> send_loop(int outep);
    void cleanupWorkers();

    ThreadChannel<uint8_t> m_incomingDataChannel;
};

#endif // USBACMCONN_H
