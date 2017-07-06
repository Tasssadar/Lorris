/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef FULLPROGRAMMERUI_H
#define FULLPROGRAMMERUI_H

#include <QObject>
#include "programmerui.h"
#include "../../shared/hexfile.h"

#include "ui_fullprogrammerui.h"

class FuseWidget;
class FlashButtonMenu;
class QHexEdit;

class FullProgrammerUI : public ProgrammerUI
{
    Q_OBJECT
public:
    explicit FullProgrammerUI(QObject *parent = 0);
    ~FullProgrammerUI();

    void setupUi(LorrisProgrammer *widget);
    void connectProgrammer(Programmer *prog);
    void connectedStatus(bool connected);
    void tunnelStop(bool stop);
    void setTunnelActive(bool active);
    void tunnelStateChanged(bool opened);
    void log(const QString& text);
    void setChipId(const QString &text);
    void postFlashSwitchCheck(chip_definition &chip);
    void setStartStopBtn(bool start);
    void setFileAndTime(const QString &file, const QDateTime &time) override;
    void setFileNeverFlashed(bool neverFlashed) override;
    void setActiveMem(quint32 memId);
    void warnSecondFlash();
    int getMemIndex();

    void saveData(DataFileParser *file);
    void loadData(DataFileParser *file);

    void setHexData(quint32 memid, const QByteArray& data);
    void setHexColor(quint32 memid, const QString &clr);
    QByteArray getHexData(quint32 memid) const;
    void clearHexChanged(quint32 memid);
    bool hasHexChanged(quint32 memid);

    void readFuses(chip_definition &chip);
    void writeFuses(chip_definition &chip);

    void writeSelectedMem() override;

    void enableButtons(bool enable) override;

protected:
    QToolButton *startStopBtn() const { return ui->startStopBtn; }
    QBoxLayout *vddLayout() const { return ui->vddLayout; }
    QLabel *engineLabel() const { return ui->engineLabel; }
    QLabel *vccLabel() const { return ui->vccLabel; }

    void programmerCapsChanged() override;

private slots:
    void readMemButton() { readMemInFlash(MEM_FLASH); }
    void readEEPROMBtn() { readMemInFlash(MEM_EEPROM); }
    void writeFlashBtn() { writeMemInFlash(MEM_FLASH); }
    void writeEEPROMBtn(){ writeMemInFlash(MEM_EEPROM); }

    void hideLogBtn(bool checked);
    void hideFusesBtn(bool checked);
    void hideSettingsBtn(bool checked);

    void saveTermSettings();

    void readFusesInFlash();
    void writeFusesInFlash();

    void flashWarnBox(bool checked);

    void overvoltageSwitched(bool enabled);
    void overvoltageChanged(double val);
    void overvoltageTurnOffVcc(bool enabled);

    void setActiveAction(int actInt);

    void readButtonClicked();
    void writeButtonClicked();

    void pwmRadioClicked(int id);
    void pwmChanged(uint32_t freq_hz, float duty_cycle);
    void setPwmGeneric(bool force = false);
    void setPwmServo(bool force = false);

    void hexEditMenuReq(const QPoint& p);

    void progButtonClicked();

private:
    void initMenus();
    void updateProgrammersBox(Programmer *prog);

    Ui::FullProgrammerUI *ui;

    FuseWidget *m_fuse_widget;
    QHexEdit *m_hexAreas[MEM_FUSES];
    QTextEdit * m_svfEdit;
    void applySources();

    enum tabs_t
    {
        tab_widgets,
        tab_terminal,
        tab_flash,
        tab_eeprom,
        tab_svf
    };

    tabs_t currentTab() const;
    void setCurrentTab(tabs_t t);

    QFont m_font;
    QFont m_boldFont;

    enum ActionSlots
    {
        ACT_FLASH  = 0,
        ACT_EEPROM,
        ACT_ALL,
        ACT_FUSES,
        ACT_SVF,
    };
    ActionSlots m_active;

    QMenu * m_read_menu;
    QMenu * m_write_menu;

    std::map<ActionSlots, QAction*> m_read_actions;
    std::map<ActionSlots, QAction*> m_write_actions;

    void createActions();
};

#endif // FULLPROGRAMMERUI_H
