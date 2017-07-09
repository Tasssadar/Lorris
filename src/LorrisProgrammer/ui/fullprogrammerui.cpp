/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QToolBar>
#include <qhexedit.h>

#include "fullprogrammerui.h"
#include "../lorrisprogrammer.h"
#include "fusewidget.h"
#include "../../misc/datafileparser.h"
#include "../modes/shupitomode.h"
#include "../../ui/tooltipwarn.h"
#include "overvccdialog.h"
#include "../../ui/bytevalidator.h"
#include "../../ui/floatinginputdialog.h"

FullProgrammerUI::FullProgrammerUI(QObject *parent) :
    ProgrammerUI(UI_FULL, parent), ui(new Ui::FullProgrammerUI)
{
}

FullProgrammerUI::~FullProgrammerUI()
{
    sConfig.set(CFG_QUITN32_SHUPITO_TERM_FMT, ui->terminal->getFmt());

    Utils::deleteLayoutMembers(ui->mainLayout);
    delete ui->mainLayout;

    delete ui;
}

void FullProgrammerUI::setupUi(LorrisProgrammer *widget)
{
    ProgrammerUI::setupUi(widget);

    ui->setupUi(widget);

    QByteArray data = QByteArray(1024, (char)0xFF);
    m_hexAreas[0] = NULL;
    for(quint8 i = 1; i < 3; ++i)
    {
        QHexEdit *h = new QHexEdit(widget);
        h->setData(data);
        m_hexAreas[i] = h;

        h->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(h, SIGNAL(customContextMenuRequested(QPoint)), SLOT(hexEditMenuReq(QPoint)));
    }

    m_svfEdit = new QTextEdit(widget);
    m_svfEdit->setReadOnly(true);
    m_svfEdit->setVisible(false);

    m_programmer_caps.flash = true;
    m_programmer_caps.eeprom = true;

    m_read_menu = new QMenu(m_widget);
    m_write_menu = new QMenu(m_widget);

    m_fuse_widget = new FuseWidget();
    ui->fuseContainer->layout()->addWidget(m_fuse_widget);

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
    connect(ui->pwmRadioGroup,   SIGNAL(buttonClicked(int)),       SLOT(pwmRadioClicked(int)));
    connect(ui->pwmFreqSpin,     SIGNAL(valueChanged(int)),        SLOT(setPwmGeneric()));
    connect(ui->dutySpin,        SIGNAL(valueChanged(int)),        SLOT(setPwmGeneric()));
    connect(ui->maxServoPeriodBox, SIGNAL(valueChanged(double)),   SLOT(setPwmServo()));
    connect(ui->minServoPeriodBox, SIGNAL(valueChanged(double)),   SLOT(setPwmServo()));
    connect(ui->servoPosSlider,  SIGNAL(valueChanged(int)),        ui->servoPosSpin, SLOT(setValue(int)));
    connect(ui->servoPosSpin,    SIGNAL(valueChanged(int)),        SLOT(setPwmServo()));
    connect(ui->servoPosSpin,    SIGNAL(valueChanged(int)),        ui->servoPosSlider, SLOT(setValue(int)));
    connect(ui->progButton,      SIGNAL(clicked(bool)),            SLOT(progButtonClicked()));

    ui->pwmRadioGroup->setId(ui->disablePwmRadio, 0);
    ui->pwmRadioGroup->setId(ui->genericPwmRadio, 1);
    ui->pwmRadioGroup->setId(ui->servoPwmRadio, 2);

    this->enableButtons(widget->m_buttons_enabled);

    int w = ui->hideFusesBtn->fontMetrics().height()+10;
    ui->hideLogBtn->setFixedHeight(w);
    ui->hideFusesBtn->setFixedWidth(w);
    ui->hideFusesBtn->setRotation(ROTATE_90);

    initMenus();

    hideLogBtn(sConfig.get(CFG_BOOL_SHUPITO_SHOW_LOG));
    hideFusesBtn(sConfig.get(CFG_BOOL_SHUPITO_SHOW_FUSES));
    hideSettingsBtn(sConfig.get(CFG_BOOL_SHUPITO_SHOW_SETTINGS));

    ui->tunnelSpeedBox->setValidator(new QIntValidator(1, INT_MAX, this));
    ui->progSpeedBox->setValidator(new QIntValidator(1, INT_MAX, this));
    ui->bootseqEdit->setValidator(new ByteValidator(this));

    ui->terminal->setFmt(sConfig.get(CFG_QUITN32_SHUPITO_TERM_FMT));
    ui->terminal->loadSettings(sConfig.get(CFG_STRING_SHUPITO_TERM_SET));

    ui->over_enable->setChecked(sConfig.get(CFG_BOOL_SHUPITO_OVERVOLTAGE));
    ui->over_val->setValue(sConfig.get(CFG_FLOAT_SHUPITO_OVERVOLTAGE_VAL));
    ui->over_turnoff->setChecked(sConfig.get(CFG_BOOL_SHUPITO_TURNOFF_VCC));

    ui->flashWarnBox->setChecked(sConfig.get(CFG_BOOL_SHUPITO_SHOW_FLASH_WARN));

    this->applySources();
    widget->createConnBtn(ui->connectButton);
}

void FullProgrammerUI::enableButtons(bool enable)
{
    ui->readButton->setEnabled(enable && (m_programmer_caps.flash || m_programmer_caps.eeprom || m_programmer_caps.fuses));
    ui->writeButton->setEnabled(enable);
    ui->eraseButton->setEnabled(enable && m_programmer_caps.supports_erase());
    ui->startStopBtn->setEnabled(enable);
    m_fuse_widget->enableButtons(enable);
}

void FullProgrammerUI::createActions()
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

void FullProgrammerUI::setActiveAction(int)
{
    if (this->currentTab() == tab_svf)
    {
        ui->writeButton->setMenu(0);
        ui->writeButton->setPopupMode(QToolButton::DelayedPopup);
    }
    else
    {
        ui->writeButton->setMenu(m_write_menu);
        ui->writeButton->setPopupMode(QToolButton::MenuButtonPopup);

        ActionSlots act = this->currentTab() == tab_eeprom? ACT_EEPROM: ACT_FLASH;
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
}

void FullProgrammerUI::readButtonClicked()
{
    if (m_active == ACT_FLASH)
        readMemButton();
    else
        readEEPROMBtn();
}

void FullProgrammerUI::writeButtonClicked()
{
    if (this->currentTab() == tab_svf)
    {
        writeMemInFlash(MEM_JTAG);
    }
    else
    {
        if (m_active == ACT_FLASH)
            writeMemInFlash(MEM_FLASH);
        else
            writeEEPROMBtn();
    }
}

void FullProgrammerUI::initMenus()
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
    ui->topLayout->insertWidget(2, bar);
    bar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    bar->setIconSize(QSize(16, 16));

    bar->addSeparator();
    bar->addAction(m_widget->m_load_flash);
    bar->addAction(m_widget->m_save_flash);
    bar->addSeparator();

    QPushButton *btn = new QPushButton(QIcon(":/actions/wire"), tr("Mode"), m_widget);
    btn->setFlat(true);
    btn->setMenu(m_widget->m_modeBar);
    bar->addWidget(btn);
}

void FullProgrammerUI::connectProgrammer(Programmer * prog)
{
    ProgrammerUI::connectProgrammer(prog);

    connect(ui->terminal,    SIGNAL(keyPressed(QString)),    prog,         SLOT(sendTunnelData(QString)));
    connect(ui->bootseqEdit, SIGNAL(textEdited(QString)),    prog,         SLOT(setBootseq(QString)));
    connect(prog,            SIGNAL(tunnelData(QByteArray)), ui->terminal, SLOT(appendText(QByteArray)));
    connect(prog,            SIGNAL(pwmChanged(uint32_t,float)), this,     SLOT(pwmChanged(uint32_t, float)));

    m_widget->m_programmer->setTunnelSpeed(ui->tunnelSpeedBox->currentText().toInt(), false);
    ui->bootseqEdit->setText(prog->getBootseq());

    updateProgrammersBox(prog);
    this->programmerCapsChanged();
}

void FullProgrammerUI::updateProgrammersBox(Programmer *prog)
{
    // corresponds to enum ProgrammerTypes
    static const QString names[] = { "Shupito", "Flip", "avr232boot", "atsam", "avr109", "STM32 STLink", "Arduino", "Zmodem" };
    static const QString icons[] = {
        ":/icons/symbol_triangle",
        ":/icons/symbol_circle",
        ":/icons/symbol_star",
        ":/icons/symbol_cross",
        ":/icons/symbol_moon",
        ":/icons/symbol_circle",
        ":/icons/arduino",
        ":/icons/zmodem"
    };

    Q_ASSERT(sizeof_array(names) == programmer_max);
    Q_ASSERT(sizeof_array(icons) == programmer_max);

    ui->progButton->setText(names[prog->getType()]);
    ui->progButton->setIcon(QIcon(icons[prog->getType()]).pixmap(16, 16));
}

void FullProgrammerUI::hideLogBtn(bool checked)
{
    ui->hideLogBtn->setChecked(checked);
    ui->logText->setVisible(checked);
    sConfig.set(CFG_BOOL_SHUPITO_SHOW_LOG, checked);
}

void FullProgrammerUI::hideFusesBtn(bool checked)
{
    ui->hideFusesBtn->setChecked(checked);
    m_fuse_widget->setVisible(checked);
    sConfig.set(CFG_BOOL_SHUPITO_SHOW_FUSES, checked);
}

void FullProgrammerUI::hideSettingsBtn(bool checked)
{
    ui->progBox->setVisible(checked);
    ui->overvccBox->setVisible(checked);
    ui->settingsBtn->setChecked(checked);
    sConfig.set(CFG_BOOL_SHUPITO_SHOW_SETTINGS, checked);
    this->programmerCapsChanged();
}

void FullProgrammerUI::saveTermSettings()
{
    sConfig.set(CFG_STRING_SHUPITO_TERM_SET, ui->terminal->getSettingsData());
}

void FullProgrammerUI::connectedStatus(bool connected)
{
    ProgrammerUI::connectedStatus(connected);

    ui->tunnelCheck->setEnabled(connected);
    ui->tunnelSpeedBox->setEnabled(connected);
    ui->progSpeedBox->setEnabled(connected);
    this->programmerCapsChanged();
}

void FullProgrammerUI::applySources()
{
    ui->fuseContainer->setVisible(m_programmer_caps.fuses);

    tabs_t tab = this->currentTab();

    bool prevBlock = ui->memTabs->blockSignals(true);

    QVariant prop;
    while(ui->memTabs->count())
    {
        prop = ui->memTabs->widget(0)->property(PROG_WIDGET_PROPERTY);
        if(prop.type() == QVariant::Bool && prop.toBool() == true)
            delete ui->memTabs->widget(0);
        else
            ui->memTabs->removeTab(0);
    }

    if (m_programmer_caps.widgets)
    {
        QList<QWidget*> widgets = prog()->widgets();
        Q_ASSERT(widgets.size() == m_programmer_caps.widgets);

        for(int i = 0; i < widgets.size(); ++i)
        {
            widgets[i]->setParent(ui->memTabs);
            widgets[i]->setProperty(PROG_WIDGET_PROPERTY, QVariant(true));
            ui->memTabs->addTab(widgets[i], widgets[i]->windowTitle());
        }
    }

    if (m_programmer_caps.terminal)
        ui->memTabs->addTab(ui->terminal, tr("Terminal"));
    if (m_programmer_caps.flash)
        ui->memTabs->addTab(m_hexAreas[MEM_FLASH], tr("Program memory"));
    if (m_programmer_caps.eeprom)
        ui->memTabs->addTab(m_hexAreas[MEM_EEPROM], tr("EEPROM"));
    if (m_programmer_caps.svf)
        ui->memTabs->addTab(m_svfEdit, tr("SVF"));

    ui->memTabs->blockSignals(prevBlock);

    this->setCurrentTab(tab);
}

void FullProgrammerUI::programmerCapsChanged()
{
    if (this->prog())
        this->applySources();

    ui->pwmBox->setVisible(ui->settingsBtn->isChecked() && this->prog() && this->prog()->supportsPwm());
    ui->tunnelBox->setVisible(ui->settingsBtn->isChecked() && this->prog() && this->prog()->supportsTunnel());

    bool bootseq = ui->settingsBtn->isChecked() && this->prog() && this->prog()->supportsBootseq();
    ui->bootseqLabel->setVisible(bootseq);
    ui->bootseqEdit->setVisible(bootseq);
    this->enableButtons(m_widget->m_buttons_enabled);
}

void FullProgrammerUI::tunnelStop(bool stop)
{
    ProgrammerUI::tunnelStop(stop);

    if(stop)
    {
        ui->tunnelCheck->setEnabled(false);
        ui->tunnelCheck->setChecked(false);
    }
}

void FullProgrammerUI::setTunnelActive(bool active)
{
    ui->tunnelCheck->setChecked(active);
}

void FullProgrammerUI::tunnelStateChanged(bool opened)
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

void FullProgrammerUI::log(const QString &text)
{
    ui->logText->appendPlainText(text);
}

void FullProgrammerUI::postFlashSwitchCheck(chip_definition &chip)
{
    if(m_fuse_widget->getChipDef().getSign() != chip.getSign())
        m_fuse_widget->clear(true);
}

void FullProgrammerUI::setHexData(quint32 memid, const QByteArray &data)
{
    if (memid == MEM_JTAG)
        m_svfEdit->setText(QString::fromUtf8(data.data(), data.size()));
    else
        m_hexAreas[memid]->setData(data);
}

void FullProgrammerUI::setHexColor(quint32 memid, const QString &clr)
{
    if (memid != MEM_JTAG)
        m_hexAreas[memid]->setBackgroundColor(clr);
}

QByteArray FullProgrammerUI::getHexData(quint32 memid) const
{
    if (memid == MEM_JTAG)
        return m_svfEdit->toPlainText().toUtf8();
    else
        return m_hexAreas[memid]->data();
}

void FullProgrammerUI::clearHexChanged(quint32 memid)
{
    if (memid != MEM_JTAG)
        m_hexAreas[memid]->clearDataChanged();
}

bool FullProgrammerUI::hasHexChanged(quint32 memid)
{
    if (memid == MEM_JTAG)
        return false;
    return m_hexAreas[memid]->hasDataChanged();
}

void FullProgrammerUI::setActiveMem(quint32 memId)
{
    switch (memId)
    {
    case MEM_FLASH:
        this->setCurrentTab(tab_flash);
        break;
    case MEM_EEPROM:
        this->setCurrentTab(tab_eeprom);
        break;
    case MEM_JTAG:
        this->setCurrentTab(tab_svf);
        break;
    }
}

FullProgrammerUI::tabs_t FullProgrammerUI::currentTab() const
{
    int idx = ui->memTabs->currentIndex();
    if (m_programmer_caps.widgets > 0 && idx < m_programmer_caps.widgets)
        return tab_widgets;
    if (m_programmer_caps.terminal && idx == 0)
        return tab_terminal;
    if (m_programmer_caps.flash && idx == (int)m_programmer_caps.terminal + m_programmer_caps.widgets)
        return tab_flash;
    if (m_programmer_caps.eeprom && idx == m_programmer_caps.terminal + m_programmer_caps.flash + m_programmer_caps.widgets)
        return tab_eeprom;
    if (m_programmer_caps.svf && idx == m_programmer_caps.terminal + m_programmer_caps.flash + m_programmer_caps.eeprom + m_programmer_caps.widgets)
        return tab_svf;

    Q_ASSERT(0);
    return tab_terminal;
}

void FullProgrammerUI::setCurrentTab(tabs_t t)
{
    switch (t)
    {
    case tab_widgets:
        if (m_programmer_caps.widgets)
            ui->memTabs->setCurrentIndex(0);
    case tab_terminal:
        if (m_programmer_caps.terminal)
            ui->memTabs->setCurrentIndex(m_programmer_caps.widgets);
        break;
    case tab_flash:
        if (m_programmer_caps.flash)
            ui->memTabs->setCurrentIndex(m_programmer_caps.widgets + m_programmer_caps.terminal);
        break;
    case tab_eeprom:
        if (m_programmer_caps.eeprom)
            ui->memTabs->setCurrentIndex(m_programmer_caps.widgets + m_programmer_caps.terminal + m_programmer_caps.flash);
        break;
    case tab_svf:
        if (m_programmer_caps.svf)
            ui->memTabs->setCurrentIndex(m_programmer_caps.widgets + m_programmer_caps.terminal + m_programmer_caps.flash + m_programmer_caps.eeprom);
        break;
    }

    this->setActiveAction(0);
}

void FullProgrammerUI::readFusesInFlash()
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

void FullProgrammerUI::readFuses(chip_definition& chip)
{
    log("Reading fuses");

    m_fuse_widget->clear(false);

    std::vector<quint8>& data = m_fuse_widget->getFuseData();
    data.clear();

    prog()->readFuses(data, chip);
    m_fuse_widget->setFuses(chip);

    hideFusesBtn(true);
}


void FullProgrammerUI::writeFusesInFlash()
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

void FullProgrammerUI::writeFuses(chip_definition &chip)
{
    if(!m_fuse_widget->isLoaded())
        throw QString(tr("Fuses had not been read yet"));

    log("Writing fuses");
    std::vector<quint8>& data = m_fuse_widget->getFuseData();
    prog()->writeFuses(data, chip, m_widget->m_verify_mode);
}

void FullProgrammerUI::writeSelectedMem()
{
    this->writeButtonClicked();
}

void FullProgrammerUI::warnSecondFlash()
{
    if(ui->flashWarnBox->isChecked())
    {
        new ToolTipWarn(tr("You have flashed this file already, and it was not changed since."), ui->writeButton, m_widget);
        Utils::playErrorSound();
    }
}

int FullProgrammerUI::getMemIndex()
{
    switch (this->currentTab())
    {
    case tab_terminal:
        return m_programmer_caps.flash? MEM_FLASH: MEM_JTAG;
    case tab_flash:
        return MEM_FLASH;
    case tab_eeprom:
        return MEM_EEPROM;
    case tab_svf:
        return MEM_JTAG;
    }

    return MEM_NONE;
}

void FullProgrammerUI::overvoltageSwitched(bool enabled)
{
    sConfig.set(CFG_BOOL_SHUPITO_OVERVOLTAGE, enabled);
    m_widget->m_enable_overvcc = enabled;

    if(!enabled && m_widget->m_overvcc_dialog)
        delete m_widget->m_overvcc_dialog;
}

void FullProgrammerUI::overvoltageChanged(double val)
{
    sConfig.set(CFG_FLOAT_SHUPITO_OVERVOLTAGE_VAL, val);
    m_widget->m_overvcc = val;

    disableOvervoltVDDs();
}

void FullProgrammerUI::overvoltageTurnOffVcc(bool enabled)
{
    sConfig.set(CFG_BOOL_SHUPITO_TURNOFF_VCC, enabled);
    m_widget->m_overvcc_turnoff = enabled;
}

void FullProgrammerUI::flashWarnBox(bool checked)
{
    ui->flashWarnBox->setChecked(checked);
    sConfig.set(CFG_BOOL_SHUPITO_SHOW_FLASH_WARN, checked);
}

void FullProgrammerUI::saveData(DataFileParser *file)
{
    ProgrammerUI::saveData(file);

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
        file->writeVal((int)this->currentTab());
        file->writeVal(ui->hideLogBtn->isChecked());
        file->writeVal(ui->hideFusesBtn->isChecked());
        file->writeVal(ui->settingsBtn->isChecked());
    }

    file->writeBlockIdentifier("LorrShupitoProgSett");
    file->writeVal(m_widget->m_prog_speed_hz);
    file->writeVal(ui->flashWarnBox->isChecked());

    file->writeBlockIdentifier("LorrShupitoTunnel");
    file->writeVal(ui->tunnelCheck->isChecked());
    file->writeVal(ui->tunnelSpeedBox->currentText().toInt());

    file->writeBlockIdentifier("LorrShupitoOvervcc");
    file->writeVal(ui->over_enable->isChecked());
    file->writeVal(ui->over_turnoff->isChecked());
    file->writeVal(ui->over_val->value());
}

void FullProgrammerUI::loadData(DataFileParser *file)
{
    ProgrammerUI::loadData(file);

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
        this->setCurrentTab((tabs_t)file->readVal<int>());

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

void FullProgrammerUI::setFileAndTime(const QString &file, const QDateTime &time)
{
    ui->filename->setText(file);
    ui->filename->setToolTip(file);

    QString str = time.toString(tr(" | h:mm:ss M.d.yyyy"));
    ui->filedate->setText(str);
    ui->filedate->setToolTip(str);
}

void FullProgrammerUI::setFileNeverFlashed(bool neverFlashed)
{
    if(neverFlashed)
        ui->new_indicator->setText(tr(" | NEW"));
    else
        ui->new_indicator->setText(QString());
}

void FullProgrammerUI::setChipId(const QString &text)
{
    ui->chipIdLabel->setText(text);
    ui->chipIdLabel->setToolTip(text);
}

void FullProgrammerUI::pwmRadioClicked(int id)
{
    ui->pwmStack->setCurrentIndex(id);
    if(!this->prog())
        return;

    switch(id)
    {
        case 0:
            this->prog()->setPwmFreq(0, 0.f);
            break;
        case 1:
            setPwmGeneric(true);
            break;
        case 2:
            setPwmServo(true);
            break;
    }
}

void FullProgrammerUI::pwmChanged(uint32_t freq_hz, float duty_cycle)
{
    if (freq_hz == 0)
    {
        ui->disablePwmRadio->setChecked(true);
        ui->pwmStack->setCurrentIndex(0);
    }
    else
    {
        if(ui->disablePwmRadio->isChecked())
            ui->genericPwmRadio->click();

        if(!ui->pwmFreqSpin->hasFocus() && !ui->dutySpin->hasFocus())
        {
            ui->pwmFreqSpin->setValue(freq_hz);
            ui->dutySpin->setValue(round(duty_cycle*100));
        }
        ui->pwmReal->setText(tr("%1 Hz @ %2%").arg(freq_hz).arg(round(duty_cycle*100)));
    }
}

void FullProgrammerUI::setPwmGeneric(bool force)
{
    if(!this->prog())
        return;

    if(ui->pwmFreqSpin->value() < 2)
        return;

    if(force || ui->pwmFreqSpin->hasFocus() || ui->dutySpin->hasFocus())
        this->prog()->setPwmFreq(ui->pwmFreqSpin->value(), float(ui->dutySpin->value())/100);
}

void FullProgrammerUI::setPwmServo(bool force)
{
    if(!this->prog())
        return;

    if(!force && !ui->servoPosSlider->hasFocus() && !ui->servoPosSpin->hasFocus() &&
        !ui->minServoPeriodBox->hasFocus() && !ui->maxServoPeriodBox->hasFocus())
        return;

    const uint32_t freq = 50;
    const uint32_t freq_ms = 1000/freq;
    const float maxP = ui->maxServoPeriodBox->value();
    const float minP = ui->minServoPeriodBox->value();

    if(minP > maxP)
        return;

    const float pctPos = float(ui->servoPosSlider->value())/ui->servoPosSlider->maximum();
    const float duty_cycle_ms = minP + (maxP - minP)*pctPos;
    const float duty_cycle = duty_cycle_ms/freq_ms;
    ui->pwmFreqSpin->setValue(freq);
    ui->dutySpin->setValue(round(duty_cycle*100));
    this->prog()->setPwmFreq(freq, duty_cycle);
}

void FullProgrammerUI::hexEditMenuReq(const QPoint &/*p*/)
{
    Q_ASSERT(sender() && sender()->inherits("QHexEdit"));

    QByteArray fill = FloatingInputDialog::getBytes(tr("Fill with..."), "0xFF");
    if(fill.isEmpty())
        return;

    QHexEdit *h = (QHexEdit*)sender();
    int data_size = h->data().size();
    int fill_size = fill.size();

    QByteArray data = fill.repeated(data_size/fill_size);
    data += fill.left(data_size%fill_size);

    h->setData(data);
}

void FullProgrammerUI::progButtonClicked()
{
    auto *btn = m_widget->getConnBtn();
    if(btn)
        btn->choose();
}
