/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

/*
 * Some parts of this file are derived from project stlink:
 *
 *     https://github.com/texane/stlink
 *
 * See stlink_COPYING in root of Lorris' repository
 *
 */

#include "stm32connection.h"
#include "stm32defines.h"
#include "../misc/utils.h"

STM32Connection::stm32_cmd::stm32_cmd(uint8_t b1)
{
    memset(data, 0, sizeof(data));
    data[0] = b1;
}

STM32Connection::stm32_cmd::stm32_cmd(uint8_t b1, uint8_t b2)
{
    memset(data, 0, sizeof(data));
    data[0] = b1;
    data[1] = b2;
}

STM32Connection::stm32_cmd::stm32_cmd(uint8_t b1, uint8_t b2, uint8_t b3)
{
    memset(data, 0, sizeof(data));
    data[0] = b1;
    data[1] = b2;
    data[2] = b3;
}

STM32Connection::STM32Connection(yb::async_runner & runner) :
    GenericUsbConnection(runner, yb::usb_device(), CONNECTION_STM32)
{
    m_enumerated = false;
    m_pid = m_vid = 0;

    this->markMissing();
}

void STM32Connection::setEnumeratedIntf(yb::usb_device_interface const & intf)
{
    m_enumerated = true;

    yb::usb_device dev = intf.device();
    m_vid = dev.vidpid() >> 16;
    m_pid = dev.vidpid() & 0xFFFF;
    m_serialNumber = QString::fromUtf8(dev.serial_number().c_str());
    m_intfName = UsbAcmConnection2::formatIntfName(intf);

    this->setup(intf);
}

void STM32Connection::setup(const yb::usb_device_interface &intf)
{
    assert(!intf.empty());
    yb::usb_interface const & idesc = intf.descriptor();
    assert(idesc.altsettings.size() == 1);

    yb::usb_interface_descriptor const & desc = idesc.altsettings[0];

    m_intf = intf;

    // FIXME: some better endpoint recognition?
    assert(desc.endpoints.size() >= 2);
    m_in_ep = desc.endpoints[0].bEndpointAddress;
    m_out_ep = desc.endpoints[1].bEndpointAddress;

    this->setDevice(intf.device(), true);
}

void STM32Connection::clear()
{
    m_intf.clear();
    this->setDevice(yb::usb_device());
}

void STM32Connection::notifyIntfPlugin(yb::usb_device_interface const & intf)
{
    if (!m_enumerated && this->isMissing())
    {
        yb::usb_device const & dev = intf.device();
        yb::usb_device_descriptor const & desc = dev.descriptor();

        if (desc.idVendor == m_vid && desc.idProduct == m_pid
            && QString::fromUtf8(intf.device().serial_number().c_str()) == m_serialNumber
            && UsbAcmConnection2::formatIntfName(intf) == m_intfName)
        {
            this->setup(intf);
        }
    }
}

void STM32Connection::notifyIntfUnplug(yb::usb_device_interface const & intf)
{
    if (!m_enumerated && m_intf == intf)
        this->clear();
}

void STM32Connection::doOpen()
{
    yb::usb_device dev = m_intf.device();
    if (!m_intf_guard.claim(dev, m_intf.interface_index()))
        return Utils::showErrorBox("Cannot claim the interface");

    this->SetState(st_connected);
}

void STM32Connection::doClose()
{
    emit disconnecting();
    m_intf_guard.release();
    this->SetState(st_disconnected);
}

QHash<QString, QVariant> STM32Connection::config() const
{
    QHash<QString, QVariant> cfg = GenericUsbConnection::config();
    cfg["vid"] = this->vid();
    cfg["pid"] = this->pid();
    cfg["serial_number"] = this->serialNumber();
    cfg["intf_name"] = this->intfName();
    return cfg;
}

bool STM32Connection::applyConfig(const QHash<QString, QVariant> &config)
{
    if (!m_enumerated)
    {
        m_vid = config.value("vid", 0).toInt();
        m_pid = config.value("pid", 0).toInt();
        m_serialNumber = config.value("serial_number").toString();
        m_intfName = config.value("intf_name").toString();
    }
    return this->Connection::applyConfig(config);
}

void STM32Connection::send_cmd(const stm32_cmd &cmd)
{
    yb::task_result<size_t> res = m_runner.try_run(m_dev.bulk_write(m_out_ep, cmd.data, sizeof(cmd.data)));

    if(res.has_exception())
        res.rethrow();
}

int STM32Connection::send_recv_cmd(const stm32_cmd &cmd, uint8_t *reply, size_t reply_len)
{
    yb::task_result<size_t> res = m_runner.try_run(m_dev.bulk_write(m_out_ep, cmd.data, sizeof(cmd.data))
        .then([this, reply, reply_len](size_t r) -> yb::task<size_t>  {
            return m_dev.bulk_read(m_in_ep, reply, reply_len);
        }));

    if(res.has_exception())
        res.rethrow();
    return res.get();
}

void STM32Connection::send_data(const uint8_t *data, size_t len)
{
    yb::task_result<size_t> res = m_runner.try_run(m_dev.bulk_write(m_out_ep, data, len));

    if(res.has_exception())
        res.rethrow();
}

void STM32Connection::c_enter_swd_mode()
{
    send_cmd(stm32_cmd(STLINK_DEBUG_COMMAND, STLINK_DEBUG_ENTER, STLINK_DEBUG_ENTER_SWD));
}

void STM32Connection::c_exit_dfu_mode()
{
    send_cmd(stm32_cmd(STLINK_DFU_COMMAND, STLINK_DFU_EXIT));
}

void STM32Connection::c_enter_dfu_mode()
{
    send_cmd(stm32_cmd(STLINK_DFU_COMMAND, STLINK_DFU_ENTER));
}

void STM32Connection::c_exit_debug_mode()
{
    send_cmd(stm32_cmd(STLINK_DEBUG_COMMAND, STLINK_DEBUG_EXIT));
}

int STM32Connection::c_current_mode()
{
    uint8_t reply[2] = { 0 };
    send_recv_cmd(stm32_cmd(STLINK_GET_CURRENT_MODE), reply, sizeof(reply));
    return reply[0];
}

void STM32Connection::c_reset()
{
    // Reset the ARM core and leave it in frozen state
    uint8_t reply[2] = { 0 }; // ?
    send_recv_cmd(stm32_cmd(STLINK_DEBUG_COMMAND, STLINK_DEBUG_RESETSYS), reply, sizeof(reply));
}

void STM32Connection::c_run()
{
    uint8_t reply[2] = { 0 }; // ?
    send_recv_cmd(stm32_cmd(STLINK_DEBUG_COMMAND, STLINK_DEBUG_RUNCORE), reply, sizeof(reply));
}

void STM32Connection::c_version()
{
    uint8_t b[6] = { 0 };
    send_recv_cmd(stm32_cmd(STLINK_GET_VERSION), b, sizeof(b));

    // b0 b1                       || b2 b3  | b4 b5
    // 4b        | 6b     | 6b     || 2B     | 2B
    // stlink_v  | jtag_v | swim_v || st_vid | stlink_pid

    m_version.stlink_v = (b[0] & 0xf0) >> 4;
    m_version.jtag_v = ((b[0] & 0x0f) << 2) | ((b[1] & 0xc0) >> 6);
    m_version.swim_v = b[1] & 0x3f;
    m_version.st_vid = (b[3] << 8) | b[2];
    m_version.stlink_pid = (b[5] << 8) | b[4];
}

uint32_t STM32Connection::c_core_id()
{
    uint32_t id = 0;
    send_recv_cmd(stm32_cmd(STLINK_DEBUG_COMMAND, STLINK_DEBUG_READCOREID), (uint8_t*)&id, sizeof(id));
    return id;
}

uint32_t STM32Connection::c_read_debug32(uint32_t address)
{
    uint32_t reply[2] = { 0 };

    stm32_cmd cmd(STLINK_DEBUG_COMMAND, STLINK_JTAG_READDEBUG_32BIT);
    memcpy(&cmd.data[2], &address, sizeof(address));

    send_recv_cmd(cmd, (uint8_t*)reply, sizeof(reply));
    return reply[1];
}

QByteArray STM32Connection::c_read_mem32(uint32_t address, uint16_t len)
{
    if (len % 4 != 0)
        throw tr("STM32Connection::c_read_mem32: read len is not 32bit aligned!");

    QByteArray res(len, 0);

    stm32_cmd cmd(STLINK_DEBUG_COMMAND, STLINK_DEBUG_READMEM_32BIT);
    memcpy(&cmd.data[2], &address, sizeof(address));
    memcpy(&cmd.data[6], &len, sizeof(len));

    send_recv_cmd(cmd, (uint8_t*)res.data(), len);
    return res;
}

void STM32Connection::c_write_debug32(uint32_t address, uint32_t val)
{
    uint8_t reply[2] = { 0 };

    stm32_cmd cmd(STLINK_DEBUG_COMMAND, STLINK_JTAG_WRITEDEBUG_32BIT);
    memcpy(&cmd.data[2], &address, sizeof(address));
    memcpy(&cmd.data[6], &val, sizeof(val));

    send_recv_cmd(cmd, (uint8_t*)reply, sizeof(reply));
}

void STM32Connection::c_write_mem32(uint32_t address, const uint8_t *data, uint16_t size)
{
    if(size % 4 != 0)
        throw tr("STM32Connection::c_write_mem32: write len is not 32bit aligned!");

    stm32_cmd cmd(STLINK_DEBUG_COMMAND, STLINK_DEBUG_WRITEMEM_32BIT);
    memcpy(&cmd.data[2], &address, sizeof(address));
    memcpy(&cmd.data[6], &size, sizeof(size));

    send_cmd(cmd);
    send_data(data, size);
}

void STM32Connection::c_write_mem8(uint32_t address, const uint8_t *data, uint16_t size)
{
    if(size > 64)
        throw tr("STM32Connection::c_write_mem8: can't write more than 64 bytes at once!");

    stm32_cmd cmd(STLINK_DEBUG_COMMAND, STLINK_DEBUG_WRITEMEM_8BIT);
    memcpy(&cmd.data[2], &address, sizeof(address));
    memcpy(&cmd.data[6], &size, sizeof(size));

    send_cmd(cmd);
    send_data(data, size);
}

void STM32Connection::c_write_reg(uint32_t val, uint8_t idx)
{
    if(idx > 20)
        throw tr("STM32Connection::c_write_reg: reg idx must be in range 0..20");

    uint8_t reply[2] = { 0 };

    stm32_cmd cmd(STLINK_DEBUG_COMMAND, STLINK_DEBUG_WRITEREG);
    cmd.data[2] = idx;
    memcpy(&cmd.data[3], &val, sizeof(val));

    send_recv_cmd(cmd, (uint8_t*)reply, sizeof(reply));
}

uint32_t STM32Connection::c_read_reg(uint8_t idx)
{
    if(idx > 20)
        throw tr("STM32Connection::c_write_reg: reg idx must be in range 0..20");

    uint32_t reg_val = 0;

    stm32_cmd cmd(STLINK_DEBUG_COMMAND, STLINK_DEBUG_READREG);
    cmd.data[2] = idx;

    send_recv_cmd(cmd, (uint8_t*)&reg_val, sizeof(reg_val));
    return reg_val;
}

int STM32Connection::c_status()
{
    uint8_t reply[2] = { 0 };

    stm32_cmd cmd(STLINK_DEBUG_COMMAND, STLINK_DEBUG_GETSTATUS);
    send_recv_cmd(cmd, (uint8_t*)reply, sizeof(reply));

    return reply[0];
}

bool STM32Connection::is_core_halted()
{
    return this->c_status() == STLINK_CORE_HALTED;
}

/*
 * This method was constructed by reverse-engineering the usb communication
 * from official STM32 ST-Link Utility - these commands (plus some reads)
 * are transfered when "System reset" button is clicked in
 * "Target -> MCU Core.." window.
 *
 * FIXME: It was only tested with STM32F3 Discovery board!
 *
 */
void STM32Connection::c_force_reset()
{
    // clear "Debug Exception and Monitor Control Register"
    c_write_debug32(DEMCR, 0);

    // FIXME: this may be device-specific!
    c_write_debug32(AIRCR, AIRCR_VECTKEY | AIRCR_SYSRESETREQ);

    // Not needed, no idea what it is. It's probably some write...
    // maybe to register 0x10?
    // Response is classic 0x80 0x00 (STLINK_OKAY)
    /*uint8_t reply[2] = { 0 };
    stm32_cmd c(STLINK_DEBUG_COMMAND, 0x34);
    c.data[2] = 0x10; // uint32?
    c.data[6] = 1;    // uint32, 16 or 8?
    send_recv_cmd(c, reply, 2);*/

    // clear "Debug Exception and Monitor Control Register"
    c_write_debug32(DEMCR, 0);

    // Debug Halting Control and Status Register operations
    c_write_debug32(DHCSR, DBGKEY | DHCSR_C_DEBUGEN);
    c_write_debug32(DHCSR, DBGKEY | DHCSR_C_DEBUGEN | DHCSR_C_HALT);

    // Not needed, unknown command - returns 12 bytes as answer,
    // I've only seen either all zeros or first one 0x80 (STLINK_OKAY)
    /*uint8_t reply2[12] = { 0 };
    send_recv_cmd(stm32_cmd(STLINK_DEBUG_COMMAND, 0x3e), reply2, sizeof(reply2));
    send_recv_cmd(stm32_cmd(STLINK_DEBUG_COMMAND, 0x3e), reply2, sizeof(reply2));*/

    c_write_debug32(DHCSR, DBGKEY | DHCSR_C_DEBUGEN);
}
