/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef STM32CONNECTION_H
#define STM32CONNECTION_H

#include <libyb/usb/usb_device.hpp>
#include <libyb/usb/interface_guard.hpp>
#include <libyb/async/async_runner.hpp>

#include "genericusbconn.h"

class STM32Connection : public GenericUsbConnection
{
    Q_OBJECT
public:
    explicit STM32Connection(yb::async_runner & runner);
    
    void setup(yb::usb_device_interface const & intf);
    void clear();

    bool is_core_halted();

    void c_enter_swd_mode();
    void c_exit_dfu_mode();
    void c_enter_dfu_mode();
    void c_exit_debug_mode();
    int c_current_mode();
    void c_reset();
    void c_run();
    void c_version();
    uint32_t c_core_id();
    uint32_t c_read_debug32(uint32_t address);
    QByteArray c_read_mem32(uint32_t address, uint16_t len);
    void c_write_debug32(uint32_t address, uint32_t val);
    void c_write_mem32(uint32_t address, const uint8_t *data, uint16_t size);
    void c_write_mem8(uint32_t address, const uint8_t *data, uint16_t size);
    void c_write_reg(uint32_t val, uint8_t idx);
    uint32_t c_read_reg(uint8_t idx);
    void c_force_reset();

    int c_status();

protected:
    virtual void doOpen();
    virtual void doClose();
    
private:
    struct stm32_cmd
    {
        stm32_cmd(uint8_t b1);
        stm32_cmd(uint8_t b1, uint8_t b2);
        stm32_cmd(uint8_t b1, uint8_t b2, uint8_t b3);
        uint8_t data[16];
    };
    struct stlink_version
    {
        uint32_t stlink_v;
        uint32_t jtag_v;
        uint32_t swim_v;
        uint32_t st_vid;
        uint32_t stlink_pid;
    };

    void send_cmd(const stm32_cmd& cmd);
    void send_data(const uint8_t *data, size_t len);
    int send_recv_cmd(const stm32_cmd& cmd, uint8_t *reply, size_t reply_len);

    yb::usb_device_interface m_intf;
    yb::usb_interface_guard m_intf_guard;

    uint8_t m_out_ep;
    uint8_t m_in_ep;

    stlink_version m_version;
};

#endif // STM32CONNECTION_H
