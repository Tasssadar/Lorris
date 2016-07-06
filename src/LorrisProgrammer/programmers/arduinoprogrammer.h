#ifndef ARDUINOPROGRAMMER_H
#define ARDUINOPROGRAMMER_H

#include <QTimer>

#include "../../shared/programmer.h"
#include "../../connection/serialport.h"

class ArduinoProgrammer : public Programmer {
    Q_OBJECT

Q_SIGNALS:
    void waitActDone();

public:
    ArduinoProgrammer(ConnectionPointer<SerialPort> const & conn, ProgrammerLogSink * logsink);
    ~ArduinoProgrammer();

    virtual void stopAll(bool wait);

    virtual void switchToFlashMode(quint32 prog_speed_hz);
    virtual void switchToRunMode();
    virtual bool isInFlashMode() { return m_flash_mode; }
    virtual chip_definition readDeviceId();

    virtual QByteArray readMemory(const QString& mem, chip_definition &chip);
    virtual void readFuses(std::vector<quint8>& data, chip_definition &chip);
    virtual void writeFuses(std::vector<quint8>& data, chip_definition &chip, VerifyMode verifyMode);
    virtual void flashRaw(HexFile& file, quint8 memId, chip_definition& chip, VerifyMode verifyMode);

    virtual void erase_device(chip_definition& chip);

    virtual int getType();

    virtual ProgrammerCapabilities capabilities() const;

public slots:
    virtual void cancelRequested();
    void sendTunnelData(QString const & data);

private slots:
    void dataRead(const QByteArray& data);
    void stayInBootloader();

private:
    bool waitForAct(int waitAct, int timeout);
    void setStayInBootloaderTimer(bool run);
    QByteArray readPage(quint16 address, quint16 pagesize, quint8 memId);

    ConnectionPointer<SerialPort> m_conn;
    bool m_flash_mode;
    bool m_ignore_incoming;
    bool m_cancel_requested;

    enum {
        WAIT_NONE,

        WAIT_RTS_DTR,
        WAIT_SYNC,
        WAIT_DEVICE_ID,
        WAIT_PAGE_READ,
    };

    int m_wait_act;
    int m_cur_page_size;
    QByteArray m_rec_buff;
    QTimer m_stay_in_bl_timer;
};

#endif // ARDUINOPROGRAMMER_H
