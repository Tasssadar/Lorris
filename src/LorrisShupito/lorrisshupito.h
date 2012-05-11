/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef LORRISSHUPITO_H
#define LORRISSHUPITO_H

#include "../WorkTab/WorkTab.h"
#include "shupito.h"
#include "shupitodesc.h"
#include "../shared/hexfile.h"
#include "../shared/terminal.h"
#include "../ui/connectbutton.h"

#include <QDateTime>
#include <QPointer>

enum state
{
    STATE_DISCONNECTED = 0x01
};

enum responses
{
    RESPONSE_NONE,
    RESPONSE_WAITING,
    RESPONSE_GOOD,
    RESPONSE_BAD
};

enum VddColor
{
    VDD_BLACK = 0,
    VDD_GREEN,
    VDD_RED,
    VDD_ORANGE
};

enum TabIndex
{
    TAB_FLASH = 0,
    TAB_EEPROM,
    TAB_TERMINAL,
    TAB_MAX
};

namespace Ui {
    class LorrisShupito;
}

class QLabel;
class QComboBox;
class QHexEdit;
class QRadioButton;
class QProgressDialog;
class QSignalMapper;
class ShupitoMode;
class chip_definition;
class FuseWidget;
class ProgressDialog;
class OverVccDialog;

class LorrisShupito : public PortConnWorkTab
{
    Q_OBJECT
Q_SIGNALS:
    void responseChanged();

public:
    LorrisShupito();
    ~LorrisShupito();

    void setConnection(PortConnection *con);
    void stopAll(bool wait);

private slots:
    void onTabShow();
    void connDisconnecting();

    void connectionResult(Connection*,bool);
    void connectedStatus(bool connected);
    void readData(const QByteArray& data);
    void descRead(bool correct);

    void vccValueChanged(quint8 id, double value);
    void vddSetup(const vdd_setup& vs);
    void vddIndexChanged(int index);

    void tunnelSpeedChanged ( const QString & text );
    void tunnelToggled(bool enable);
    void tunnelStateChanged(bool opened);
    void setTunnelName();
    void verifyChanged(int mode);

    void hideLogBtn();
    void hideFusesBtn();

    void readMemButton()
    {
        readMemInFlash(MEM_FLASH);
    }
    void readEEPROMBtn()
    {
        readMemInFlash(MEM_EEPROM);
    }

    void readAll();
    void writeAll();
    void progSpeedChanged(QString text);
    void eraseDevice();
    void readFusesInFlash();
    void writeFusesInFlash();
    void writeFlashBtn()
    {
        writeMemInFlash(MEM_FLASH);
    }
    void writeEEPROMBtn()
    {
        writeMemInFlash(MEM_EEPROM);
    }

    void updateProgressDialog(int value);
    void updateProgressLabel(const QString& text);

    void startstopChip();
    void startChip();
    void stopChip();
    void restartChip();
    void updateStartStopUi(bool stopped);

    void modeSelected(int idx);
    void status(const QString& text);

    void openFile(const QString &filename);
    void loadFromFile()
    {
        loadFromFile(MEM_FLASH);
    }

    void loadFromFile(int memId);
    void loadFromFile(int memId, const QString& filename);
    void saveToFile(int memId);
    void focusChanged(QWidget *prev, QWidget *curr);
    void saveTermFont(const QString& fontData);

    void overvoltageSwitched(bool enabled);
    void overvoltageChanged(double val);
    void overvoltageTurnOffVcc(bool enabled);

private:
    void log(const QString& text);
    bool checkVoltage(bool active);
    void readMemInFlash(quint8 memId);
    void writeMemInFlash(quint8 memId);
    void readMem(quint8 memId, chip_definition& chip);
    void writeMem(quint8 memId, chip_definition& chip);
    void readFuses(chip_definition& chip);
    void writeFuses(chip_definition& chip);
    void hideFuses(bool hide);
    void showProgressDialog(const QString& text, QObject *sender = NULL);
    bool showContinueBox(const QString& title, const QString& text);
    void postFlashSwitchCheck(chip_definition &chip);
    chip_definition switchToFlashAndGetId();
    void update_chip_description(chip_definition &cd);
    void initMenus();

    void changeVddColor(float val);
    void checkOvervoltage();
    void shutdownVcc();
    void disableOvervoltVDDs();
    void tryFileReload(quint8 memId);
    inline int getMemIndex();

    bool m_chipStopped;

    QAction *m_start_act;
    QAction *m_stop_act;
    QAction *m_restart_act;
    QAction *m_mode_act[MODE_COUNT];
    QAction *m_verify[VERIFY_MAX];
    QAction *m_load_flash;
    QAction *m_load_eeprom;
    QAction *m_save_flash;
    QAction *m_save_eeprom;

    Ui::LorrisShupito *ui;
    quint8 m_state;
    Shupito *m_shupito;
    ShupitoDesc *m_desc;
    quint32 m_prog_speed_hz;
    quint8 m_verify_mode;

    QHexEdit *m_hexAreas[MEM_FUSES];
    Terminal *m_terminal;
    QString m_hexFilenames[MEM_FUSES];
    QDateTime m_hexWriteTimes[MEM_FUSES];
    ShupitoMode *m_modes[MODE_COUNT];
    quint8 m_cur_mode;

    vdd_setup m_vdd_setup;
    double m_vcc;
    int lastVccIndex;
    VddColor m_color;
    double m_overvcc;
    bool m_enable_overvcc;
    QPointer<OverVccDialog> m_overvcc_dialog;

    std::vector<QRadioButton*> m_vdd_radios;
    QSignalMapper *m_vdd_signals;

    ShupitoDesc::config *m_vdd_config;
    ShupitoDesc::config *m_tunnel_config;

    ProgressDialog *m_progress_dialog;
    FuseWidget *m_fuse_widget;

    chip_definition m_cur_def;

    ConnectButton * m_connectButton;
};

#endif // LORRISSHUPITO_H
