/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QToolBar>
#include <qhexedit.h>

#include "fullshupitoui.h"
#include "../lorrisshupito.h"
#include "fusewidget.h"
#include "../../misc/datafileparser.h"
#include "../modes/shupitomode.h"
#include "../../ui/tooltipwarn.h"
#include "overvccdialog.h"

FullShupitoUI::FullShupitoUI(QObject *parent) :
    ShupitoUI(UI_FULL, parent), ui(new Ui::FullShupitoUI)
{
}

FullShupitoUI::~FullShupitoUI()
{
    sConfig.set(CFG_QUITN32_SHUPITO_TERM_FMT, ui->terminal->getFmt());

    Utils::deleteLayoutMembers(ui->mainLayout);
    delete ui->mainLayout;

    delete ui;
}

void FullShupitoUI::setupUi(LorrisShupito *widget)
{
    ShupitoUI::setupUi(widget);

    ui->setupUi(widget);

    m_read_menu = new QMenu(m_widget);
    m_write_menu = new QMenu(m_widget);

    m_fuse_widget = new FuseWidget(widget);
    ui->mainLayout->addWidget(m_fuse_widget);

    ui->progSpeedBox->setEditText(QString::number(widget->m_prog_speed_hz));

    ui->tunnelCheck->setChecked(sConfig.get(CFG_BOOL_SHUPITO_TUNNEL));

    connect(ui->hideFusesBtn,    SIGNAL(clicked(bool)),            SLOT(hideFusesBtn(bool)));
    connect(ui->settingsBtn,     SIGNAL(clicked(bool)),            SLOT(hideSettingsBtn(bool)));
    connect(ui->hideLogBtn,      SIGNAL(clicked(bool)),            SLOT(hideLogBtn(bool)));
    connect(ui->terminal,        SIGNAL(settingsChanged()),        SLOT(saveTermSettings()));
    connect(ui->flashWarnBox,    SIGNAL(clicked(bool)),            SLOT(flashWarnBox(bool)));
    connect(ui->over_enable,     SIGNAL(toggled(bool)),            SLOT(overvoltageSwitched(bool)));
    connect(ui->over_val,        SIGNAL(valueChanged(double)),     SLOT(overvoltageChanged(double)));
    connect(ui->over_turnoff,    SIGNAL(clicked(bool)),            SLOT(overvoltageTurnOffVcc(bool)));
    connect(m_fuse_widget,       SIGNAL(readFuses()),              SLOT(readFusesInFlash()));
    connect(m_fuse_widget,       SIGNAL(writeFuses()),             SLOT(writeFusesInFlash()));
    connect(ui->eraseButton,     SIGNAL(clicked()),                SLOT(eraseDevice()));
    connect(ui->tunnelSpeedBox,  SIGNAL(editTextChanged(QString)), widget, SLOT(tunnelSpeedChanged(QString)));
    connect(ui->tunnelCheck,     SIGNAL(clicked(bool)),            widget, SLOT(tunnelToggled(bool)));
    connect(ui->progSpeedBox,    SIGNAL(editTextChanged(QString)), widget, SLOT(progSpeedChanged(QString)));
    connect(ui->startStopBtn,    SIGNAL(clicked()),                widget, SLOT(startstopChip()));
    connect(ui->fmtBox,          SIGNAL(activated(int)),   ui->terminal, SLOT(setFmt(int)));
    connect(ui->terminal,        SIGNAL(fmtSelected(int)), ui->fmtBox,   SLOT(setCurrentIndex(int)));
    connect(ui->terminal,        SIGNAL(paused(bool)),     ui->pauseBtn, SLOT(setChecked(bool)));
    connect(ui->clearBtn,        SIGNAL(clicked()),        ui->terminal, SLOT(clear()));
    connect(ui->pauseBtn,        SIGNAL(clicked(bool)),    ui->terminal, SLOT(pause(bool)));
    connect(m_fuse_widget,       SIGNAL(status(QString)),          widget, SLOT(status(QString)));

    const QWidget *buttons[] = { ui->readButton, ui->writeButton, ui->eraseButton, ui->startStopBtn, NULL };
    for(const QWidget **itr = buttons; *itr; ++itr)
        connect(this, SIGNAL(enableButtons(bool)), *itr, SLOT(setEnabled(bool)));

    emit enableButtons(widget->m_buttons_enabled);

    int w = ui->hideFusesBtn->fontMetrics().height()+10;
    ui->hideLogBtn->setFixedHeight(w);
    ui->hideFusesBtn->setFixedWidth(w);
    ui->hideFusesBtn->setRotation(ROTATE_90);

    hideLogBtn(sConfig.get(CFG_BOOL_SHUPITO_SHOW_LOG));
    hideFusesBtn(sConfig.get(CFG_BOOL_SHUPITO_SHOW_FUSES));
    hideSettingsBtn(sConfig.get(CFG_BOOL_SHUPITO_SHOW_SETTINGS));

    ui->tunnelSpeedBox->setValidator(new QIntValidator(1, INT_MAX, this));
    ui->progSpeedBox->setValidator(new QIntValidator(1, INT_MAX, this));

    initMenus();

    ui->terminal->setFmt(sConfig.get(CFG_QUITN32_SHUPITO_TERM_FMT));
    ui->terminal->loadSettings(sConfig.get(CFG_STRING_SHUPITO_TERM_SET));

    QByteArray data = QByteArray(1024, (char)0xFF);
    static const QString memNames[] = { tr("Program memory"), tr("EEPROM") };
    m_hexAreas[0] = NULL;
    for(quint8 i = 1; i < TAB_MAX; ++i)
    {
        QHexEdit *h = new QHexEdit(widget);
        h->setData(data);
        m_hexAreas[i] = h;
        ui->memTabs->addTab(h, memNames[i-1]);
    }

    ui->over_enable->setChecked(sConfig.get(CFG_BOOL_SHUPITO_OVERVOLTAGE));
    ui->over_val->setValue(sConfig.get(CFG_FLOAT_SHUPITO_OVERVOLTAGE_VAL));
    ui->over_turnoff->setChecked(sConfig.get(CFG_BOOL_SHUPITO_TURNOFF_VCC));

    ui->flashWarnBox->setChecked(sConfig.get(CFG_BOOL_SHUPITO_SHOW_FLASH_WARN));

    widget->createConnBtn(ui->connectButton);
}

void FullShupitoUI::createActions()
{
    {
        QAction * Flash  = m_read_menu->addAction(tr("Read flash"));
        QAction * All    = m_read_menu->addAction(tr("Read all"));
        QAction * EEPROM = m_read_menu->addAction(tr("Read EEPROM"));
        QAction * Fuses  = m_read_menu->addAction(tr("Read fuses"));

        m_font = All->font();

        connect(All,      SIGNAL(triggered()), this, SLOT(readAll()));
        connect(Flash,    SIGNAL(triggered()), this, SLOT(readMemButton()));
        connect(EEPROM,   SIGNAL(triggered()), this, SLOT(readEEPROMBtn()));
        connect(Fuses,    SIGNAL(triggered()), this, SLOT(readFusesInFlash()));

        m_read_menu->insertSeparator(All);

        m_read_actions[ACT_ALL]    = All;
        m_read_actions[ACT_FLASH]  = Flash;
        m_read_actions[ACT_EEPROM] = EEPROM;
        m_read_actions[ACT_FUSES]  = Fuses;

        ui->readButton->setMenu(m_read_menu);
    }

    {
        QAction * Flash  = m_write_menu->addAction(tr("Write flash"));
        QAction * All    = m_write_menu->addAction(tr("Write all"));
        QAction * EEPROM = m_write_menu->addAction(tr("Write EEPROM"));
        QAction * Fuses  = m_write_menu->addAction(tr("Write fuses"));

        connect(All,      SIGNAL(triggered()), this, SLOT(writeAll()));
        connect(Flash,    SIGNAL(triggered()), this, SLOT(writeFlashBtn()));
        connect(EEPROM,   SIGNAL(triggered()), this, SLOT(writeEEPROMBtn()));
        connect(Fuses,    SIGNAL(triggered()), this, SLOT(writeFusesInFlash()));

        m_write_menu->insertSeparator(All);

        m_write_actions[ACT_ALL]    = All;
        m_write_actions[ACT_FLASH]  = Flash;
        m_write_actions[ACT_EEPROM] = EEPROM;
        m_write_actions[ACT_FUSES]  = Fuses;

        ui->writeButton->setMenu(m_write_menu);
    }

    m_boldFont = m_font;
    m_boldFont.setBold(true);
}

void FullShupitoUI::setActiveAction(int actInt)
{
    ActionSlots act = actInt == TAB_EEPROM ? ACT_EEPROM : ACT_FLASH;

    if(act == m_active)
        return;

    if(m_active != ACT_ALL)
    {
        m_read_menu->insertAction(m_read_actions[m_active], m_read_actions[act]);
        m_read_menu->insertAction(m_read_actions[ACT_FUSES], m_read_actions[m_active]);
        m_write_menu->insertAction(m_write_actions[m_active], m_write_actions[act]);
        m_write_menu->insertAction(m_write_actions[ACT_FUSES], m_write_actions[m_active]);
    }

    m_read_actions[m_active]->setFont(m_font);
    m_read_actions[act]->setFont(m_boldFont);
    m_write_actions[m_active]->setFont(m_font);
    m_write_actions[act]->setFont(m_boldFont);

    m_active = act;
}

void FullShupitoUI::readButtonClicked()
{
    if (m_active == ACT_FLASH)
        readMemButton();
    else
        readEEPROMBtn();
}

void FullShupitoUI::writeButtonClicked()
{
    if (m_active == ACT_FLASH)
        writeFlashBtn();
    else
        writeEEPROMBtn();
}

void FullShupitoUI::initMenus()
{
    // Flash/Read buttons
    m_active = ACT_ALL;
    this->createActions();

    connect(ui->memTabs, SIGNAL(currentChanged(int)), this, SLOT(setActiveAction(int)));
    this->setActiveAction(0);

    connect(ui->readButton, SIGNAL(clicked()), this, SLOT(readButtonClicked()));
    connect(ui->writeButton, SIGNAL(clicked()), this, SLOT(writeButtonClicked()));

    // toolbar
    QToolBar *bar = new QToolBar(m_widget);
    ui->topLayout->insertWidget(1, bar);
    bar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    bar->setIconSize(QSize(16, 16));

    bar->addAction(m_widget->m_load_flash);
    bar->addAction(m_widget->m_save_flash);
    bar->addSeparator();

    QPushButton *btn = new QPushButton(QIcon(":/actions/wire"), tr("Mode"), m_widget);
    btn->setFlat(true);
    btn->setMenu(m_widget->m_modeBar);
    bar->addWidget(btn);
}

void FullShupitoUI::connectProgrammer(Programmer * prog)
{
    connect(ui->terminal, SIGNAL(keyPressed(QString)),   prog,      SLOT(sendTunnelData(QString)));
    connect(prog,  SIGNAL(tunnelData(QByteArray)),    ui->terminal, SLOT(appendText(QByteArray)));
    m_widget->m_programmer->setTunnelSpeed(ui->tunnelSpeedBox->itemText(0).toInt(), false);
}

void FullShupitoUI::hideLogBtn(bool checked)
{
    ui->hideLogBtn->setChecked(checked);
    ui->logText->setVisible(checked);
    sConfig.set(CFG_BOOL_SHUPITO_SHOW_LOG, checked);
}

void FullShupitoUI::hideFusesBtn(bool checked)
{
    ui->hideFusesBtn->setChecked(checked);
    m_fuse_widget->setVisible(checked);
    sConfig.set(CFG_BOOL_SHUPITO_SHOW_FUSES, checked);
}

void FullShupitoUI::hideSettingsBtn(bool checked)
{
    ui->progBox->setVisible(checked);
    ui->tunnelBox->setVisible(checked);
    ui->overvccBox->setVisible(checked);
    ui->settingsBtn->setChecked(checked);
    sConfig.set(CFG_BOOL_SHUPITO_SHOW_SETTINGS, checked);
}

void FullShupitoUI::saveTermSettings()
{
    sConfig.set(CFG_STRING_SHUPITO_TERM_SET, ui->terminal->getSettingsData());
}

void FullShupitoUI::connectedStatus(bool connected)
{
    ShupitoUI::connectedStatus(connected);

    ui->tunnelCheck->setEnabled(connected);
    ui->tunnelSpeedBox->setEnabled(connected);
    ui->progSpeedBox->setEnabled(connected);
}

void FullShupitoUI::tunnelStop(bool stop)
{
    ShupitoUI::tunnelStop(stop);

    if(stop)
    {
        ui->tunnelCheck->setEnabled(false);
        ui->tunnelCheck->setChecked(false);
    }
}

void FullShupitoUI::setTunnelActive(bool active)
{
    ui->tunnelCheck->setChecked(active);
}

void FullShupitoUI::tunnelStateChanged(bool opened)
{
    if(opened)
    {
        ui->tunnelCheck->setEnabled(true);
    }
    else if(ui->tunnelCheck->isChecked())
    {
        ui->tunnelCheck->setEnabled(false);
    }

    ui->tunnelCheck->setChecked(opened);
}

void FullShupitoUI::log(const QString &text)
{
    ui->logText->appendPlainText(text);
}

void FullShupitoUI::postFlashSwitchCheck(chip_definition &chip)
{
    if(m_fuse_widget->getChipDef().getSign() != chip.getSign())
        m_fuse_widget->clear(true);
}

void FullShupitoUI::setHexData(quint32 memid, const QByteArray &data)
{
    m_hexAreas[memid]->setData(data);
}

void FullShupitoUI::setHexColor(quint32 memid, const QString &clr)
{
    m_hexAreas[memid]->setBackgroundColor(clr);
}

QByteArray FullShupitoUI::getHexData(quint32 memid) const
{
    return m_hexAreas[memid]->data();
}

void FullShupitoUI::clearHexChanged(quint32 memid)
{
    m_hexAreas[memid]->clearDataChanged();
}

bool FullShupitoUI::hasHexChanged(quint32 memid)
{
    return m_hexAreas[memid]->hasDataChanged();
}

void FullShupitoUI::setActiveMem(quint32 memId)
{
    ui->memTabs->setCurrentIndex(memId);
}

void FullShupitoUI::readFusesInFlash()
{
    if(!m_widget->checkVoltage(true))
        return;

    status("");

    try
    {
        bool restart = !prog()->isInFlashMode();
        chip_definition chip = m_widget->switchToFlashAndGetId();

        readFuses(chip);

        if(restart)
        {
            log("Switching to run mode");
            prog()->switchToRunMode();
        }
        status(tr("Fuses had been succesfully read"));
    }
    catch(QString ex)
    {
        Utils::showErrorBox(ex);
    }
}

void FullShupitoUI::readFuses(chip_definition& chip)
{
    log("Reading fuses");

    m_fuse_widget->clear(false);

    std::vector<quint8>& data = m_fuse_widget->getFuseData();
    data.clear();

    prog()->readFuses(data, chip);
    m_fuse_widget->setFuses(chip);

    hideFusesBtn(true);
}


void FullShupitoUI::writeFusesInFlash()
{
    if(!m_widget->checkVoltage(true))
        return;

    status("");

    if(!m_fuse_widget->isLoaded())
    {
        Utils::showErrorBox(tr("Fuses had not been read yet"));
        return;
    }

    if(m_fuse_widget->isChanged())
    {
        Utils::showErrorBox(tr("You have to \"Remember\" fuses prior to writing"));
        return;
    }

    if(!m_widget->showContinueBox(tr("Write fuses?"), tr("Do you really wanna to write fuses to the chip?")))
        return;

    try
    {
        bool restart = !prog()->isInFlashMode();
        chip_definition chip = m_widget->switchToFlashAndGetId();

        writeFuses(chip);

        if(restart)
        {
            log("Switching to run mode");
            prog()->switchToRunMode();
        }
        status(tr("Fuses had been succesfully written"));
    }
    catch(QString ex)
    {
        Utils::showErrorBox(ex);
    }
}

void FullShupitoUI::writeFuses(chip_definition &chip)
{
    if(!m_fuse_widget->isLoaded())
        throw QString(tr("Fuses had not been read yet"));

    log("Writing fuses");
    std::vector<quint8>& data = m_fuse_widget->getFuseData();
    prog()->writeFuses(data, chip, m_widget->m_verify_mode);
}

void FullShupitoUI::writeSelectedMem()
{
    this->writeButtonClicked();
}

void FullShupitoUI::warnSecondFlash()
{
    if(ui->flashWarnBox->isChecked())
    {
        new ToolTipWarn(tr("You have flashed this file already, and it was not changed since."), ui->writeButton, m_widget);
        Utils::playErrorSound();
    }
}

int FullShupitoUI::getMemIndex()
{
    switch(ui->memTabs->currentIndex())
    {
        default:
        case TAB_TERMINAL:
        case TAB_FLASH:
            return MEM_FLASH;
        case TAB_EEPROM:
            return MEM_EEPROM;
    }
}

void FullShupitoUI::overvoltageSwitched(bool enabled)
{
    sConfig.set(CFG_BOOL_SHUPITO_OVERVOLTAGE, enabled);
    m_widget->m_enable_overvcc = enabled;

    if(!enabled && m_widget->m_overvcc_dialog)
        delete m_widget->m_overvcc_dialog;
}

void FullShupitoUI::overvoltageChanged(double val)
{
    sConfig.set(CFG_FLOAT_SHUPITO_OVERVOLTAGE_VAL, val);
    m_widget->m_overvcc = val;

    disableOvervoltVDDs();
}

void FullShupitoUI::overvoltageTurnOffVcc(bool enabled)
{
    sConfig.set(CFG_BOOL_SHUPITO_TURNOFF_VCC, enabled);
    m_widget->m_overvcc_turnoff = enabled;
}

void FullShupitoUI::flashWarnBox(bool checked)
{
    ui->flashWarnBox->setChecked(checked);
    sConfig.set(CFG_BOOL_SHUPITO_SHOW_FLASH_WARN, checked);
}

void FullShupitoUI::saveData(DataFileParser *file)
{
    ShupitoUI::saveData(file);

    file->writeBlockIdentifier("LorrShupitoTermSett");
    file->writeString(ui->terminal->getSettingsData());
    file->writeVal(ui->terminal->getFmt());

    file->writeBlockIdentifier("LorrShupitoTermData");
    {
        QByteArray data = ui->terminal->getData();
        file->writeVal(data.size());
        file->write(data);
    }

    file->writeBlockIdentifier("LorrShupitoSett");
    {
        file->writeVal(ui->memTabs->currentIndex());

        file->writeVal(ui->hideLogBtn->isChecked());
        file->writeVal(ui->hideFusesBtn->isChecked());
        file->writeVal(ui->settingsBtn->isChecked());
    }

    file->writeBlockIdentifier("LorrShupitoProgSett");
    file->writeVal(m_widget->m_prog_speed_hz);
    file->writeVal(ui->flashWarnBox->isChecked());

    file->writeBlockIdentifier("LorrShupitoTunnel");
    file->writeVal(ui->tunnelCheck->isChecked());
    file->writeVal(ui->tunnelSpeedBox->itemText(0).toInt());

    file->writeBlockIdentifier("LorrShupitoOvervcc");
    file->writeVal(ui->over_enable->isChecked());
    file->writeVal(ui->over_turnoff->isChecked());
    file->writeVal(ui->over_val->value());
}

void FullShupitoUI::loadData(DataFileParser *file)
{
    ShupitoUI::loadData(file);

    if(file->seekToNextBlock("LorrShupitoTermSett", BLOCK_WORKTAB))
    {
        ui->terminal->loadSettings(file->readString());
        ui->terminal->setFmt(file->readVal<int>());
    }

    if(file->seekToNextBlock("LorrShupitoTermData", BLOCK_WORKTAB))
    {
        int size = file->readVal<int>();
        QByteArray data = file->read(size);
        ui->terminal->appendText(data);
    }

    if(file->seekToNextBlock("LorrShupitoSett", BLOCK_WORKTAB))
    {
        ui->memTabs->setCurrentIndex(file->readVal<int>());

        hideLogBtn(file->readVal<bool>());
        hideFusesBtn(file->readVal<bool>());
        hideSettingsBtn(file->readVal<bool>());
    }

    if(file->seekToNextBlock("LorrShupitoProgSett", BLOCK_WORKTAB))
    {
        ui->progSpeedBox->setEditText(QString::number(file->readVal<quint32>()));
        ui->flashWarnBox->setChecked(file->readVal<bool>());
    }

    if(file->seekToNextBlock("LorrShupitoTunnel", BLOCK_WORKTAB))
    {
        ui->tunnelCheck->setChecked(file->readVal<bool>());
        ui->tunnelSpeedBox->setEditText(QString::number(file->readVal<quint32>()));
    }

    if(file->seekToNextBlock("LorrShupitoOvervcc", BLOCK_WORKTAB))
    {
        ui->over_enable->setChecked(file->readVal<bool>());
        ui->over_turnoff->setChecked(file->readVal<bool>());
        ui->over_val->setValue(file->readVal<double>());
    }
}

void FullShupitoUI::setFileAndTime(const QString &file, const QDateTime &time)
{
    ui->filename->setText(file);
    ui->filename->setToolTip(file);

    QString str = time.toString(tr(" | h:mm:ss M.d.yyyy"));
    ui->filedate->setText(str);
    ui->filedate->setToolTip(str);
}

void FullShupitoUI::setChipId(const QString &text)
{
    ui->chipIdLabel->setText(text);
    ui->chipIdLabel->setToolTip(text);
}

