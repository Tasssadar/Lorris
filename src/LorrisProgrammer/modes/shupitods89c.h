/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SHUPITODS89C_H
#define SHUPITODS89C_H

#include "shupitomode.h"
#include "../shupito.h"
#include <stdint.h>

class ShupitoDs89c : public ShupitoMode
{
    Q_OBJECT
public:
    ShupitoDs89c(Shupito *shupito);

    bool isInFlashMode() override { return m_flash_mode; }
    void switchToFlashMode(quint32 speed_hz) override;
    void switchToRunMode() override;

    chip_definition readDeviceId() override;
    void erase_device(chip_definition& chip) override;

    ProgrammerCapabilities capabilities() const override;

protected:
    virtual ShupitoDesc::config const *getModeCfg() override;
    void flashPage(chip_definition::memorydef *memdef, std::vector<quint8>& memory, quint32 address) override;
    void readMemRange(quint8 memid, QByteArray& memory, quint32 address, quint32 size) override;
    void readFuses(std::vector<quint8>& data, chip_definition &chip) override;

private:
    void write(QByteArray const & line);
    QString read();
    QString waitForPrompt();
    void waitForNewLine();

    struct StringCapture
        : ShupitoPacketCapture
    {
        void onPacket(ShupitoPacket const & packet) override
        {
            data.append(QString::fromLatin1((char const *)packet.data() + 1, packet.size() - 1));
        }

        QString data;
    };

    StringCapture m_capture;
    QString m_banner;
    bool m_flash_mode;
    uint8_t m_prog_cmd_base;
};

#endif // SHUPITODS89C_H
