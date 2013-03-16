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

    virtual void stopAll(bool wait) override;

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

private:
    ConnectionPointer<GenericUsbConnection> m_conn;
    yb::async_runner & m_runner;
    yb::flip2 m_flip;
};

#endif
