/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef STM32PROGRAMMER_H
#define STM32PROGRAMMER_H


#include "../../shared/programmer.h"
#include "../../connection/stm32connection.h"
#include <libyb/libyb/usb/usb_device.hpp>
#include <libyb/libyb/async/task.hpp>

typedef struct _stlink stlink_t;

class STM32FlashController;

class STM32Programmer : public Programmer
{
    Q_OBJECT

public:
    STM32Programmer(ConnectionPointer<STM32Connection> const & conn, ProgrammerLogSink * logsink);
    virtual ~STM32Programmer();

    virtual void stopAll(bool wait) override;
    virtual void cancelRequested();

    virtual void switchToFlashMode(quint32 prog_speed_hz) override;
    virtual void switchToRunMode() override;
    virtual bool isInFlashMode() override;
    virtual chip_definition readDeviceId() override;

    virtual QByteArray readMemory(const QString& mem, chip_definition &chip) override;
    virtual void readFuses(std::vector<quint8>& data, chip_definition &chip) override;
    virtual void writeFuses(std::vector<quint8>& data, chip_definition &chip, VerifyMode verifyMode) override;
    virtual void flashRaw(HexFile& file, quint8 memId, chip_definition& chip, VerifyMode verifyMode) override;

    virtual void erase_device(chip_definition& chip) override;

    virtual int getType() override;

    virtual ProgrammerCapabilities capabilities() const override;

private:
    typedef QScopedPointer<STM32FlashController> flash_ptr;
    uint32_t readChipId();

    ConnectionPointer<STM32Connection> m_conn;
    bool m_cancel_req;
};

class STM32FlashController : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void updateProgressDialog(int pct);

public:
    explicit STM32FlashController(ConnectionPointer<STM32Connection> const & conn, QObject *parent = NULL);
    virtual ~STM32FlashController();

    static STM32FlashController *getController(const QString& name, ConnectionPointer<STM32Connection> const & conn);

    virtual bool supports_mass_erase() = 0;

    virtual void erase_mass() = 0;
    virtual bool is_busy() = 0;
    virtual void unlock(bool guard = true) = 0;
    virtual void lock() = 0;
    virtual bool is_locked() = 0;
    virtual void erase_page(uint32_t addr) = 0;
    virtual void write(chip_definition& chip, uint32_t addr, const char* data, int size) = 0;

protected:
    struct flash_loader
    {
        uint32_t addr;
        uint32_t buff_addr;
    };

    ConnectionPointer<STM32Connection> m_conn;
    QByteArray m_flash_loader;
};


class STM32VLFlash : public STM32FlashController
{
    Q_OBJECT
public:
    STM32VLFlash(ConnectionPointer<STM32Connection> const & conn);
    ~STM32VLFlash();

    virtual bool supports_mass_erase() { return true; }

    virtual void erase_mass();
    virtual bool is_busy();
    virtual void unlock(bool guard = true);
    virtual void lock();
    virtual bool is_locked();
    virtual void erase_page(uint32_t addr);
    virtual void write(chip_definition &chip, uint32_t addr, const char* data, int size);

private:
    uint32_t read_sr();
    uint32_t read_cr();
    void set_cr_bit(uint8_t bit);
    void add_cr_bit(uint8_t bit);
    void init_flash_loader(flash_loader& loader);
    void run_flash_loader(const flash_loader& loader, uint32_t target, int size);
    void write_data_for_loader(const flash_loader& loader, const uint8_t *data, int len);

    bool m_lock_on_destroy;
};


#endif // STM32PROGRAMMER_H
