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

private:
    void initMenus();

    Ui::FullShupitoUI *ui;

    FuseWidget *m_fuse_widget;
    QHexEdit *m_hexAreas[MEM_FUSES];
    FlashButtonMenu *m_readBtnMenu;
    FlashButtonMenu *m_writeBtnMenu;
};

#endif // FULLSHUPITOUI_H
