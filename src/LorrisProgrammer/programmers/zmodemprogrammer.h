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
    : public Programmer
{
    Q_OBJECT
Q_SIGNALS:
    void waitPktDone();

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

    virtual void cancelRequested();

public slots:
    virtual void setBootseq(const QString& seq);

private slots:
    void dataRead(const QByteArray& data);

private:
    void sendHexHeader(quint8 type, quint8 a1 = 0, quint8 a2 = 0, quint8 a3 = 0, quint8 a4 = 0);
    void sendBin32Header(quint8 type, quint8 a1 = 0, quint8 a2 = 0, quint8 a3 = 0, quint8 a4 = 0);
    void sendBin32Data(quint8 type, const QByteArray& data);
    void appendHex(QByteArray& dest, quint8 val);
    void appendEscapedByte(QByteArray &dest, quint8 c);
    bool rx_byte(int &c);
    bool rx_hex_byte(int &c);
    void processHeader(const QByteArray& hdr);
    bool waitForPkt(int waitPkt, int timeout);

    ConnectionPointer<PortConnection> m_conn;
    QString m_bootseq;
    bool m_flash_mode;
    bool m_cancel_requested;
    int m_wait_pkt;
    QByteArray m_wait_hdr;

    int m_recv_state;
    bool m_escape_ctrl_chars;
    bool m_inside_zdle_seq;
    bool m_drop_newline;
    bool m_recv_32bit_data;
    QByteArray m_recv_buff;
    QByteArray m_hex_nibble_buff;
    int m_send_bufsize;
};



#endif
