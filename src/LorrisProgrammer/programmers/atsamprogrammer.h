#ifndef ATSAM_PROGRAMMER_H
#define ATSAM_PROGRAMMER_H

#include "../../shared/programmer.h"
#include "../../connection/genericusbconn.h"
#include <QEventLoop>

class AtsamProgrammer
    : public Programmer
{
    Q_OBJECT

public:
    AtsamProgrammer(ConnectionPointer<PortConnection> const & conn, ProgrammerLogSink * logsink);
    ~AtsamProgrammer();

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

    virtual ProgrammerCapabilities capabilities() const override;

public slots:
    virtual void cancelRequested() { m_cancelled = true; }
    virtual void sendTunnelData(const QString & data) override;

private slots:
    void dataRead(QByteArray const & data);

private:

    void wait_eefc_ready();

    quint16 crc16(const QByteArray & data, quint32 crc);
    void write_file(const uint32_t& address, const QByteArray & data);

    uint32_t read_word(uint32_t address);
    void write_word(uint32_t address, uint32_t data);
    QString transact(const QString & data, const QString & delimiter = ">");
    QByteArray transact(const QByteArray& data, const QString & delimiter, const bool& hex_debug);

    void debug_output(const QString & dir, QString data);

    QByteArray m_recvBuffer;
    QByteArray m_recvBuffer1;
    QByteArray m_recvDelimiter;
    QEventLoop m_waitLoop;
    bool m_cancelled;
    bool m_flash_mode;
    bool m_tunnel_enabled;
    uint32_t m_applet_address;
    chip_definition* m_chipdef;

    ConnectionPointer<PortConnection> m_conn;
};

#endif
