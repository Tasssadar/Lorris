/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef AVR109PROGRAMMER_H
#define AVR109PROGRAMMER_H

#include "../../shared/programmer.h"
#include "../../connection/serialport.h"

class avr109Programmer : public Programmer
{
    Q_OBJECT

Q_SIGNALS:
    void waitActDone();

public:
    avr109Programmer(ConnectionPointer<PortConnection> const & conn, ProgrammerLogSink * logsink);

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

public slots:
    void cancelRequested();
    void sendTunnelData(QString const & data);
    void setBootseq(const QString& seq);

private slots:
    void dataRead(const QByteArray& data);

private:
    bool waitForAct(int waitAct, int timeout = 1000);
    bool checkBlockSupport(int &block_size);
    bool checkAutoIncrement();
    void setAddress(quint32 address);
    QByteArray readMem(quint8 id, quint32 start, quint32 size);
    QByteArray readFlashMem(quint32 start, quint32 size, bool autoincrement);
    QByteArray readFlashMemBlock(quint32 start, quint32 size, int block_size);
    QByteArray readEEPROM(quint32 start, quint32 size, bool autoincrement);
    QByteArray readEEPROMBlock(quint32 start, quint32 size, int block_size);
    void writeFlashMem(const std::vector<page>& pages, const std::set<quint32>& skip, bool autoincrement);
    void writeFlashMemBlock(const std::vector<page>& pages, const std::set<quint32>& skip);
    void writeEEPROM(const std::vector<page>& pages, bool autoincrement);
    void writeEEPROMBlock(const std::vector<page>& pages);

    enum {
        WAIT_NONE,

        WAIT_EMPTY,
        WAIT_SUPPORTED,
        WAIT_CHAR1,
        WAIT_CHAR2,
        WAIT_CHAR3,
        WAIT_BLOCK
    };

    ConnectionPointer<PortConnection> m_conn;
    QByteArray m_rec_buff;
    int m_wait_act;
    int m_block_size;
    bool m_flash_mode;
    bool m_cancel_requested;
    QString m_bootseq;
};

#endif // AVR109PROGRAMMER_H
