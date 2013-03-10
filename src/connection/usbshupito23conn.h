#ifndef LORRIS_CONNECTION_USBSHUPITO23CONN_H
#define LORRIS_CONNECTION_USBSHUPITO23CONN_H

#include "connection.h"
#include "shupitoconn.h"
#include "../misc/threadchannel.h"
#include <libyb/usb/usb_device.hpp>
#include <libyb/usb/interface_guard.hpp>
#include <libyb/async/async_channel.hpp>
#include <libyb/async/async_runner.hpp>
#include <QDateTime>

struct ShupitoFirmwareDetails
{
    uint8_t hw_major;
    uint8_t hw_minor;
    QDateTime fw_timestamp;
    int16_t fw_zone_offset;
    QString fw_revision;

    QString firmwareFilename() const;
};

class UsbShupito23Connection
    : public ShupitoConnection
{
    Q_OBJECT

public:
    explicit UsbShupito23Connection(yb::async_runner & runner);

    QString details() const;

    void setup(yb::usb_device_interface const & intf);
    void clear();

    void requestDesc();

    bool getFirmwareDetails(ShupitoFirmwareDetails & details) const;

public slots:
    void sendPacket(ShupitoPacket const & packet);

protected:
    void doOpen();
    void doClose();

private slots:
    void incomingPacketsReceived();
    void sendCompleted();

private:
    void closeImpl();

    yb::async_runner & m_runner;
    yb::usb_device_interface m_intf;
    yb::usb_interface_guard m_intf_guard;
    uint8_t m_out_ep;
    std::vector<uint8_t> m_in_eps;
    QString m_details;
    ShupitoDesc m_desc;

    yb::async_channel<std::vector<uint8_t> > m_write_channel;
    yb::async_future<void> m_write_loop;

    struct write_loop_ctx
    {
        std::vector<std::vector<uint8_t> > packets;
        size_t packet_index;
    };

    write_loop_ctx m_write_loop_ctx;

    yb::task<void> write_loop();
    yb::task<void> write_packets();

    struct read_loop_ctx
    {
        uint8_t read_buffer[256];
        yb::async_future<void> read_loop;
    };

    std::unique_ptr<read_loop_ctx[]> m_read_loops;
    yb::task<void> read_loop(uint8_t i);

    ThreadChannel<ShupitoPacket> m_incomingPackets;
    ThreadChannel<void> m_sendCompleted;
};

#endif // LORRIS_CONNECTION_USBSHUPITO23CONN_H
