/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SHUPITOMODE_H
#define SHUPITOMODE_H

#include <QTypeInfo>
#include <QByteArray>
#include <QObject>

#include "../shupitodesc.h"
#include "../../shared/chipdefs.h"

class Shupito;
class HexFile;

class ShupitoMode : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void updateProgressDialog(int val);
    void updateProgressLabel(const QString& text);

public:
    ShupitoMode(Shupito *shupito);

    static ShupitoMode *getMode(quint8 mode, Shupito *shupito);

    bool isInFlashMode() { return m_flash_mode; }

    virtual void switchToFlashMode(quint32 speed_hz);
    virtual void switchToRunMode();
    virtual chip_definition readDeviceId();

    virtual QByteArray readMemory(const QString& mem, chip_definition &chip);
    virtual void readMemRange(quint8 memid, QByteArray& memory, quint32 address, quint32 size);
    virtual void readFuses(std::vector<quint8>& data, chip_definition &chip);
    virtual void writeFuses(std::vector<quint8>& data, chip_definition &chip, quint8 verifyMode);
    virtual void flashRaw(HexFile& file, quint8 memId, chip_definition& chip, quint8 verifyMode);

    virtual void erase_device(chip_definition& chip);

public slots:
    void cancelRequested();

protected:
    virtual void prepare();
    virtual ShupitoDesc::config *getModeCfg();
    virtual void editIdArgs(QString& id, quint8& id_lenght);
    virtual bool is_read_memory_supported(chip_definition::memorydef */*memdef*/)
    {
        return true;
    }
    virtual void prepareMemForWriting(chip_definition::memorydef *memdef, chip_definition& chip);
    virtual void flashPage(chip_definition::memorydef *memdef, std::vector<quint8>& memory, quint32 address);
    virtual bool canSkipPages(quint8 memId);


    bool m_prepared;
    bool m_flash_mode;

    quint32 m_bsel_base;
    quint16 m_bsel_min;
    quint16 m_bsel_max;

    quint8 m_prog_cmd_base;

    volatile bool m_cancel_requested;

    Shupito *m_shupito;
};

#endif // SHUPITOMODE_H
