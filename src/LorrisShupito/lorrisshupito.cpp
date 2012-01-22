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

#include "shupito.h"
#include "lorrisshupito.h"
#include "ui_lorrisshupito.h"
#include "shupitomode.h"
#include "chipdefs.h"
#include "fusewidget.h"

static const QString colorFromDevice = "#C0FFFF";
static const QString colorFromFile   = "#C0FFC0";
static const QString colorSavedToFile= "#FFE0E0";

LorrisShupito::LorrisShupito() : WorkTab(),ui(new Ui::LorrisShupito)
{
    ui->setupUi(this);

    m_fuse_widget = new FuseWidget(this);
    ui->mainLayout->addWidget(m_fuse_widget);

    m_cur_mode = sConfig.get(CFG_QUINT32_SHUPITO_MODE);
    if(m_cur_mode >= MODE_COUNT)
        m_cur_mode = MODE_SPI;

    int progIdx = sConfig.get(CFG_QUINT32_SHUPITO_PRG_SPEED);
    if(progIdx < 0 || progIdx >= ui->progSpeedBox->count())
        progIdx = 0;

    ui->progSpeedBox->setCurrentIndex(progIdx);

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
    connect(ui->progSpeedBox,   SIGNAL(currentIndexChanged(int)), SLOT(progSpeedChanged(int)));
    connect(ui->readEEPROM,     SIGNAL(clicked()),                SLOT(readEEPROMBtn()));
    connect(ui->hideLogBtn,     SIGNAL(clicked()),                SLOT(hideLogBtn()));
    connect(ui->eraseBtn,       SIGNAL(clicked()),                SLOT(eraseDevice()));
    connect(ui->hideFusesBtn,   SIGNAL(clicked()),                SLOT(hideFusesBtn()));
    connect(m_fuse_widget,      SIGNAL(readFuses()),              SLOT(readFuses()));
    connect(m_fuse_widget,      SIGNAL(status(QString)),          SLOT(status(QString)));

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
}

LorrisShupito::~LorrisShupito()
{
    stopAll();
    delete m_shupito;
    delete m_desc;
    delete ui;

    for(quint8 i = 0; i < MODE_COUNT; ++i)
        delete m_modes[i];
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
    ui->vddBox->setEnabled(connected);
}

void LorrisShupito::readData(const QByteArray &data)
{
    m_shupito->readData(data);
}

void LorrisShupito::stopAll()
{
    if(m_tunnel_config)
    {
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
    if(m_tunnel_config && ui->tunnelCheck->isChecked())
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
    if(id == 0 && vccText.length() != 0)
    {
        if((value < 0 ? -value : value) < 0.03)
            value = 0;
        m_vcc = value;
        char buff[24];
        sprintf(buff, " %4.2f", value);
        ui->vccLabel->setText(vccText % QString(buff));
    }
}

//void do_vdd_setup(avrflash::device<comm>::vdd_setup const & vs)
void LorrisShupito::vddSetup(const vdd_setup &vs)
{
    m_vdd_setup = vs;

    disconnect(ui->vddBox, SIGNAL(currentIndexChanged(int)), 0, 0);

    ui->vddBox->clear();

    if(vs.empty())
    {
        vccText = "";
        return;
    }

    for(quint8 i = 0; i < vs[0].drives.size(); ++i)
        ui->vddBox->addItem(vs[0].drives[i]);
    lastVccIndex = vs[0].current_drive;
    ui->vddBox->setCurrentIndex(vs[0].current_drive);
    vccText = vs[0].name;

    connect(ui->vddBox, SIGNAL(currentIndexChanged(int)), this, SLOT(vddIndexChanged(int)));
}

void LorrisShupito::vddIndexChanged(int index)
{
    if(index == -1)
        return;

    if(lastVccIndex == 0 && index > 0 && m_vcc != 0)
    {
        ui->vddBox->setCurrentIndex(0);
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
        text += tr("enabled");
    else
        text += tr("disabled");

    log(text);
    if(ui->tunnelCheck->isChecked())
        ui->tunnelCheck->setEnabled(opened);
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

void LorrisShupito::readMem(quint8 memId)
{
    if(!checkVoltage(true))
        return;

    try
    {
        bool restart = !m_modes[m_cur_mode]->isInFlashMode();
        chip_definition chip = switchToFlashAndGetId();

        log("Reading memory");

        static const QString memNames[] = { "", "flash", "eeprom" };
        showProgressDialog(tr("Reading memory"), m_modes[m_cur_mode]);
        QByteArray mem = m_modes[m_cur_mode]->readMemory(memNames[memId], chip);

        m_hexAreas[memId]->setData(mem);
        m_hexAreas[memId]->setBackgroundColor(colorFromDevice);

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

void LorrisShupito::progSpeedChanged(int idx)
{
    sConfig.set(CFG_QUINT32_SHUPITO_PRG_SPEED, idx);
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
        connect(sender, SIGNAL(updateProgressDialog(int)), this, SLOT(updateProgressDialog(int)));
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
        return;
    }
    m_progress_dialog->setValue(value);
}

void LorrisShupito::readMemButton()
{
    readMem(MEM_FLASH);
}

void LorrisShupito::readEEPROMBtn()
{
    readMem(MEM_EEPROM);
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
    m_fuse_widget->setVisible(!m_fuse_widget->isVisible());
    sConfig.set(CFG_BOOL_SHUPITO_SHOW_FUSES, m_fuse_widget->isVisible());

    if(m_fuse_widget->isVisible())
        ui->hideFusesBtn->setText(">");
    else
        ui->hideFusesBtn->setText("<");
}

chip_definition LorrisShupito::switchToFlashAndGetId()
{
    log("Swithing to flash mode");

    int speed_hz = ui->progSpeedBox->currentText().toInt();
    m_modes[m_cur_mode]->switchToFlashMode(speed_hz);

    log("Reading device id");

    QString id = m_modes[m_cur_mode]->readDeviceId();
    m_shupito->setChipId(id);

    log("Got device id: " % id);

    chip_definition chip = update_chip_description(id);

    if(ui->chipIdLabel->text().contains("<"))
        throw QString(tr("Unsupported chip: %1")).arg(id);

    //TODO:
    //void post_flash_switch_check(), avrclient232.cpp

    return chip;
}

void LorrisShupito::showErrorBox(const QString &text)
{
    QMessageBox box(this);
    box.setIcon(QMessageBox::Critical);
    box.setWindowTitle(tr("Error!"));
    box.setText(text);
    box.exec();
}

void LorrisShupito::eraseDevice()
{
    if(!checkVoltage(true))
        return;

    QMessageBox box(this);
    box.setWindowTitle(tr("Erase chip?"));
    box.setText(tr("Do you really wanna to erase WHOLE chip?"));
    box.addButton(tr("Yes"), QMessageBox::YesRole);
    box.addButton(tr("No"), QMessageBox::NoRole);
    box.setIcon(QMessageBox::Question);
    if(box.exec())
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

    status(tr("Chip has been restarted"));
}

void LorrisShupito::readFuses()
{
    if(!checkVoltage(true))
        return;

    status("");

    try
    {
        bool restart = !m_modes[m_cur_mode]->isInFlashMode();
        chip_definition chip = switchToFlashAndGetId();

        log("Reading fuses");
        std::vector<quint8>& data = m_fuse_widget->getFuseData();
        data.clear();

        m_modes[m_cur_mode]->readFuses(data, chip);
        m_fuse_widget->setFuses(chip.getFuses());

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
