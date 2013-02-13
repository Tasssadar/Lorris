#ifndef ATSAM_PROGRAMMER_H
#define ATSAM_PROGRAMMER_H

#include "../../shared/programmer.h"
#include "../../connection/genericusbconn.h"
#include <libyb/libyb/shupito/flip2.hpp>
#include <QEventLoop>

class AtsamProgrammer
    : public Programmer
{
    Q_OBJECT

public:
    AtsamProgrammer(ConnectionPointer<PortConnection> const & conn, ProgrammerLogSink * logsink);

    virtual void stopAll(bool wait);

    virtual void switchToFlashMode(quint32 prog_speed_hz);
    virtual void switchToRunMode();
    virtual bool isInFlashMode();
    virtual chip_definition readDeviceId();

    virtual QByteArray readMemory(const QString& mem, chip_definition &chip);
    virtual void readFuses(std::vector<quint8>& data, chip_definition &chip);
    virtual void writeFuses(std::vector<quint8>& data, chip_definition &chip, quint8 verifyMode);
    virtual void flashRaw(HexFile& file, quint8 memId, chip_definition& chip, quint8 verifyMode);

    virtual void erase_device(chip_definition& chip);

    virtual int getType();

public slots:
    virtual void cancelRequested() { m_cancelled = true; }

private slots:
    void dataRead(QByteArray const & data);

private:
    uint32_t read_word(uint32_t address);
    void write_word(uint32_t address, uint32_t data);
    void wait_eefc_ready();

    QString transact(QString const & data);
    QByteArray m_recvBuffer;
    QEventLoop m_waitLoop;
    bool m_cancelled;

    ConnectionPointer<PortConnection> m_conn;
};

#endif
