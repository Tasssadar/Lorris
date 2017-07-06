/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QRadioButton>
#include <QSignalMapper>
#include <QFileInfo>
#include <QMessageBox>
#include <QStringBuilder>

#include "programmerui.h"
#include "fullprogrammerui.h"
#include "../lorrisprogrammer.h"
#include "../../ui/tooltipwarn.h"
#include "../modes/shupitomode.h"
#include "miniprogrammerui.h"

static const QString colorFromDevice = "#C0FFFF";
static const QString colorFromFile   = "#C0FFC0";
static const QString colorSavedToFile= "#FFE0E0";
static const QString memNames[] = { "", "flash", "eeprom", "fuses", "sdram" };

ProgrammerUI::ProgrammerUI(ui_type type, QObject *parent) :
    QObject(parent)
{
    m_ui_type = type;
    m_vdd_signals = NULL;
    m_color = VDD_BLACK;
}

ProgrammerUI::~ProgrammerUI()
{

}

ProgrammerUI *ProgrammerUI::createUI(ui_type type, QObject *parent)
{
    switch(type)
    {
    case UI_FULL:
        return new FullProgrammerUI(parent);
    case UI_MINIMAL:
        return new MiniProgrammerUI(parent);
    default:
        break;
    }
    return NULL;
}

void ProgrammerUI::setupUi(LorrisProgrammer *widget)
{
    m_widget = widget;
    connect(this,   SIGNAL(statusBarMsg(QString,int)), widget, SIGNAL(statusBarMsg(QString,int)));
}

void ProgrammerUI::connectProgrammer(Programmer * prog)
{
    connect(prog, SIGNAL(capabilitiesChanged()), this, SLOT(updateProgrammerCaps()));
    m_programmer_caps = prog->capabilities();
}

Programmer *ProgrammerUI::prog() const
{
    return m_widget->m_programmer.data();
}

void ProgrammerUI::clearVCC()
{
    for(quint8 i = 0; i < m_vdd_radios.size(); ++i)
        delete m_vdd_radios[i];
    m_vdd_radios.clear();

    vccLabel()->clear();
    engineLabel()->clear();
}

void ProgrammerUI::vccValChanged(double val)
{
    static const QString base = "%1V";
    vccLabel()->setText(base.arg(val, 3, 'f', 2, '0'));

    changeVddColor(val);
}

void ProgrammerUI::changeVddColor(double val)
{
    VddColor newClr;
    if     (val == 0)                newClr = VDD_BLACK;
    else if(val > 0 && val <= 2.7)   newClr = VDD_ORANGE;
    else if(val > 2.7 && val <= 5.5) newClr = VDD_GREEN;
    else                             newClr = VDD_RED;

    if(newClr == m_color)
        return;

    static const QString vddColorHTML[] =
    {
        "#000000", // VDD_BLACK
        "#009900", // VDD_GREEN
        "#FF0000", // VDD_RED
        "#996600", // VDD_ORANGE
    };

    vccLabel()->setStyleSheet("color: " % vddColorHTML[newClr]);
    m_color = newClr;
}

void ProgrammerUI::vddSetup(const vdd_setup &vs)
{
    if(vs.empty())
        return;

    engineLabel()->setText(vs[0].name);

    if(m_vdd_signals)
    {
        disconnect(m_widget, SLOT(vddIndexChanged(int)));
        delete m_vdd_signals;
    }
    m_vdd_signals = new QSignalMapper(this);

    for(quint8 i = 0; i < vs[0].drives.size() && i < 5; ++i)
    {
        QRadioButton *rad = new QRadioButton(vs[0].drives[i], m_widget);
        m_vdd_radios.push_back(rad);
        vddLayout()->addWidget(rad);

        m_vdd_signals->setMapping(rad, i);
        connect(rad, SIGNAL(clicked()), m_vdd_signals, SLOT(map()));
    }
    int lastVccIndex = vs[0].current_drive;

    if(m_vdd_radios.size() > (uint)lastVccIndex)
        m_vdd_radios[lastVccIndex]->setChecked(true);

    disableOvervoltVDDs();

    connect(m_vdd_signals, SIGNAL(mapped(int)), m_widget, SLOT(vddIndexChanged(int)));
}

void ProgrammerUI::disableOvervoltVDDs()
{
    bool ok = false;
    for(std::vector<QRadioButton*>::iterator itr = m_vdd_radios.begin(); itr != m_vdd_radios.end(); ++itr)
    {
        if((*itr)->text() == "<hiz>")
            continue;

        QStringList split = (*itr)->text().split("V,", QString::SkipEmptyParts);
        if(split.size() != 2)
            return;

        double val = split[0].toDouble(&ok);
        if(!ok)
            continue;
        (*itr)->setEnabled(val < m_widget->m_overvcc);
    }
}

void ProgrammerUI::vddIndexChanged(quint32 idx)
{
    if(idx >= m_vdd_radios.size())
        return;

    m_vdd_radios[idx]->setChecked(true);
}

void ProgrammerUI::connectedStatus(bool connected)
{
    if(!connected)
        clearVCC();
}

void ProgrammerUI::readAll()
{
    if(!m_widget->checkVoltage(true))
        return;

    status(QString());

    try
    {
        bool restart = !prog()->isInFlashMode();
        chip_definition chip = m_widget->switchToFlashAndGetId();

        readMem(MEM_FLASH, chip);
        readMem(MEM_EEPROM, chip);
        readFuses(chip);

        if(restart)
        {
            log("Switching to run mode");
            prog()->switchToRunMode();
        }

        status(tr("Data has been successfuly read"));
    }
    catch(QString ex)
    {
        m_widget->updateProgressDialog(-1);
        Utils::showErrorBox(ex);
    }
}

void ProgrammerUI::readMemInFlash(quint8 memId)
{
    if(!m_widget->checkVoltage(true))
        return;

    try
    {
        bool restart = !prog()->isInFlashMode();
        chip_definition chip = m_widget->switchToFlashAndGetId();

        readMem(memId, chip);

        if(restart)
        {
            log("Switching to run mode");
            prog()->switchToRunMode();
        }

        setActiveMem(memId);
    }
    catch(QString ex)
    {
        m_widget->updateProgressDialog(-1);

        Utils::showErrorBox(ex);
    }
}

void ProgrammerUI::readMem(quint8 memId, chip_definition &chip)
{
    log("Reading memory");

    m_widget->showProgressDialog(tr("Reading memory"), prog());

    QByteArray mem = prog()->readMemory(memNames[memId], chip);
    setHexData(memId, mem);
    setHexColor(memId, colorFromDevice);
    m_widget->m_hexFilenames[memId].clear();

    m_widget->updateProgressDialog(-1);
}

void ProgrammerUI::writeAll()
{
    if(!m_widget->checkVoltage(true))
        return;

    status("");

    try
    {
        bool restart = !prog()->isInFlashMode();
        chip_definition chip = m_widget->switchToFlashAndGetId();

        writeMem(MEM_FLASH, chip);
        writeMem(MEM_EEPROM, chip);
        writeFuses(chip);

        if(restart)
        {
            log("Switching to run mode");
            prog()->switchToRunMode();
        }

        status(tr("Data has been successfuly written"));
    }
    catch(QString ex)
    {
        m_widget->updateProgressDialog(-1);
        Utils::showErrorBox(ex);
    }
}

void ProgrammerUI::writeMemInFlash(quint8 memId)
{
    if(!m_widget->checkVoltage(true))
        return;

    status("");

    try
    {
        bool restart = !prog()->isInFlashMode();
        chip_definition chip = m_widget->switchToFlashAndGetId();

        writeMem(memId, chip);

        clearHexChanged(memId);

        if(restart)
        {
            log("Switching to run mode");
            prog()->switchToRunMode();
        }

        status(tr("Data has been successfuly written"));
    }
    catch(QString ex)
    {
        m_widget->updateProgressDialog(-1);
        Utils::showErrorBox(ex);
    }
}


void ProgrammerUI::writeMem(quint8 memId, chip_definition &chip)
{
    m_widget->tryFileReload(memId);

    QByteArray data = getHexData(memId);

    if (memId != MEM_JTAG)
    {
        chip_definition::memorydef *memdef = chip.getMemDef(memId);
        if(!memdef)
            throw QString(tr("Unknown memory id"));
        if (memdef->size != 0 && (quint32)data.size() > memdef->size)
            throw QString(tr("Something's wrong, data in tab: %1, chip size: %2")).arg(data.size()).arg(memdef->size);
    }

    log("Writing memory");
    m_widget->showProgressDialog(tr("Writing memory"), prog());

    QDateTime lastMod = m_widget->m_hexFlashTimes[memId];
    if(!m_widget->m_hexFilenames[memId].isEmpty())
    {
        QFileInfo info(m_widget->m_hexFilenames[memId]);

        if(info.exists() && m_widget->m_hexFlashTimes[memId] == info.lastModified())
            warnSecondFlash();
        setFileNeverFlashed(false);

        lastMod = info.lastModified();
    }

    if (memId != MEM_JTAG)
    {
        HexFile file;
        file.setFilePath(m_widget->m_hexFilenames[memId]);
        file.setData(data);

        prog()->flashRaw(file, memId, chip, m_widget->m_verify_mode);
        setHexColor(memId, colorFromDevice);
    }
    else
    {
        prog()->executeText(data, memId, chip);
    }

    m_widget->updateProgressDialog(-1);

    m_widget->m_hexFlashTimes[memId] = lastMod;
}

void ProgrammerUI::eraseDevice()
{
    if(!m_widget->checkVoltage(true))
        return;

    if(!m_widget->showContinueBox(tr("Erase chip?"), tr("Do you really want to erase the WHOLE chip?")))
        return;

    try
    {
        bool restart = !prog()->isInFlashMode();

        chip_definition cd = m_widget->switchToFlashAndGetId();

        log("Erasing device");
        m_widget->showProgressDialog(tr("Erasing chip..."));
        prog()->erase_device(cd);

        if(restart)
        {
            log("Switching to run mode");
            prog()->switchToRunMode();
        }

        m_widget->updateProgressDialog(-1);
    }
    catch(QString ex)
    {
        m_widget->updateProgressDialog(-1);
        Utils::showErrorBox(ex);
        return;
    }

    ToolTipWarn *w = new ToolTipWarn(tr("Chip was succesfuly erased!"), NULL, NULL, 3000, ":/actions/info");
    w->toRightBottom();
}

void ProgrammerUI::updateProgrammerCaps()
{
    if (this->prog())
        m_programmer_caps = this->prog()->capabilities();

    this->programmerCapsChanged();
}

void ProgrammerUI::saveData(DataFileParser *file)
{
    // keep as LorrisShupito because of saved sessions
    file->writeBlockIdentifier("LorrShupitoUItype");
    file->writeVal((int)m_ui_type);
}

void ProgrammerUI::loadData(DataFileParser *)
{

}

void ProgrammerUI::setStartStopBtn(bool start)
{
    if(start)
    {
        startStopBtn()->setIcon(QIcon(":/icons/start"));
        startStopBtn()->setText(tr("Start"));
    }
    else
    {
        startStopBtn()->setIcon(QIcon(":/icons/stop"));
        startStopBtn()->setText(tr("Stop"));
    }
}
