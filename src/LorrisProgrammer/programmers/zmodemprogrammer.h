/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef ZMODEMPROGRAMMER_H
#define ZMODEMPROGRAMMER_H

#include "../../shared/programmer.h"
#include "../../connection/serialport.h"

class ZmodemProgrammer
    : public QObject
{
    Q_OBJECT
public:
    ZmodemProgrammer(ConnectionPointer<PortConnection> const & conn, ProgrammerLogSink * logsink);

    virtual bool supportsBootseq() const { return true; }
    virtual QString getBootseq() const;

    virtual void stopAll(bool wait);

    virtual void switchToFlashMode(quint32 prog_speed_hz);
    virtual void switchToRunMode();
    virtual bool isInFlashMode();
    virtual chip_definition readDeviceId();

    virtual QByteArray readMemory(const QString& mem, chip_definition &chip);
    virtual void readFuses(std::vector<quint8>& data, chip_definition &chip);
    virtual void writeFuses(std::vector<quint8>& data, chip_definition &chip, VerifyMode verifyMode);
    virtual void flashRaw(HexFile& file, quint8 memId, chip_definition& chip, VerifyMode verifyMode);

    virtual void erase_device(chip_definition& chip);

    virtual int getType();

    virtual ProgrammerCapabilities capabilities() const;

public slots:
    virtual void setBootseq(const QString& seq);

private slots:
    void dataRead(const QByteArray& data);

private:
    ConnectionPointer<PortConnection> m_conn;
    QString m_bootseq;
    bool m_flash_mode;
    bool m_cancel_requested;
    int m_wait_act;
};


#endif
