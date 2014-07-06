/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SHUPITO_PROGRAMMER_H
#define SHUPITO_PROGRAMMER_H

#include "../shupito.h"
#include "../../connection/shupitoconn.h"
#include "../modes/shupitomode.h"
#include "../../shared/programmer.h"

class ShupitoProgrammer
    : public Programmer
{
    Q_OBJECT

public:
    ShupitoProgrammer(ConnectionPointer<ShupitoConnection> const & conn, ProgrammerLogSink * logsink);
    ~ShupitoProgrammer();

    bool supportsPwm() const override;
    bool setPwmFreq(uint32_t freq_hz, float duty_cycle) override;

    bool supportsVdd() const override { return true; }

    QStringList getAvailableModes() override;
    int getMode() override;
    void setMode(int mode) override;

    void stopAll(bool wait) override;

    void setVddIndex(int index) override;
    void setTunnelSpeed(quint32 speed, bool send) override;
    void setTunnelState(bool enable, bool wait) override;
    quint32 getTunnelSpeed() const override;

    void switchToFlashMode(quint32 prog_speed_hz) override;
    void switchToRunMode() override;
    bool isInFlashMode() override;
    chip_definition readDeviceId() override;

    QByteArray readMemory(const QString& mem, chip_definition &chip) override;
    void readFuses(std::vector<quint8>& data, chip_definition &chip) override;
    void writeFuses(std::vector<quint8>& data, chip_definition &chip, VerifyMode verifyMode) override;
    void flashRaw(HexFile& file, quint8 memId, chip_definition& chip, VerifyMode verifyMode) override;

    void executeText(QByteArray const & data, quint8 memId, chip_definition & chip) override;

    void erase_device(chip_definition& chip) override;

    ShupitoMode *mode() const { return m_modes[m_cur_mode]; }

    bool canBlinkLed() override;
    void blinkLed() override;

    int getType() override;

    virtual ProgrammerCapabilities capabilities() const override;
    virtual QList<QWidget*> widgets() override;

public slots:
    void sendTunnelData(QString const & data);
    void cancelRequested();

private slots:
    void connectedStatus(bool connected);
    void readPacket(const ShupitoPacket & packet);
    void descRead(bool correct);

private:
    ConnectionPointer<ShupitoConnection> m_con;

    ShupitoDesc::config const *m_vdd_config;
    ShupitoDesc::config const *m_tunnel_config;
    ShupitoDesc::config const *m_btn_config;
    ShupitoDesc::config const *m_led_config;
    ShupitoDesc::config const *m_pwm_config;

    Shupito *m_shupito;
    ShupitoMode *m_modes[MODE_COUNT];
    quint8 m_cur_mode;

    ShupitoDesc *m_desc;
};

#endif
