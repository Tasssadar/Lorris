/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef PROGRAMMERUI_H
#define PROGRAMMERUI_H

#include <QObject>
#include <vector>

#include "../shupito.h"
#include "../../shared/hexfile.h"
#include "../../shared/programmer.h"

class LorrisProgrammer;
class QSignalMapper;
class QRadioButton;
class ShupitoMode;
class DataFileParser;
class QToolButton;
class QLabel;

enum ui_type
{
    UI_FULL,
    UI_MINIMAL,

    UI_MAX
};

enum VddColor
{
    VDD_BLACK = 0,
    VDD_GREEN,
    VDD_RED,
    VDD_ORANGE
};

class ProgrammerUI : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void statusBarMsg(const QString& text, int time);

public:   
     ~ProgrammerUI();

    static ProgrammerUI *createUI(ui_type type, QObject *parent);

    ui_type getType() const { return m_ui_type; }

    virtual void setupUi(LorrisProgrammer *widget);
    virtual void connectProgrammer(Programmer *) { }

    virtual void connectedStatus(bool connected);
    virtual void tunnelStop(bool /*stop*/) { }
    virtual void setTunnelActive(bool /*active*/) {}
    virtual void tunnelStateChanged(bool /*opened*/) {}
    virtual void log(const QString&) { }
    virtual void postFlashSwitchCheck(chip_definition&) { }
    virtual void setActiveMem(quint32) { }
    virtual void warnSecondFlash() { }
    virtual int getMemIndex() { return MEM_FLASH; }

    virtual void setHexData(quint32 memid, const QByteArray& data) = 0;
    virtual void setHexColor(quint32 /*memid*/, const QString&) { }
    virtual QByteArray getHexData(quint32 memid) const = 0;
    virtual void clearHexChanged(quint32 /*memid*/) {}
    virtual bool hasHexChanged(quint32 /*memid*/) { return false; }

    virtual void saveData(DataFileParser *file);
    virtual void loadData(DataFileParser *file);

    void vddIndexChanged(quint32 idx);
    void clearVCC();
    void vccValChanged(double val);
    void vddSetup(const vdd_setup &vs);
    void setStartStopBtn(bool start);
    virtual void setFileAndTime(const QString& /*file*/, const QDateTime&) {}
    virtual void setChipId(const QString&) {}

    void readMemInFlash(quint8 memId);
    void writeMemInFlash(quint8 memId);
    void readMem(quint8 memId, chip_definition& chip);
    void writeMem(quint8 memId, chip_definition& chip);
    virtual void readFuses(chip_definition &) { }
    virtual void writeFuses(chip_definition&) { }
    virtual void readFusesInFlash() { }
    virtual void writeFusesInFlash() { }

    virtual void writeSelectedMem() {}

    virtual void enableButtons(bool enable) = 0;

protected slots:
    void readMemButton() { readMemInFlash(MEM_FLASH); }
    void readEEPROMBtn() { readMemInFlash(MEM_EEPROM); }
    void writeFlashBtn() { writeMemInFlash(MEM_FLASH); }
    void writeEEPROMBtn(){ writeMemInFlash(MEM_EEPROM); }

    void readAll();
    void writeAll();
    void eraseDevice();

protected:
    ProgrammerUI(ui_type type, QObject *parent = 0);

    Programmer *prog() const;

    void disableOvervoltVDDs();
    void changeVddColor(double val);
    void status(const QString& text)
    {
        emit statusBarMsg(text, 5000);
    }

    virtual QToolButton *startStopBtn() const = 0;
    virtual QBoxLayout *vddLayout() const = 0;
    virtual QLabel *engineLabel() const = 0;
    virtual QLabel *vccLabel() const = 0;

    LorrisProgrammer *m_widget;
    std::vector<QRadioButton*> m_vdd_radios;
    QSignalMapper *m_vdd_signals;

    VddColor m_color;

private:
    ui_type m_ui_type;
};

#endif // PROGRAMMERUI_H
