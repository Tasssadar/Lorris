/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SHUPITOSPITUNNEL_H
#define SHUPITOSPITUNNEL_H

#include "shupitomode.h"
#include "../../connection/shupitospitunnelconn.h"
#include "ui_shupitospitunnelwidget.h"

enum SpiModes
{
    SAMPLE_LEADING  = 0x00,
    SAMPLE_TRAILING = 0x01
};

enum SlaveSelectModes
{
    SS_HIGH_ON_START = 0x01,
    SS_HIGH_ON_END   = 0x02
};

class ShupitoSpiTunnel : public ShupitoMode
{
    Q_OBJECT

Q_SIGNALS:
    void spiStateSwitchComplete(bool success);

public:
    ShupitoSpiTunnel(Shupito *shupito);
    ~ShupitoSpiTunnel();

    void switchToFlashMode(quint32 speed_hz);

    virtual chip_definition readDeviceId();
    virtual void erase_device(chip_definition& chip);

    virtual ProgrammerCapabilities capabilities() const;
    virtual QList<QWidget*> widgets();
    virtual void setActive(bool active);

protected:
    virtual ShupitoDesc::config const *getModeCfg();
    virtual void flashPage(chip_definition::memorydef *memdef, std::vector<quint8>& memory, quint32 address);
    virtual void readMemRange(quint8 memid, QByteArray& memory, quint32 address, quint32 size);

private slots:
    void tunnelState(ConnectionState state);
    void writeSpiData(const QByteArray& data);
    void packetRead(const ShupitoPacket& p);
    void applySettings(int speed, quint8 sample_mode, quint8 ss_mode, bool lsb_first);

private:
    ConnectionPointer<ShupitoSpiTunnelConn> m_tunnel_conn;

    int m_tunnel_speed;
    quint8 m_sample_mode;
    quint8 m_ss_mode;
    bool m_lsb_first;
};

class ShupitoSpiTunnelWidget : public QWidget, private Ui::ShupitoSpiTunnelWidget
{
    Q_OBJECT

Q_SIGNALS:
    void applySettings(int speed, quint8 sample_mode, quint8 ss_mode, bool lsb_first);

public:
    ShupitoSpiTunnelWidget(int speed, quint8 sample_mode, quint8 ss_mode, bool lsb_first, QWidget *parent = NULL);
    ~ShupitoSpiTunnelWidget();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);
    void enableApplyBtn();

private:
    Ui::ShupitoSpiTunnelWidget *ui;
};

#endif // SHUPITOSPITUNNEL_H
