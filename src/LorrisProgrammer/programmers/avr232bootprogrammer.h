/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef AVR232BOOTPROGRAMMER_H
#define AVR232BOOTPROGRAMMER_H

#include "../../shared/programmer.h"
#include "../../connection/serialport.h"

class avr232bootProgrammer : public Programmer
{
    Q_OBJECT

Q_SIGNALS:
    void waitActDone();

public:
    avr232bootProgrammer(ConnectionPointer<PortConnection> const & conn, ProgrammerLogSink * logsink);

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
    void cancelRequested();
    void sendTunnelData(QString const & data);

private slots:
    void dataRead(const QByteArray& data);

private:
    bool waitForAct(int waitAct, int timeout = 1000);
    void writeFlashPage(page& p);
    void writeEEPROMPage(page& p);

    enum {
        WAIT_NONE,

        WAIT_DEV_ID,
        WAIT_ACK,
        WAIT_EEPROM_READ
    };

    ConnectionPointer<PortConnection> m_conn;
    QByteArray m_rec_buff;
    int m_wait_act;
    bool m_flash_mode;
    bool m_cancel_requested;
};

#endif // AVR232BOOTPROGRAMMER_H
