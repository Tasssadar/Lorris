/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SHUPITOSPIFLASH_H
#define SHUPITOSPIFLASH_H

#include "shupitomode.h"
#include <stdint.h>

class ShupitoSpiFlash : public ShupitoMode
{
    Q_OBJECT
public:
    ShupitoSpiFlash(Shupito *shupito);

    virtual chip_definition readDeviceId() override;

    virtual QByteArray readMemory(const QString& mem, chip_definition &chip) override;
    virtual void flashRaw(HexFile& file, quint8 memId, chip_definition& chip, VerifyMode verifyMode) override;

    virtual void erase_device(chip_definition& chip) override;

protected:
    virtual ShupitoDesc::config const *getModeCfg() override;

private:
    void writeEnable();
    uint8_t readStatus();

    void transfer(uint8_t const * out_data, uint8_t * in_data, size_t size);
    void readSfdp(uint32_t addr, uint8_t * data, size_t size);
};

#endif // SHUPITOSPIFLASH_H
