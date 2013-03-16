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
