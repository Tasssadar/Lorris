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
#include "../ui/terminal.h"
#include "../ui/connectbutton.h"
#include "ui/shupitoui.h"

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

enum TabIndex
{
    TAB_TERMINAL = 0,
    TAB_FLASH,
    TAB_EEPROM,
    TAB_MAX
};

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
class ToolTipWarn;

class LorrisShupito : public WorkTab
{
    Q_OBJECT
Q_SIGNALS:
    void responseChanged();
    void enableButtons(bool enable);

    friend class FullShupitoUI;
    friend class MiniShupitoUI;
    friend class ShupitoUI;
public:
    LorrisShupito();
    ~LorrisShupito();

    void stopAll(bool wait);
    void createConnBtn(QToolButton *btn);
    ShupitoMode *mode() const { return m_modes[m_cur_mode]; }

public slots:
    void setConnection(ConnectionPointer<Connection> const & con);

protected:
    ConnectionPointer<ShupitoConnection> m_con;

    QString GetIdString();

    void saveData(DataFileParser *file);
    void loadData(DataFileParser *file);

private slots:
    void onTabShow(const QString& filename);
    void connDisconnecting();
    void setMiniUi(bool mini);

    void connectedStatus(bool connected);
    void readPacket(const ShupitoPacket & data);
    void descRead(bool correct);

    void vccValueChanged(quint8 id, double value);
    void vddSetup(const vdd_setup& vs);
    void vddIndexChanged(int index);

    void tunnelSpeedChanged ( const QString & text );
    void tunnelToggled(bool enable);
    void tunnelStateChanged(bool opened);
    void setTunnelName();
    void verifyChanged(int mode);
    void progSpeedChanged(QString text);

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

    void timeout();

private:
    bool checkVoltage(bool active);
    void showProgressDialog(const QString& text, QObject *sender = NULL);
    bool showContinueBox(const QString& title, const QString& text);
    void postFlashSwitchCheck(chip_definition &chip);
    chip_definition switchToFlashAndGetId();
    void update_chip_description(chip_definition &cd);
    void initMenus();

    void checkOvervoltage();
    void shutdownVcc();
    void tryFileReload(quint8 memId);
    void setUiType(int type);

    void setEnableButtons(bool enable);

    ShupitoUI *ui;

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
    QAction *m_miniUi;
    QMenu *m_modeBar;

    quint8 m_state;
    Shupito *m_shupito;
    ShupitoDesc *m_desc;
    quint32 m_prog_speed_hz;
    quint8 m_verify_mode;

    QString m_hexFilenames[MEM_FUSES];
    QDateTime m_hexWriteTimes[MEM_FUSES];
    QDateTime m_hexFlashTimes[MEM_FUSES];
    ShupitoMode *m_modes[MODE_COUNT];
    quint8 m_cur_mode;

    vdd_setup m_vdd_setup;
    double m_vcc;
    int lastVccIndex;
    double m_overvcc;
    bool m_enable_overvcc;
    bool m_overvcc_turnoff;
    QPointer<OverVccDialog> m_overvcc_dialog;

    ShupitoDesc::config *m_vdd_config;
    ShupitoDesc::config *m_tunnel_config;

    ProgressDialog *m_progress_dialog;

    chip_definition m_cur_def;

    ConnectButton * m_connectButton;

    QTimer m_timeout_timer;
    QPointer<ToolTipWarn> m_timeout_warn;

    bool m_buttons_enabled;
};

#endif // LORRISSHUPITO_H
