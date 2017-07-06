/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef LORRISPROGRAMMER_H
#define LORRISPROGRAMMER_H

#include "../WorkTab/WorkTab.h"
#include "shupito.h"
#include "shupitodesc.h"
#include "../shared/hexfile.h"
#include "../ui/terminal.h"
#include "../ui/connectbutton.h"
#include "ui/programmerui.h"

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

class LorrisProgrammer : public WorkTab
{
    Q_OBJECT

Q_SIGNALS:
    void responseChanged();

    friend class FullProgrammerUI;
    friend class MiniProgrammerUI;
    friend class ProgrammerUI;
public:
    LorrisProgrammer();
    ~LorrisProgrammer();

    void stopAll(bool wait);
    void createConnBtn(QToolButton *btn);
    ConnectButton *getConnBtn() const { return m_connectButton; };

public slots:
    void setConnection(ConnectionPointer<Connection> const & con);

protected:
    ConnectionPointer<Connection> m_con;

    QString GetIdString();

    void saveData(DataFileParser *file);
    void loadData(DataFileParser *file);

private slots:
    void onTabShow(const QString& filename);
    void connDisconnecting();
    void setMiniUi(bool mini);

    void connectedStatus(bool connected);

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

    void loadFromFile();
    void loadFromFile(int memId, const QString& filename);
    void saveToFile();
    void focusChanged(QWidget *prev, QWidget *curr);

    void timeout();

    void updateModeBar();

    void buttonPressed(int btnid);
    void enableHardwareButtonToggled(bool checked);

    void blinkLed();

    void updateProgrammer();

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

    QString getFileDialogFilter(int memid);

    ProgrammerUI *ui;

    bool m_chipStopped;

    QAction *m_start_act;
    QAction *m_stop_act;
    QAction *m_restart_act;
    QAction *m_verify[VERIFY_MAX];
    QAction *m_load_flash;
    QAction *m_save_flash;
    QAction *m_blink_led;
    QAction *m_miniUi;
    QAction *m_set_tunnel_name_act;

    QMenu *m_modeBar;
    std::vector<QAction *> m_mode_acts;
    QSignalMapper * m_mode_act_signalmap;

    quint8 m_state;
    quint32 m_prog_speed_hz;
    VerifyMode m_verify_mode;

    QString m_hexFilenames[MEM_COUNT];
    QDateTime m_hexWriteTimes[MEM_COUNT];
    QDateTime m_hexFlashTimes[MEM_COUNT];

    vdd_setup m_vdd_setup;
    double m_vcc;
    int lastVccIndex;
    double m_overvcc;
    bool m_enable_overvcc;
    bool m_overvcc_turnoff;
    QPointer<OverVccDialog> m_overvcc_dialog;

    ProgressDialog *m_progress_dialog;

    chip_definition m_cur_def;

    ConnectButton * m_connectButton;

    QTimer m_timeout_timer;
    QPointer<ToolTipWarn> m_timeout_warn;

    bool m_buttons_enabled;

    QScopedPointer<Programmer> m_programmer;

    struct LogSink
        : ProgrammerLogSink
    {
        explicit LogSink(LorrisProgrammer * self)
            : m_self(self)
        {
        }

        void log(QString const & msg)
        {
            m_self->ui->log(msg);
        }

        LorrisProgrammer * m_self;
    };

    LogSink m_logsink;

    QAction * m_enableHardwareButton;
};

#endif // LORRISSHUPITO_H
