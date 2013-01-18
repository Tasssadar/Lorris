#ifndef FLIP_PROGRAMMER_H
#define FLIP_PROGRAMMER_H

#include "../../shared/programmer.h"
#include "../../connection/genericusbconn.h"
#include <libyb/libyb/shupito/flip2.hpp>

class FlipProgrammer
    : public Programmer
{
    Q_OBJECT

public:
    FlipProgrammer(ConnectionPointer<GenericUsbConnection> const & conn, ProgrammerLogSink * logsink);

    virtual void stopAll(bool wait);

    virtual void switchToFlashMode(quint32 prog_speed_hz);
    virtual void switchToRunMode();
    virtual bool isInFlashMode();
    virtual chip_definition readDeviceId();

    virtual QByteArray readMemory(const QString& mem, chip_definition &chip);
    virtual void readMemRange(quint8 memid, QByteArray& memory, quint32 address, quint32 size);
    virtual void readFuses(std::vector<quint8>& data, chip_definition &chip);
    virtual void writeFuses(std::vector<quint8>& data, chip_definition &chip, quint8 verifyMode);
    virtual void flashRaw(HexFile& file, quint8 memId, chip_definition& chip, quint8 verifyMode);

    virtual void erase_device(chip_definition& chip);

private:
    ConnectionPointer<GenericUsbConnection> m_conn;
    yb::async_runner & m_runner;
    yb::flip2 m_flip;
};

#endif
