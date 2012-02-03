/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef LORRISSHUPITO_H
#define LORRISSHUPITO_H

#include "WorkTab/WorkTab.h"
#include "shupito.h"
#include "shupitodesc.h"
#include "shared/hexfile.h"

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

class LorrisShupito : public WorkTab
{
    Q_OBJECT
Q_SIGNALS:
    void responseChanged();

public:
    LorrisShupito();
    ~LorrisShupito();

    void stopAll();

private slots:
    void connectButton();
    void onTabShow();

    void connectionResult(Connection*,bool);
    void connectedStatus(bool connected);
    void readData(const QByteArray& data);
    void descRead();

    void responseReceived(char error_code);
    void vccValueChanged(quint8 id, double value);
    void vddSetup(const vdd_setup& vs);
    void vddIndexChanged(int index);

    void tunnelSpeedChanged ( const QString & text );
    void tunnelToggled(bool enable);
    void tunnelStateChanged(bool opened);
    void setTunnelName();

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

    void startChip();
    void stopChip();
    void restartChip();

    void modeSelected(int idx);
    void status(const QString& text);

    void loadFromFile(int memId);
    void saveToFile(int memId);

private:
    void sendAndWait(const QByteArray &data);
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
    void showErrorBox(const QString& text);
    bool showContinueBox(const QString& title, const QString& text);
    void postFlashSwitchCheck(chip_definition &chip);
    chip_definition switchToFlashAndGetId();
    void update_chip_description(chip_definition &cd);
    void initMenus();

    void changeVddColor(float val);

    QAction *m_start_act;
    QAction *m_stop_act;
    QAction *m_restart_act;
    QAction *m_mode_act[MODE_COUNT];
    QAction *m_auto_verify;
    QAction *m_load_flash;
    QAction *m_load_eeprom;
    QAction *m_save_flash;
    QAction *m_save_eeprom;

    Ui::LorrisShupito *ui;
    quint8 m_state;
    Shupito *m_shupito;
    ShupitoDesc *m_desc;
    QTimer *responseTimer;
    volatile quint8 m_response;
    quint32 m_prog_speed_hz;

    QHexEdit *m_hexAreas[MEM_FUSES];
    ShupitoMode *m_modes[MODE_COUNT];
    quint8 m_cur_mode;

    vdd_setup m_vdd_setup;
    double m_vcc;
    int lastVccIndex;
    VddColor m_color;

    std::vector<QRadioButton*> m_vdd_radios;
    QSignalMapper *m_vdd_signals;

    ShupitoDesc::config *m_vdd_config;
    ShupitoDesc::config *m_tunnel_config;

    QProgressDialog *m_progress_dialog;
    FuseWidget *m_fuse_widget;

    chip_definition m_cur_def;
};

#endif // LORRISSHUPITO_H
