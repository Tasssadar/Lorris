#ifndef FLIPCONNECTION_H
#define FLIPCONNECTION_H

#include "connection.h"
#include <libyb/usb/usb_device.hpp>
#include <libyb/shupito/flip2.hpp>

class FlipConnection : public Connection
{
    Q_OBJECT

public:
    FlipConnection();

    void OpenConcurrent();
    void Close();

    bool present() const;

    void setDevice(yb::usb_device const & dev);
    void clearDevice();
    yb::usb_device device() const;

    static bool isDeviceSupported(yb::usb_device const & dev);

private:
    yb::usb_device m_dev;
    yb::flip2 m_flip2;
};

#endif // FLIPCONNECTION_H
