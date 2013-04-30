/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SHUPITOJTAG_H
#define SHUPITOJTAG_H

#include "shupitomode.h"
#include <libyb/utils/svf_file.hpp>
#include <stdint.h>

class ShupitoJtag : public ShupitoMode
{
    Q_OBJECT

public:
    ShupitoJtag(Shupito *shupito);
    ProgrammerCapabilities capabilities() const override;

    bool isInFlashMode() override { return m_flash_mode; }
    void switchToFlashMode(quint32 speed_hz) override;
    void switchToRunMode() override;

    chip_definition readDeviceId() override;
    void erase_device(chip_definition& chip) override;
    void flashPage(chip_definition::memorydef *memdef, std::vector<quint8>& memory, quint32 address) override;
    void readMemRange(quint8 memid, QByteArray& memory, quint32 address, quint32 size) override;

    void executeText(QByteArray const & data, quint8 memId, chip_definition & chip) override;

protected:
    ShupitoDesc::config const * getModeCfg() override;

private:
    struct cost_visitor;
    struct play_visitor;

    void cmd_frequency(uint32_t speed_hz);

    uint32_t m_freq_base;
    uint32_t m_max_freq_hz;
};

#endif // SHUPITOJTAG_H
