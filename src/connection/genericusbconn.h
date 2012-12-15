#ifndef GENERICUSBCONN_H
#define GENERICUSBCONN_H

#include "connection.h"
#include "usbacmconn.h"
#include "deviceenumerator.h"
#include <libyb/async/async_runner.hpp>
#include <libyb/usb/usb_device.hpp>

class GenericUsbConnection : public Connection
{
    Q_OBJECT

public:
    explicit GenericUsbConnection(yb::async_runner & runner, yb::usb_device const & dev);

    yb::async_runner & runner() const;

    QString details() const { return m_details; }
    QString serialNumber() const { return m_serialNumber; }

    void OpenConcurrent();
    void Close();

    void setDevice(yb::usb_device const & dev, bool updateName = false);
    void clearDevice();
    yb::usb_device device() const;

    static bool isShupito20Device(yb::usb_device const & dev);
    static bool isFlipDevice(yb::usb_device const & dev);
    bool isFlipDevice() const;

    static QString formatDeviceName(yb::usb_device const & dev);

private:
    yb::async_runner & m_runner;
    yb::usb_device m_dev;
    uint16_t m_selected_langid;

    QString m_serialNumber;
    QString m_details;
};

#endif // GENERICUSBCONN_H
