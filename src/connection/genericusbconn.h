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
    bool isFlipDevice() const;

private:
    yb::async_runner & m_runner;
    yb::usb_device m_dev;
    uint16_t m_selected_langid;

    QString m_serialNumber;
    QString m_details;

    struct acm_id
    {
        struct standby_info_type
        {
            uint8_t intfno;
            std::string intfname;
        };

        standby_info_type standby_info(UsbAcmConnection2 *) const
        {
            standby_info_type info;
            info.intfno = intfno;
            info.intfname = intfname;
            return info;
        }

        bool compatible_with(standby_info_type const & si) const
        {
            return (!si.intfname.empty() && !intfname.empty() && si.intfname == intfname)
                || (si.intfname.empty() && intfname.empty() && si.intfno == intfno);
        }

        uint8_t intfno;
        std::string intfname;

        uint8_t outep;
        uint8_t inep;

        friend bool operator<(acm_id const & lhs, acm_id const & rhs)
        {
            return lhs.intfno < rhs.intfno
                || (lhs.intfno == rhs.intfno && lhs.intfname < rhs.intfname);
        }
    };

    DeviceEnumerator<UsbAcmConnection2, acm_id> m_acm_conns;
};

#endif // GENERICUSBCONN_H
