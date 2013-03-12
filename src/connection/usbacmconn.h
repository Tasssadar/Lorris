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
    enum stop_bits_t
    {
        sb_one,
        sb_one_and_half,
        sb_two
    };

    enum parity_t
    {
        pp_none,
        pp_odd,
        pp_even
    };

    UsbAcmConnection2(yb::async_runner & runner);
    ~UsbAcmConnection2();

    QString details() const { return m_details; }

    int baudRate() const { return m_baudrate; }
    void setBaudRate(int value);

    stop_bits_t stopBits() const { return m_stop_bits; }
    void setStopBits(stop_bits_t value);

    parity_t parity() const { return m_parity; }
    void setParity(parity_t value);

    int dataBits() const { return m_data_bits; }
    void setDataBits(int value);

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
    void sendCompleted();

private:
    void setIntf(yb::usb_device_interface const & intf);
    void updateIntf();

    bool m_enumerated;

    int m_vid;
    int m_pid;
    QString m_serialNumber;
    QString m_intfName;

    int m_baudrate;
    stop_bits_t m_stop_bits;
    parity_t m_parity;
    int m_data_bits;
    void update_line_control(bool force = false);

    yb::usb_device_interface m_intf;

    QString m_details;

    bool m_configurable;

    yb::async_runner & m_runner;
    yb::async_future<void> m_receive_worker;
    yb::async_future<void> m_send_worker;

    static size_t const read_buffer_count = 2;
    uint8_t m_read_buffers[read_buffer_count][64];

    yb::async_channel<uint8_t> m_send_channel;
    std::vector<uint8_t> m_write_buffer;
    size_t m_sent;

    yb::task<void> write_loop(int outep);
    yb::task<void> send_loop(int outep);
    void cleanupWorkers();

    ThreadChannel<uint8_t> m_incomingDataChannel;
    ThreadChannel<void> m_sendCompleted;
};

#endif // USBACMCONN_H
