#ifndef LORRIS_CONNECTION_USBSHUPITO23CONN_H
#define LORRIS_CONNECTION_USBSHUPITO23CONN_H

#include "connection.h"
#include "shupitoconn.h"
#include "../misc/threadchannel.h"
#include <libyb/usb/usb_device.hpp>
#include <libyb/usb/interface_guard.hpp>
#include <libyb/async/async_channel.hpp>
#include <libyb/async/async_runner.hpp>

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
    size_t maxPacketSize() const { return 255; }

    bool getFirmwareDetails(ShupitoFirmwareDetails & details) const;
    int vid() const;
    int pid() const;
    QString serialNumber() const;
    QString intfName() const;

    QHash<QString, QVariant> config() const;
    bool canSaveToSession() const { return true; }

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
    size_t m_out_ep_size;
    std::vector<uint8_t> m_in_eps;
    QString m_details;
    ShupitoDesc m_desc;

    ThreadChannel<ShupitoPacket> m_incomingPackets;
    ThreadChannel<void> m_sendCompleted;

    yb::async_channel<std::vector<uint8_t> > m_write_channel;

    struct write_loop_ctx
    {
        std::vector<std::vector<uint8_t> > packets;
        size_t packet_index;
    };

    write_loop_ctx m_write_loop_ctx;
    yb::async_future<void> m_write_loop;

    yb::task<void> write_loop();
    yb::task<void> write_packets();

    struct read_loop_ctx
    {
        uint8_t read_buffer[256];
        yb::async_future<void> read_loop;
    };

    std::unique_ptr<read_loop_ctx[]> m_read_loops;
    yb::task<void> read_loop(uint8_t i);
};

#endif // LORRIS_CONNECTION_USBSHUPITO23CONN_H
