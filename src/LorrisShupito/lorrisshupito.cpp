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

#define QT_USE_FAST_CONCATENATION

#include <QMessageBox>
#include <QTimer>
#include <stdio.h>
#include <QComboBox>
#include <qhexedit.h>
#include <QByteArray>
#include <QProgressDialog>
#include <QSignalMapper>
#include <QRadioButton>

#include "shupito.h"
#include "lorrisshupito.h"
#include "ui_lorrisshupito.h"
#include "shupitomode.h"
#include "fusewidget.h"
#include "shared/hexfile.h"
#include "shared/chipdefs.h"

static const QString colorFromDevice = "#C0FFFF";
static const QString colorFromFile   = "#C0FFC0";
static const QString colorSavedToFile= "#FFE0E0";
static const QString memNames[] = { "", "flash", "eeprom" };

static const QString filters = QObject::tr("Intel HEX file (*.hex)");

LorrisShupito::LorrisShupito() : WorkTab(),ui(new Ui::LorrisShupito)
{
    ui->setupUi(this);

    m_fuse_widget = new FuseWidget(this);
    ui->mainLayout->addWidget(m_fuse_widget);

    m_cur_mode = sConfig.get(CFG_QUINT32_SHUPITO_MODE);
    if(m_cur_mode >= MODE_COUNT)
        m_cur_mode = MODE_SPI;

    m_prog_speed_hz = sConfig.get(CFG_QUINT32_SHUPITO_PRG_SPEED);
    if(m_prog_speed_hz < 1 || m_prog_speed_hz >= 6000000)
        m_prog_speed_hz = 250000;

    ui->progSpeedBox->setEditText(QString::number(m_prog_speed_hz));

    if(!sConfig.get(CFG_BOOL_SHUPITO_SHOW_LOG))
    {
        ui->logText->hide();
        ui->hideLogBtn->setText("^");
    }

    if(!sConfig.get(CFG_BOOL_SHUPITO_SHOW_FUSES))
    {
        m_fuse_widget->hide();
        ui->hideFusesBtn->setText("<");
    }

    ui->tunnelCheck->setChecked(sConfig.get(CFG_BOOL_SHUPITO_TUNNEL));

    connect(ui->connectButton,  SIGNAL(clicked()),                SLOT(connectButton()));
    connect(ui->tunnelSpeedBox, SIGNAL(editTextChanged(QString)), SLOT(tunnelSpeedChanged(QString)));
    connect(ui->tunnelCheck,    SIGNAL(clicked(bool)),            SLOT(tunnelToggled(bool)));
    connect(ui->readButton,     SIGNAL(clicked()),                SLOT(readMemButton()));
    connect(ui->progSpeedBox,   SIGNAL(editTextChanged(QString)), SLOT(progSpeedChanged(QString)));
    connect(ui->hideLogBtn,     SIGNAL(clicked()),                SLOT(hideLogBtn()));
    connect(ui->eraseButton,    SIGNAL(clicked()),                SLOT(eraseDevice()));
    connect(ui->hideFusesBtn,   SIGNAL(clicked()),                SLOT(hideFusesBtn()));
    connect(ui->writeButton,    SIGNAL(clicked()),                SLOT(writeFlashBtn()));
    connect(m_fuse_widget,      SIGNAL(readFuses()),              SLOT(readFusesInFlash()));
    connect(m_fuse_widget,      SIGNAL(status(QString)),          SLOT(status(QString)));
    connect(m_fuse_widget,      SIGNAL(writeFuses()),             SLOT(writeFusesInFlash()));

    initMenuBar();

    QByteArray data = QByteArray(1024, (char)0xFF);
    static const QString memNames[] = { tr("Program memory"), tr("EEPROM") };
    m_hexAreas[0] = NULL;
    for(quint8 i = 0; i < 2; ++i)
    {
        QHexEdit *h = new QHexEdit(this);
        h->setData(data);
        m_hexAreas[i+1] = h;
        ui->memTabs->addTab(h, memNames[i]);
    }

    m_shupito = new Shupito(this);
    m_desc = new ShupitoDesc();

    connect(m_shupito, SIGNAL(descRead()), this, SLOT(descRead()));
    connect(m_shupito, SIGNAL(responseReceived(char)), this, SLOT(responseReceived(char)));
    connect(m_shupito, SIGNAL(vccValueChanged(quint8,double)), this, SLOT(vccValueChanged(quint8,double)));
    connect(m_shupito, SIGNAL(vddDesc(vdd_setup)), this, SLOT(vddSetup(vdd_setup)));
    connect(m_shupito, SIGNAL(tunnelStatus(bool)), this, SLOT(tunnelStateChanged(bool)));

    m_shupito->setTunnelSpeed(ui->tunnelSpeedBox->itemText(0).toInt(), false);

    m_response = RESPONSE_NONE;

    responseTimer = NULL;
    m_vcc = 0;
    lastVccIndex = 0;

    m_vdd_config = NULL;
    m_tunnel_config = NULL;

    for(quint8 i = 0; i < MODE_COUNT; ++i)
        m_modes[i] = ShupitoMode::getMode(i, m_shupito);

    m_progress_dialog = NULL;
    m_color = VDD_BLACK;
    m_vdd_signals = NULL;
}

LorrisShupito::~LorrisShupito()
{
    sConfig.set(CFG_BOOL_SHUPITO_VERIFY, m_auto_verify->isChecked());

    stopAll();
    delete m_shupito;
    delete m_desc;
    delete ui;

    for(quint8 i = 0; i < MODE_COUNT; ++i)
        delete m_modes[i];

    delete m_vdd_signals;
}

void LorrisShupito::initMenuBar()
{
    QMenu *chipBar = new QMenu(tr("Chip"), this);
    m_menus.push_back(chipBar);

    m_start_act = chipBar->addAction(tr("Start chip"));
    m_stop_act = chipBar->addAction(tr("Stop chip"));
    m_restart_act = chipBar->addAction(tr("Restart chip"));

    m_start_act->setEnabled(false);
    m_restart_act->setShortcut(QKeySequence("R"));

    connect(m_start_act,  SIGNAL(triggered()), SLOT(startChip()));
    connect(m_stop_act,   SIGNAL(triggered()), SLOT(stopChip()));
    connect(m_restart_act, SIGNAL(triggered()), SLOT(restartChip()));

    QMenu *modeBar = new QMenu(tr("Mode"), this);
    m_menus.push_back(modeBar);

    static const QString modeNames[] = { "SPI", "PDI", "JTAG" };

    QSignalMapper *signalMap = new QSignalMapper(this);

    for(quint8 i = 0; i < MODE_COUNT; ++i)
    {
        m_mode_act[i] = modeBar->addAction(modeNames[i]);
        m_mode_act[i]->setCheckable(true);

        signalMap->setMapping(m_mode_act[i], i);
        connect(m_mode_act[i], SIGNAL(triggered()), signalMap, SLOT(map()));
    }
    m_mode_act[m_cur_mode]->setChecked(true);
    connect(signalMap, SIGNAL(mapped(int)), SLOT(modeSelected(int)));

    modeBar->addSeparator();
    m_auto_verify = modeBar->addAction(tr("Verify write"));
    m_auto_verify->setCheckable(true);
    m_auto_verify->setChecked(sConfig.get(CFG_BOOL_SHUPITO_VERIFY));

    QMenu *dataBar = new QMenu(tr("Data"), this);
    m_menus.push_back(dataBar);

    m_load_flash = dataBar->addAction(tr("Load data into flash"));
    m_load_flash->setShortcut(QKeySequence("Ctrl+O"));
    m_load_eeprom = dataBar->addAction(tr("Load data into EEPROM"));
    dataBar->addSeparator();

    m_save_flash = dataBar->addAction(tr("Save flash memory"));
    m_save_flash->setShortcut(QKeySequence("Ctrl+S"));
    m_save_eeprom = dataBar->addAction(tr("Save EERPOM"));

    QSignalMapper *signalMapLoad = new QSignalMapper(this);
    signalMapLoad->setMapping(m_load_flash,  MEM_FLASH);
    signalMapLoad->setMapping(m_load_eeprom, MEM_EEPROM);
    connect(signalMapLoad, SIGNAL(mapped(int)), this,          SLOT(loadFromFile(int)));
    connect(m_load_flash,  SIGNAL(triggered()), signalMapLoad, SLOT(map()));
    connect(m_load_eeprom, SIGNAL(triggered()), signalMapLoad, SLOT(map()));

    QSignalMapper *signalMapSave = new QSignalMapper(this);
    signalMapSave->setMapping(m_save_flash,  MEM_FLASH);
    signalMapSave->setMapping(m_save_eeprom, MEM_EEPROM);
    connect(signalMapSave, SIGNAL(mapped(int)), this,          SLOT(saveToFile(int)));
    connect(m_save_flash,  SIGNAL(triggered()), signalMapSave, SLOT(map()));
    connect(m_save_eeprom, SIGNAL(triggered()), signalMapSave, SLOT(map()));

    QMenu *readBtnMenu = new QMenu(this);
    QAction *readFlash = readBtnMenu->addAction(tr("Read flash"));
    readBtnMenu->addSeparator();
    QAction *readAll = readBtnMenu->addAction(tr("Read all"));
    QAction *readEEPROM = readBtnMenu->addAction(tr("Read EEPROM"));
    QAction *readFuses = readBtnMenu->addAction(tr("Read fuses"));

    connect(readAll,    SIGNAL(triggered()), SLOT(readAll()));
    connect(readFlash,  SIGNAL(triggered()), SLOT(readMemButton()));
    connect(readEEPROM, SIGNAL(triggered()), SLOT(readEEPROMBtn()));
    connect(readFuses,  SIGNAL(triggered()), SLOT(readFusesInFlash()));

    ui->readButton->setMenu(readBtnMenu);

    QMenu *writeBtnMenu = new QMenu(this);
    QAction *writeFlash = writeBtnMenu->addAction(tr("Write flash"));
    writeBtnMenu->addSeparator();
    QAction *writeAll = writeBtnMenu->addAction(tr("Write all"));
    QAction *writeEEPROM = writeBtnMenu->addAction(tr("Write EEPROM"));
    QAction *writeFuses = writeBtnMenu->addAction(tr("Write fuses"));

    connect(writeAll,    SIGNAL(triggered()), SLOT(writeAll()));
    connect(writeFlash,  SIGNAL(triggered()), SLOT(writeFlashBtn()));
    connect(writeEEPROM, SIGNAL(triggered()), SLOT(writeEEPROMBtn()));
    connect(writeFuses,  SIGNAL(triggered()), SLOT(writeFusesInFlash()));

    ui->writeButton->setMenu(writeBtnMenu);
}

void LorrisShupito::connectButton()
{
    if(m_state & STATE_DISCONNECTED)
    {
        ui->connectButton->setText(tr("Connecting..."));
        ui->connectButton->setEnabled(false);
        connect(m_con, SIGNAL(connectResult(Connection*,bool)), this, SLOT(connectionResult(Connection*,bool)));
        m_con->OpenConcurrent();
    }
    else
    {
        stopAll();
        m_con->Close();
        m_state |= STATE_DISCONNECTED;

        ui->connectButton->setText(tr("Connect"));
    }
}

void LorrisShupito::connectionResult(Connection */*con*/,bool result)
{
    disconnect(m_con, SIGNAL(connectResult(Connection*,bool)), this, 0);

    ui->connectButton->setEnabled(true);
    if(!result)
    {
        ui->connectButton->setText(tr("Connect"));
        showErrorBox(tr("Can't open connection!"));
    }
}

void LorrisShupito::connectedStatus(bool connected)
{
    if(connected)
    {
        m_state &= ~(STATE_DISCONNECTED);
        ui->connectButton->setText(tr("Disconnect"));
        stopAll();
        m_shupito->init(m_con, m_desc);
    }
    else
    {
        m_state |= STATE_DISCONNECTED;
        ui->connectButton->setText(tr("Connect"));  
    }
    ui->tunnelCheck->setEnabled(connected);
    ui->tunnelSpeedBox->setEnabled(connected);
    ui->progSpeedBox->setEnabled(connected);
    ui->readButton->setEnabled(connected);
    ui->writeButton->setEnabled(connected);
    ui->eraseButton->setEnabled(connected);

    for(quint8 i = 0; i < m_vdd_radios.size(); ++i)
        m_vdd_radios[i]->setEnabled(connected);
}

void LorrisShupito::readData(const QByteArray &data)
{
    m_shupito->readData(data);
}

void LorrisShupito::stopAll()
{
    if(m_tunnel_config)
    {
        ui->tunnelCheck->setEnabled(false);
        ui->tunnelCheck->setChecked(false);

        m_shupito->setTunnelState(false);

        if(!m_tunnel_config->always_active())
        {
            sendAndWait(m_tunnel_config->getStateChangeCmd(false).getData(false));
            m_response = RESPONSE_NONE;
        }
    }

    if(m_vdd_config && !m_vdd_config->always_active())
    {
        sendAndWait(m_vdd_config->getStateChangeCmd(false).getData(false));
        m_response = RESPONSE_NONE;
    }
}

void LorrisShupito::sendAndWait(const QByteArray &data)
{
    Q_ASSERT(responseTimer == NULL);

    responseTimer = new QTimer;
    responseTimer->start(1000);
    connect(responseTimer, SIGNAL(timeout()), this, SIGNAL(responseChanged()));

    m_response = RESPONSE_WAITING;

    QEventLoop loop;
    loop.connect(this, SIGNAL(responseChanged()), SLOT(quit()));

    m_con->SendData(data);

    loop.exec();

    delete responseTimer;
    responseTimer = NULL;

    if(m_response == RESPONSE_WAITING)
        m_response = RESPONSE_BAD;
}

void LorrisShupito::responseReceived(char error_code)
{
    if(responseTimer)
        responseTimer->stop();
    if(error_code == 0)
        m_response = RESPONSE_GOOD;
    else
        m_response = RESPONSE_BAD;
    emit responseChanged();
}

void LorrisShupito::onTabShow()
{
    sConfig.set(CFG_STRING_SHUPITO_PORT, m_con->GetIDString());
}

void LorrisShupito::descRead()
{
    log("Device GUID: " % m_desc->getGuid());

    ShupitoDesc::intf_map map = m_desc->getInterfaceMap();
    for(ShupitoDesc::intf_map::iterator itr = map.begin(); itr != map.end(); ++itr)
        log("Got interface GUID: " % itr->first);

    m_vdd_config = m_desc->getConfig("1d4738a0-fc34-4f71-aa73-57881b278cb1");
    m_shupito->setVddConfig(m_vdd_config);
    if(m_vdd_config)
    {
        if(!m_vdd_config->always_active())
        {
            sendAndWait(m_vdd_config->getStateChangeCmd(true).getData(false));
            if(m_response == RESPONSE_GOOD)
                log("VDD started!");
            else
                log("Could not start VDD!");
            m_response = RESPONSE_NONE;
        }
        ShupitoPacket packet(m_vdd_config->cmd, 2, 0, 0);
        m_con->SendData(packet.getData(false));
    }

    m_tunnel_config = m_desc->getConfig("356e9bf7-8718-4965-94a4-0be370c8797c");
    m_shupito->setTunnelConfig(m_tunnel_config);
    if(m_tunnel_config && sConfig.get(CFG_BOOL_SHUPITO_TUNNEL))
    {
        if(!m_tunnel_config->always_active())
        {
            sendAndWait(m_tunnel_config->getStateChangeCmd(true).getData(false));
            if(m_response == RESPONSE_GOOD)
                log("Tunnel started!");
            else
                log("Could not start tunnel!");
            m_response = RESPONSE_NONE;
        }

        m_shupito->setTunnelState(true);
    }else
        ui->tunnelCheck->setChecked(false);
}

void LorrisShupito::vccValueChanged(quint8 id, double value)
{
    if(id == 0 && !ui->engineLabel->text().isEmpty())
    {
        if((value < 0 ? -value : value) < 0.03)
            value = 0;

        char buff[24];
        sprintf(buff, "%4.2fV", value);
        ui->vccLabel->setText(QString(buff));

        changeVddColor(value);
        m_vcc = value;
    }
}

void LorrisShupito::changeVddColor(float val)
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

    ui->vccLabel->setStyleSheet("color: " % vddColorHTML[newClr]);
    m_color = newClr;
}

//void do_vdd_setup(avrflash::device<comm>::vdd_setup const & vs)
void LorrisShupito::vddSetup(const vdd_setup &vs)
{
    m_vdd_setup = vs;

    for(quint8 i = 0; i < m_vdd_radios.size(); ++i)
        delete m_vdd_radios[i];
    m_vdd_radios.clear();

    if(vs.empty())
    {
        ui->engineLabel->setText("");
        return;
    }

    if(m_vdd_signals)
    {
        disconnect(this, SLOT(vddIndexChanged(int)));
        delete m_vdd_signals;
    }
    m_vdd_signals = new QSignalMapper(this);

    for(quint8 i = 0; i < vs[0].drives.size() && i < 5; ++i)
    {
        QRadioButton *rad = new QRadioButton(vs[0].drives[i], this);
        m_vdd_radios.push_back(rad);
        ui->vddLayout->addWidget(rad);

        m_vdd_signals->setMapping(rad, i);
        connect(rad, SIGNAL(clicked()), m_vdd_signals, SLOT(map()));
    }
    lastVccIndex = vs[0].current_drive;

    if(m_vdd_radios.size() > (uint)lastVccIndex)
        m_vdd_radios[lastVccIndex]->setChecked(true);

    ui->engineLabel->setText(vs[0].name);

    connect(m_vdd_signals, SIGNAL(mapped(int)), this, SLOT(vddIndexChanged(int)));
}

void LorrisShupito::vddIndexChanged(int index)
{
    if(index == -1)
        return;

    if(lastVccIndex == 0 && index > 0 && m_vcc != 0)
    {
        m_vdd_radios[0]->setChecked(true);
        showErrorBox(tr("Can't set output VCC, voltage detected!"));
        return;
    }

    lastVccIndex = index;

    ShupitoPacket p(MSG_VCC, 3, 1, 2, quint8(index));
    m_con->SendData(p.getData(false));
}

void LorrisShupito::tunnelSpeedChanged(const QString &text)
{
    bool ok = false;
    quint32 speed = 0;

    if(text.length() != 0 && text.length() < 8)
        speed = text.toInt(&ok);

    if(ok)
    {
        ui->tunnelSpeedBox->setStyleSheet("");
        m_shupito->setTunnelSpeed(speed);
    }
    else
        ui->tunnelSpeedBox->setStyleSheet("background-color: #FF7777");
}

void LorrisShupito::tunnelToggled(bool enable)
{
    if(!m_tunnel_config)
    {
        if(enable)
            showErrorBox(tr("It looks like your Shupito does not support RS232 tunnel!"));
        return;
    }

    m_shupito->setTunnelState(enable);
    sConfig.set(CFG_BOOL_SHUPITO_TUNNEL, enable);
}

void LorrisShupito::tunnelStateChanged(bool opened)
{
    if(!m_tunnel_config)
        return;

    QString text = tr("RS232 tunnel ");

    if(opened)
    {
        if(!ui->tunnelCheck->isEnabled())
            ui->tunnelCheck->setEnabled(true);
        text += tr("enabled");
    }
    else
    {
        if(ui->tunnelCheck->isChecked())
            ui->tunnelCheck->setEnabled(false);
        text += tr("disabled");
    }

    log(text);
    ui->tunnelCheck->setChecked(opened);
}

void LorrisShupito::modeSelected(int idx)
{
    if(!m_modes[idx])
        showErrorBox(tr("This mode is unsupported by Lorris, for now."));
    else
    {
        m_cur_mode = idx;
        sConfig.set(CFG_QUINT32_SHUPITO_MODE, idx);
    }
    for(quint8 i = 0; i < MODE_COUNT; ++i)
        m_mode_act[i]->setChecked(i == m_cur_mode);
}

void LorrisShupito::progSpeedChanged(QString text)
{
    bool ok;
    quint32 speed = text.toInt(&ok);
    if(!ok)
    {
        ui->progSpeedBox->setStyleSheet("background-color: red");
        return;
    }
    else
        ui->progSpeedBox->setStyleSheet("");

    m_prog_speed_hz = speed;
    sConfig.set(CFG_QUINT32_SHUPITO_PRG_SPEED, m_prog_speed_hz);
}

void LorrisShupito::log(const QString &text)
{
    ui->logText->appendPlainText(text);
}

void LorrisShupito::status(const QString &text)
{
    ui->statusLabel->setText(text);
}

bool LorrisShupito::checkVoltage(bool active)
{
    bool error = false;
    if(active && m_vcc == 0.0)
        error = true;
    if(!active && m_vcc != 0.0)
        error = true;

    if(error)
    {
        if(active)
            showErrorBox(tr("No voltage present"));
        else
            showErrorBox(tr("Output voltage detected!"));
    }
    return !error;
}

// avrflash::chip_definition update_chip_description(std::string const & chip_id), avr232client.cpp
chip_definition LorrisShupito::update_chip_description(const QString& chip_id)
{
    chip_definition cd;
    cd.setSign(chip_id);
    chip_definition::update_chipdef(m_shupito->getDefs(), cd);

    if(cd.getName().isEmpty())
    {
        if(cd.getSign().isEmpty())
            ui->chipIdLabel->setText(tr("<unknown>"));
        else
            ui->chipIdLabel->setText("<" % cd.getSign() % ">");
    }
    else
    {
        QString name = cd.getName();

        chip_definition::memorydef *mem = cd.getMemDef("flash");
        if(mem)
            name += " flash: " % QString::number(mem->size/1024) % " kb, page: " %
                    QString::number(mem->pagesize) % " bytes";

        mem = cd.getMemDef("eeprom");
        if(mem)
            name += "  EEPROM: " % QString::number(mem->size) % " bytes";

        ui->chipIdLabel->setText(name);
        m_cur_def = cd;
    }
    return cd;
}

void LorrisShupito::showProgressDialog(const QString& text, QObject *sender)
{
    Q_ASSERT(!m_progress_dialog);

    m_progress_dialog = new QProgressDialog(this, Qt::CustomizeWindowHint);
    m_progress_dialog->setWindowTitle(tr("Progress"));
    m_progress_dialog->setLabelText(text);
    m_progress_dialog->setMaximum(100);
    m_progress_dialog->setFixedSize(500, m_progress_dialog->height());
    m_progress_dialog->open();

    if(sender)
    {
        connect(sender, SIGNAL(updateProgressDialog(int)),    this, SLOT(updateProgressDialog(int)));
        connect(sender, SIGNAL(updateProgressLabel(QString)), this, SLOT(updateProgressLabel(QString)));
        connect(m_progress_dialog, SIGNAL(canceled()), sender, SLOT(cancelRequested()));
    }
}

void LorrisShupito::updateProgressDialog(int value)
{
    if(!m_progress_dialog)
        return;

    if(value == -1 || value == 100)
    {
        m_progress_dialog->close();
        delete m_progress_dialog;
        m_progress_dialog = NULL;
        disconnect(this, SLOT(updateProgressDialog(int)));
        disconnect(this, SLOT(updateProgressLabel(QString)));
        return;
    }
    m_progress_dialog->setValue(value);
}

void LorrisShupito::updateProgressLabel(const QString &text)
{
    if(!m_progress_dialog)
        return;
    m_progress_dialog->setLabelText(text);
}

void LorrisShupito::hideLogBtn()
{
   ui->logText->setVisible(!ui->logText->isVisible());
   sConfig.set(CFG_BOOL_SHUPITO_SHOW_LOG, ui->logText->isVisible());

   if(ui->logText->isVisible())
       ui->hideLogBtn->setText("v");
   else
       ui->hideLogBtn->setText("^");
}

void LorrisShupito::hideFusesBtn()
{
    hideFuses(m_fuse_widget->isVisible());
}

void LorrisShupito::hideFuses(bool hide)
{
    m_fuse_widget->setVisible(!hide);
    sConfig.set(CFG_BOOL_SHUPITO_SHOW_FUSES, !hide);

    if(m_fuse_widget->isVisible())
        ui->hideFusesBtn->setText(">");
    else
        ui->hideFusesBtn->setText("<");
}

chip_definition LorrisShupito::switchToFlashAndGetId()
{
    log("Swithing to flash mode");

    m_modes[m_cur_mode]->switchToFlashMode(m_prog_speed_hz);

    log("Reading device id");

    QString id = m_modes[m_cur_mode]->readDeviceId();
    m_shupito->setChipId(id);

    log("Got device id: " % id);

    chip_definition chip = update_chip_description(id);

    if(ui->chipIdLabel->text().contains("<"))
        throw QString(tr("Unsupported chip: %1")).arg(id);

    postFlashSwitchCheck(chip);

    return chip;
}

//void post_flash_switch_check(), avrclient232.cpp
void LorrisShupito::postFlashSwitchCheck(chip_definition& chip)
{
    if(m_fuse_widget->getChipDef().getSign() != chip.getSign())
        m_fuse_widget->clear(true);
}

void LorrisShupito::showErrorBox(const QString &text)
{
    QMessageBox box(this);
    box.setIcon(QMessageBox::Critical);
    box.setWindowTitle(tr("Error!"));
    box.setText(text);
    box.exec();
}

bool LorrisShupito::showContinueBox(const QString &title, const QString &text)
{
    QMessageBox box(this);
    box.setWindowTitle(title);
    box.setText(text);
    box.addButton(tr("Yes"), QMessageBox::YesRole);
    box.addButton(tr("No"), QMessageBox::NoRole);
    box.setIcon(QMessageBox::Question);
    return !((bool)box.exec());
}

void LorrisShupito::startChip()
{
    if(!checkVoltage(true))
        return;

    status("");
    try
    {
        log("Switching to run mode");
        m_modes[m_cur_mode]->switchToRunMode();

        status(tr("Chip has been started"));
    }
    catch(QString ex)
    {
        showErrorBox(ex);
    }
    m_start_act->setEnabled(false);
    m_stop_act->setEnabled(true);
}

void LorrisShupito::stopChip()
{
    if(!checkVoltage(true))
        return;

    status("");

    try
    {
        log("Swithing to flash mode");
        switchToFlashAndGetId();

        status(tr("Chip has been stopped"));
    }
    catch(QString ex)
    {
        showErrorBox(ex);
    }
    m_start_act->setEnabled(true);
    m_stop_act->setEnabled(false);
}

void LorrisShupito::restartChip()
{
    if(!checkVoltage(true))
        return;

    status("");

    stopChip();
    startChip();
}

void LorrisShupito::loadFromFile(int memId)
{
    try
    {
        if(m_cur_def.getName().isEmpty())
            restartChip();

        QString filename = QFileDialog::getOpenFileName(this, QObject::tr("Import data"),
                                                        sConfig.get(CFG_SHUPITO_HEX_FOLDER),
                                                        filters);
        if(filename.isEmpty())
            return;

        sConfig.set(CFG_SHUPITO_HEX_FOLDER, filename);

        status("");

        HexFile file;

        file.LoadFromFile(filename);

        quint32 len = 0;
        if(!m_cur_def.getName().isEmpty())
        {
            chip_definition::memorydef *memdef = m_cur_def.getMemDef(memId);
            if(memdef)
                len = memdef->size;
        }
        m_hexAreas[memId]->setData(file.getDataArray(len));
        m_hexAreas[memId]->setBackgroundColor(colorFromFile);

        status(tr("File loaded"));
    }
    catch(QString ex)
    {
        showErrorBox(ex);
    }
}

void LorrisShupito::saveToFile(int memId)
{
    try
    {
        if(m_cur_def.getName().isEmpty())
            restartChip();

        QString filename = QFileDialog::getSaveFileName(this, QObject::tr("Export data"),
                                                        sConfig.get(CFG_SHUPITO_HEX_FOLDER),
                                                        filters);
        if(filename.isEmpty())
            return;

        sConfig.set(CFG_SHUPITO_HEX_FOLDER, filename);

        status("");

        HexFile file;
        file.setData(m_hexAreas[memId]->data());
        file.SaveToFile(filename);

        m_hexAreas[memId]->setBackgroundColor(colorFromFile);
        m_hexAreas[memId]->clearDataChanged();

        status(tr("File saved"));
    }
    catch(QString ex)
    {
        showErrorBox(ex);
    }
}

void LorrisShupito::readAll()
{
    if(!checkVoltage(true))
        return;

    status("");

    try
    {
        bool restart = !m_modes[m_cur_mode]->isInFlashMode();
        chip_definition chip = switchToFlashAndGetId();

        readMem(MEM_FLASH, chip);
        readMem(MEM_EEPROM, chip);
        readFuses(chip);

        if(restart)
        {
            log("Switching to run mode");
            m_modes[m_cur_mode]->switchToRunMode();
        }

        status(tr("Data has been successfuly read"));
    }
    catch(QString ex)
    {
        updateProgressDialog(-1);
        showErrorBox(ex);
    }
}

void LorrisShupito::readMemInFlash(quint8 memId)
{
    if(!checkVoltage(true))
        return;

    try
    {
        bool restart = !m_modes[m_cur_mode]->isInFlashMode();
        chip_definition chip = switchToFlashAndGetId();

        readMem(memId, chip);

        if(restart)
        {
            log("Switching to run mode");
            m_modes[m_cur_mode]->switchToRunMode();
        }

        ui->memTabs->setCurrentIndex(memId - 1);
    }
    catch(QString ex)
    {
        updateProgressDialog(-1);

        showErrorBox(ex);
    }
}

void LorrisShupito::readFusesInFlash()
{
    if(!checkVoltage(true))
        return;

    status("");

    try
    {
        bool restart = !m_modes[m_cur_mode]->isInFlashMode();
        chip_definition chip = switchToFlashAndGetId();

        readFuses(chip);

        if(restart)
        {
            log("Switching to run mode");
            m_modes[m_cur_mode]->switchToRunMode();
        }
        status(tr("Fuses had been succesfully read"));
    }
    catch(QString ex)
    {
        showErrorBox(ex);
    }
}

void LorrisShupito::readMem(quint8 memId, chip_definition &chip)
{
    log("Reading memory");

    showProgressDialog(tr("Reading memory"), m_modes[m_cur_mode]);
    QByteArray mem = m_modes[m_cur_mode]->readMemory(memNames[memId], chip);

    m_hexAreas[memId]->setData(mem);
    m_hexAreas[memId]->setBackgroundColor(colorFromDevice);

    updateProgressDialog(-1);
}

void LorrisShupito::readFuses(chip_definition& chip)
{
    log("Reading fuses");
    std::vector<quint8>& data = m_fuse_widget->getFuseData();
    data.clear();

    m_modes[m_cur_mode]->readFuses(data, chip);
    m_fuse_widget->setFuses(chip);

    hideFuses(false);
}

void LorrisShupito::writeAll()
{
    if(!checkVoltage(true))
        return;

    status("");

    try
    {
        bool restart = !m_modes[m_cur_mode]->isInFlashMode();
        chip_definition chip = switchToFlashAndGetId();

        writeMem(MEM_FLASH, chip);
        writeMem(MEM_EEPROM, chip);
        writeFuses(chip);

        if(restart)
        {
            log("Switching to run mode");
            m_modes[m_cur_mode]->switchToRunMode();
        }

        status(tr("Data has been successfuly written"));
    }
    catch(QString ex)
    {
        updateProgressDialog(-1);
        showErrorBox(ex);
    }
}

void LorrisShupito::writeMemInFlash(quint8 memId)
{
    if(!checkVoltage(true))
        return;

    status("");

    try
    {
        bool restart = !m_modes[m_cur_mode]->isInFlashMode();
        chip_definition chip = switchToFlashAndGetId();

        writeMem(memId, chip);

        if(restart)
        {
            log("Switching to run mode");
            m_modes[m_cur_mode]->switchToRunMode();
        }

        status(tr("Data has been successfuly written"));
    }
    catch(QString ex)
    {
        updateProgressDialog(-1);
        showErrorBox(ex);
    }
}

void LorrisShupito::writeFusesInFlash()
{
    if(!checkVoltage(true))
        return;

    status("");

    if(!m_fuse_widget->isLoaded())
    {
        showErrorBox(tr("Fuses had not been read yet"));
        return;
    }

    if(m_fuse_widget->isChanged())
    {
        showErrorBox(tr("You have to \"Remember\" fuses prior to writing"));
        return;
    }

    if(!showContinueBox(tr("Write fuses?"), tr("Do you really wanna to write fuses to the chip?")))
        return;

    try
    {
        bool restart = !m_modes[m_cur_mode]->isInFlashMode();
        chip_definition chip = switchToFlashAndGetId();

        writeFuses(chip);

        if(restart)
        {
            log("Switching to run mode");
            m_modes[m_cur_mode]->switchToRunMode();
        }
        status(tr("Fuses had been succesfully written"));
    }
    catch(QString ex)
    {
        showErrorBox(ex);
    }
}

void LorrisShupito::writeMem(quint8 memId, chip_definition &chip)
{
    chip_definition::memorydef *memdef = chip.getMemDef(memId);

    if(!memdef)
        throw QString(tr("Unknown memory id"));

    QByteArray data = m_hexAreas[memId]->data();
    if((quint32)data.size() != memdef->size)
        throw QString(tr("Somethings wrong, data in tab: %1, chip size: %2")).arg(data.size()).arg(memdef->size);

    log("Writing memory");
    showProgressDialog(tr("Writing memory"), m_modes[m_cur_mode]);

    HexFile file;
    file.setData(data);

    m_modes[m_cur_mode]->flashRaw(file, memId, chip, m_auto_verify->isChecked());
    m_hexAreas[memId]->setBackgroundColor(colorFromDevice);

    updateProgressDialog(-1);
}

void LorrisShupito::writeFuses(chip_definition &chip)
{
    if(!m_fuse_widget->isLoaded())
        throw QString(tr("Fuses had not been read yet"));

    log("Writing fuses");
    std::vector<quint8>& data = m_fuse_widget->getFuseData();
    m_modes[m_cur_mode]->writeFuses(data, chip, m_auto_verify->isChecked());
}

void LorrisShupito::eraseDevice()
{
    if(!checkVoltage(true))
        return;

    if(!showContinueBox(tr("Erase chip?"), tr("Do you really wanna to erase WHOLE chip?")))
        return;

    try
    {
        bool restart = !m_modes[m_cur_mode]->isInFlashMode();

        switchToFlashAndGetId();

        log("Erasing device");
        showProgressDialog(tr("Erasing chip..."));
        m_modes[m_cur_mode]->erase_device();

        if(restart)
        {
            log("Switching to run mode");
            m_modes[m_cur_mode]->switchToRunMode();
        }

        updateProgressDialog(-1);
    }
    catch(QString ex)
    {
        updateProgressDialog(-1);
        showErrorBox(ex);
        return;
    }

    QMessageBox succesBox(this);
    succesBox.setIcon(QMessageBox::Information);
    succesBox.setWindowTitle(tr("Succes!"));
    succesBox.setText(tr("Chip was succesfuly erased!"));
    succesBox.exec();
}
