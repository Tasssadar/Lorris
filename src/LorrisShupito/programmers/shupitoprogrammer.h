#ifndef SHUPITO_PROGRAMMER_H
#define SHUPITO_PROGRAMMER_H

#include "../shupito.h" // XXX
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

    bool supportsVdd() const { return true; }
    bool supportsTunnel() const;

    QStringList getAvailableModes();
    int getMode();
    void setMode(int mode);

    void stopAll(bool wait);

    void setVddIndex(int index);
    void setTunnelSpeed(quint32 speed, bool send);
    void setTunnelState(bool enable, bool wait);
    quint32 getTunnelSpeed() const;

    void switchToFlashMode(quint32 prog_speed_hz);
    void switchToRunMode();
    bool isInFlashMode();
    chip_definition readDeviceId();

    QByteArray readMemory(const QString& mem, chip_definition &chip);
    void readMemRange(quint8 memid, QByteArray& memory, quint32 address, quint32 size);
    void readFuses(std::vector<quint8>& data, chip_definition &chip);
    void writeFuses(std::vector<quint8>& data, chip_definition &chip, quint8 verifyMode);
    void flashRaw(HexFile& file, quint8 memId, chip_definition& chip, quint8 verifyMode);

    void erase_device(chip_definition& chip);

    ShupitoMode *mode() const { return m_modes[m_cur_mode]; }

public slots:
    void sendTunnelData(QString const & data);
    void cancelRequested();

private slots:
    void connectedStatus(bool connected);
    void readPacket(const ShupitoPacket & packet);
    void descRead(bool correct);

private:
    ShupitoDesc::config *m_vdd_config;
    ShupitoDesc::config *m_tunnel_config;
    ShupitoDesc::config *m_btn_config;

    Shupito *m_shupito;
    ShupitoMode *m_modes[MODE_COUNT];
    quint8 m_cur_mode;

    ShupitoDesc *m_desc;

    ConnectionPointer<ShupitoConnection> m_con;
};

#endif
