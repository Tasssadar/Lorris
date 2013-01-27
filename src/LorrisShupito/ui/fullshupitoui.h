/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef FULLSHUPITOUI_H
#define FULLSHUPITOUI_H

#include <QObject>
#include "shupitoui.h"
#include "../../shared/hexfile.h"

#include "ui_fullshupitoui.h"

class FuseWidget;
class FlashButtonMenu;
class QHexEdit;

class FullShupitoUI : public ShupitoUI
{
    Q_OBJECT
public:
    explicit FullShupitoUI(QObject *parent = 0);
    ~FullShupitoUI();

    void setupUi(LorrisShupito *widget);
    void connectProgrammer(Programmer *prog);
    void connectedStatus(bool connected);
    void tunnelStop(bool stop);
    void setTunnelActive(bool active);
    void tunnelStateChanged(bool opened);
    void log(const QString& text);
    void setChipId(const QString &text);
    void postFlashSwitchCheck(chip_definition &chip);
    void setStartStopBtn(bool start);
    void setFileAndTime(const QString &file, const QDateTime &time);
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

    void writeSelectedMem();

protected:
    QToolButton *startStopBtn() const { return ui->startStopBtn; }
    QBoxLayout *vddLayout() const { return ui->vddLayout; }
    QLabel *engineLabel() const { return ui->engineLabel; }
    QLabel *vccLabel() const { return ui->vccLabel; }

private slots:
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

private:
    void initMenus();
    void updateProgrammersBox(Programmer *prog);
    void updateTunnelSupport();

    Ui::FullShupitoUI *ui;

    FuseWidget *m_fuse_widget;
    QHexEdit *m_hexAreas[MEM_FUSES];

    QFont m_font;
    QFont m_boldFont;

    enum ActionSlots
    {
        ACT_FLASH  = 0,
        ACT_EEPROM,
        ACT_ALL,
        ACT_FUSES
    };
    ActionSlots m_active;

    QMenu * m_read_menu;
    QMenu * m_write_menu;

    std::map<ActionSlots, QAction*> m_read_actions;
    std::map<ActionSlots, QAction*> m_write_actions;

    void createActions();
};

#endif // FULLSHUPITOUI_H
