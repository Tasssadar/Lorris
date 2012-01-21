#ifndef SHUPITOMODE_H
#define SHUPITOMODE_H

#include <QTypeInfo>
#include <QByteArray>
#include <QObject>

#include "shupitodesc.h"

class Shupito;
class chip_definition;

class ShupitoMode : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void updateProgressDialog(int val);

public:
    ShupitoMode(Shupito *shupito);

    static ShupitoMode *getMode(quint8 mode, Shupito *shupito);

    bool isInFlashMode() { return m_flash_mode; }

    virtual void switchToFlashMode(quint32 speed_hz);
    virtual void switchToRunMode();
    virtual QString readDeviceId();

    virtual QByteArray readMemory(const QString& mem, chip_definition &chip);
    virtual void readMemRange(quint8 memid, QByteArray& memory, quint32 address, quint32 size);

public slots:
    void cancelRequested();

protected:
    virtual void prepare();
    virtual ShupitoDesc::config *getModeCfg();
    virtual void editIdArgs(QString& id, quint8& id_lenght);

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
