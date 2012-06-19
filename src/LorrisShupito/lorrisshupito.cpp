/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

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
#include <QFileInfo>
#include <QToolBar>
#include <QIntValidator>

#include "progressdialog.h"
#include "shupito.h"
#include "lorrisshupito.h"
#include "modes/shupitomode.h"
#include "fusewidget.h"
#include "../shared/hexfile.h"
#include "../shared/chipdefs.h"
#include "flashbuttonmenu.h"
#include "overvccdialog.h"
#include "../ui/tooltipwarn.h"

#include "ui_lorrisshupito.h"

static const QString colorFromDevice = "#C0FFFF";
static const QString colorFromFile   = "#C0FFC0";
static const QString colorSavedToFile= "#FFE0E0";
static const QString memNames[] = { "", "flash", "eeprom" };

static const QString filters = QObject::tr("Intel HEX file (*.hex)");

LorrisShupito::LorrisShupito()
    : ui(new Ui::LorrisShupito)
{
    ui->setupUi(this);

    m_chipStopped = false;
    m_overvcc_dialog = NULL;
    m_overvcc = 0.0;
    m_enable_overvcc = false;

    m_fuse_widget = new FuseWidget(this);
    ui->mainLayout->addWidget(m_fuse_widget);

    m_cur_mode = sConfig.get(CFG_QUINT32_SHUPITO_MODE);
    if(m_cur_mode >= MODE_COUNT)
        m_cur_mode = MODE_SPI;

    m_prog_speed_hz = sConfig.get(CFG_QUINT32_SHUPITO_PRG_SPEED);
    if(m_prog_speed_hz < 1)
        m_prog_speed_hz = 2000000;

    ui->progSpeedBox->setEditText(QString::number(m_prog_speed_hz));

    ui->tunnelCheck->setChecked(sConfig.get(CFG_BOOL_SHUPITO_TUNNEL));

    connect(ui->tunnelSpeedBox,  SIGNAL(editTextChanged(QString)), SLOT(tunnelSpeedChanged(QString)));
    connect(ui->tunnelCheck,     SIGNAL(clicked(bool)),            SLOT(tunnelToggled(bool)));
    connect(ui->progSpeedBox,    SIGNAL(editTextChanged(QString)), SLOT(progSpeedChanged(QString)));
    connect(ui->hideLogBtn,      SIGNAL(clicked(bool)),            SLOT(hideLogBtn(bool)));
    connect(ui->eraseButton,     SIGNAL(clicked()),                SLOT(eraseDevice()));
    connect(ui->hideFusesBtn,    SIGNAL(clicked(bool)),            SLOT(hideFusesBtn(bool)));
    connect(ui->settingsBtn,     SIGNAL(clicked(bool)),            SLOT(hideSettingsBtn(bool)));
    connect(ui->over_enable,     SIGNAL(toggled(bool)),            SLOT(overvoltageSwitched(bool)));
    connect(ui->over_val,        SIGNAL(valueChanged(double)),     SLOT(overvoltageChanged(double)));
    connect(ui->over_turnoff,    SIGNAL(clicked(bool)),            SLOT(overvoltageTurnOffVcc(bool)));
    connect(ui->startStopBtn,    SIGNAL(clicked()),                SLOT(startstopChip()));
    connect(ui->flashWarnBox,    SIGNAL(clicked(bool)),            SLOT(flashWarnBox(bool)));
    connect(m_fuse_widget,       SIGNAL(readFuses()),              SLOT(readFusesInFlash()));
    connect(m_fuse_widget,       SIGNAL(status(QString)),          SLOT(status(QString)));
    connect(m_fuse_widget,       SIGNAL(writeFuses()),             SLOT(writeFusesInFlash()));
    connect(qApp,                SIGNAL(focusChanged(QWidget*,QWidget*)), SLOT(focusChanged(QWidget*,QWidget*)));

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

    m_terminal = new Terminal(this);
    m_terminal->setFmt(sConfig.get(CFG_QUITN32_SHUPITO_TERM_FMT));
    m_terminal->loadSettings(sConfig.get(CFG_STRING_SHUPITO_TERM_SET));
    ui->memTabs->addTab(m_terminal, tr("Terminal"));

    QByteArray data = QByteArray(1024, (char)0xFF);
    static const QString memNames[] = { tr("Program memory"), tr("EEPROM") };
    m_hexAreas[0] = NULL;
    for(quint8 i = 1; i < TAB_MAX; ++i)
    {
        QHexEdit *h = new QHexEdit(this);
        h->setData(data);
        m_hexAreas[i] = h;
        ui->memTabs->addTab(h, memNames[i-1]);
    }

    m_shupito = new Shupito(this);
    m_desc = NULL;

    connect(m_shupito, SIGNAL(descRead(bool)), this, SLOT(descRead(bool)));
    connect(m_shupito, SIGNAL(vccValueChanged(quint8,double)), this, SLOT(vccValueChanged(quint8,double)));
    connect(m_shupito, SIGNAL(vddDesc(vdd_setup)), this, SLOT(vddSetup(vdd_setup)));
    connect(m_shupito, SIGNAL(tunnelStatus(bool)), this, SLOT(tunnelStateChanged(bool)));

    m_shupito->setTunnelSpeed(ui->tunnelSpeedBox->itemText(0).toInt(), false);

    connect(m_terminal, SIGNAL(settingsChanged()),           this,       SLOT(saveTermSettings()));
    connect(m_terminal, SIGNAL(keyPressed(QString)),         m_shupito,  SLOT(sendTunnelData(QString)));
    connect(m_shupito,  SIGNAL(tunnelData(QByteArray)),      m_terminal, SLOT(appendText(QByteArray)));

    m_vcc = 0;
    lastVccIndex = 0;

    m_vdd_config = NULL;
    m_tunnel_config = NULL;

    for(quint8 i = 0; i < MODE_COUNT; ++i)
        m_modes[i] = ShupitoMode::getMode(i, m_shupito);

    m_progress_dialog = NULL;
    m_color = VDD_BLACK;
    m_vdd_signals = NULL;

    m_state = 0;

    ui->over_enable->setChecked(sConfig.get(CFG_BOOL_SHUPITO_OVERVOLTAGE));
    ui->over_val->setValue(sConfig.get(CFG_FLOAT_SHUPITO_OVERVOLTAGE_VAL));
    ui->over_turnoff->setChecked(sConfig.get(CFG_BOOL_SHUPITO_TURNOFF_VCC));

    ui->flashWarnBox->setChecked(sConfig.get(CFG_BOOL_SHUPITO_SHOW_FLASH_WARN));

    m_connectButton = new ConnectButton(ui->connectButton);
    connect(m_connectButton, SIGNAL(connectionChosen(PortConnection*)), this, SLOT(setConnection(PortConnection*)));
}

LorrisShupito::~LorrisShupito()
{
    sConfig.set(CFG_QUITN32_SHUPITO_TERM_FMT, m_terminal->getFmt());

    stopAll(false);
    delete m_shupito;
    delete m_desc;
    delete ui;

    for(quint8 i = 0; i < MODE_COUNT; ++i)
        delete m_modes[i];

    delete m_vdd_signals;
}

void LorrisShupito::initMenus()
{
    // Flash/Read buttons
    FlashButtonMenu *readBtnMenu = new FlashButtonMenu(true, ui->readButton, this);
    FlashButtonMenu *writeBtnMenu = new FlashButtonMenu(false, ui->writeButton, this);

    connect(ui->memTabs, SIGNAL(currentChanged(int)), readBtnMenu,  SLOT(setActiveAction(int)));
    connect(ui->memTabs, SIGNAL(currentChanged(int)), writeBtnMenu, SLOT(setActiveAction(int)));

    // top menu bar
    QMenu *chipBar = new QMenu(tr("Chip"), this);
    addTopMenu(chipBar);

    m_start_act = chipBar->addAction(QIcon(":/actions/start"), tr("Start chip"));
    m_stop_act = chipBar->addAction(QIcon(":/actions/stop"), tr("Stop chip"));
    m_restart_act = chipBar->addAction(QIcon(":/actions/refresh"), tr("Restart chip"));

    m_start_act->setEnabled(false);
    m_restart_act->setShortcut(QKeySequence("R"));

    connect(m_start_act,  SIGNAL(triggered()), SLOT(startChip()));
    connect(m_stop_act,   SIGNAL(triggered()), SLOT(stopChip()));
    connect(m_restart_act, SIGNAL(triggered()), SLOT(restartChip()));

    QMenu *modeBar = new QMenu(tr("Mode"), this);
    addTopMenu(modeBar);

    static const QString modeNames[] = { "SPI", "PDI", "JTAG", "cc25xx" };

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

    QMenu *verifyMenu = modeBar->addMenu(tr("Verify write"));
    QSignalMapper *verifyMap = new QSignalMapper(this);

    for(quint8 i = 0; i < VERIFY_MAX; ++i)
    {
        static const QString verifyText[] =
        {
            tr("None"),
            tr("Verify only non-empty pages"),
            tr("Verify all")
        };

        m_verify[i] = verifyMenu->addAction(verifyText[i]);
        m_verify[i]->setCheckable(true);
        verifyMap->setMapping(m_verify[i], i);

        connect(m_verify[i], SIGNAL(triggered()), verifyMap, SLOT(map()));
    }

    connect(verifyMap, SIGNAL(mapped(int)), SLOT(verifyChanged(int)));
    verifyChanged(sConfig.get(CFG_QUINT32_SHUPITO_VERIFY));

    QAction *setTunnelName = modeBar->addAction(tr("Set RS232 tunnel name..."));
    connect(setTunnelName, SIGNAL(triggered()), SLOT(setTunnelName()));

    QMenu *dataBar = new QMenu(tr("Data"), this);
    addTopMenu(dataBar);

    m_load_flash = dataBar->addAction(QIcon(":/actions/open"), tr("Load data into flash"));
    m_load_flash->setShortcut(QKeySequence("Ctrl+O"));
    m_load_eeprom = dataBar->addAction(tr("Load data into EEPROM"));
    dataBar->addSeparator();

    m_save_flash = dataBar->addAction(QIcon(":/actions/save"), tr("Save flash memory"));
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

    QToolBar *bar = new QToolBar(this);
    ui->topLayout->insertWidget(1, bar);
    bar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    bar->setIconSize(QSize(16, 16));

    bar->addAction(m_load_flash);
    bar->addAction(m_save_flash);
    bar->addSeparator();

    QPushButton *btn = new QPushButton(QIcon(":/actions/wire"), tr("Mode"), this);
    btn->setFlat(true);
    btn->setMenu(modeBar);
    bar->addWidget(btn);
}

void LorrisShupito::connDisconnecting()
{
    stopAll(false);
}

void LorrisShupito::connectionResult(Connection */*con*/,bool result)
{
    disconnect(m_con, SIGNAL(connectResult(Connection*,bool)), this, 0);

    if(!result)
    {
        Utils::ThrowException(tr("Can't open connection!"));
    }
}

void LorrisShupito::connectedStatus(bool connected)
{
    if(connected)
    {
        m_state &= ~(STATE_DISCONNECTED);
        stopAll(true);

        delete m_desc;
        m_desc = new ShupitoDesc();

        m_shupito->init(m_con, m_desc);
    }
    else
    {
        m_state |= STATE_DISCONNECTED;
        updateStartStopUi(false);
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

void LorrisShupito::stopAll(bool wait)
{
    if(m_tunnel_config)
    {
        ui->tunnelCheck->setEnabled(false);
        ui->tunnelCheck->setChecked(false);

        m_shupito->setTunnelState(false);
        m_shupito->setTunnelPipe(0);

        if(!m_tunnel_config->always_active())
        {
            ShupitoPacket pkt = m_tunnel_config->getStateChangeCmd(false);
            if(wait)
                m_shupito->waitForPacket(pkt, MSG_INFO);
            else
                m_shupito->sendPacket(pkt);
        }
    }

    if(m_vdd_config && !m_vdd_config->always_active())
    {
        ShupitoPacket pkt = m_vdd_config->getStateChangeCmd(false);
        if(wait)
            m_shupito->waitForPacket(pkt, MSG_INFO);
        else
            m_shupito->sendPacket(pkt);
    }
}

void LorrisShupito::onTabShow()
{
    if (!m_con)
    {
        m_connectButton->choose();
        if (m_con && !m_con->isOpen())
            m_con->OpenConcurrent();
    }

    if(m_con && m_con->getType() == CONNECTION_SERIAL_PORT)
        sConfig.set(CFG_STRING_SHUPITO_PORT, m_con->GetIDString());
}

void LorrisShupito::descRead(bool correct)
{
    if(!correct)
    {
        log("Failed to read info from shupito!");
        return Utils::ThrowException(tr("Failed to read info from Shupito. If you're sure "
                        "you're connected to shupito, try to disconnect and "
                        "connect again"));
    }

    log("Device GUID: " % m_desc->getGuid());

    ShupitoDesc::intf_map map = m_desc->getInterfaceMap();
    for(ShupitoDesc::intf_map::iterator itr = map.begin(); itr != map.end(); ++itr)
        log("Got interface GUID: " % itr.key());

    m_vdd_config = m_desc->getConfig("1d4738a0-fc34-4f71-aa73-57881b278cb1");
    m_shupito->setVddConfig(m_vdd_config);
    if(m_vdd_config)
    {
        if(!m_vdd_config->always_active())
        {
            ShupitoPacket pkt = m_vdd_config->getStateChangeCmd(true);
            pkt = m_shupito->waitForPacket(pkt, MSG_INFO);

            if(pkt.getLen() == 1 && pkt[0] == 0)
                log("VDD started!");
            else
                log("Could not start VDD!");
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
            ShupitoPacket pkt = m_tunnel_config->getStateChangeCmd(true);
            pkt = m_shupito->waitForPacket(pkt, MSG_INFO);

            if(pkt.getLen() == 1 && pkt[0] == 0)
                log("Tunnel started!");
            else
                log("Could not start tunnel!");
        }

        m_shupito->setTunnelState(true);
    }else
        ui->tunnelCheck->setChecked(false);
}

void LorrisShupito::vccValueChanged(quint8 id, double value)
{
    if(id == 0 && !ui->engineLabel->text().isEmpty() && m_vcc != value)
    {
        if((value < 0 ? -value : value) < 0.03)
            value = 0;

        ui->vccLabel->setText(QString("%1").arg(value, 3, 'f', 2, '0'));

        changeVddColor(value);
        m_vcc = value;

        checkOvervoltage();
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

    disableOvervoltVDDs();

    connect(m_vdd_signals, SIGNAL(mapped(int)), this, SLOT(vddIndexChanged(int)));
}

void LorrisShupito::vddIndexChanged(int index)
{
    if(index == -1)
        return;

    if(lastVccIndex == 0 && index > 0 && m_vcc != 0)
    {
        m_vdd_radios[0]->setChecked(true);
        Utils::ThrowException(tr("Can't set output VCC, voltage detected!"));
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
    speed = text.toInt(&ok);
    if(ok)
        m_shupito->setTunnelSpeed(speed);
}

void LorrisShupito::tunnelToggled(bool enable)
{
    if(!m_tunnel_config)
    {
        if(enable)
            Utils::ThrowException(tr("It looks like your Shupito does not support RS232 tunnel!"));
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

void LorrisShupito::setTunnelName()
{
    QString name = QInputDialog::getText(this, tr("Set tunnel name"), tr("Tunnel name:"),
                              QLineEdit::Normal, sConfig.get(CFG_STRING_SHUPITO_TUNNEL));

    if(name.isEmpty())
        return;

    sConfig.set(CFG_STRING_SHUPITO_TUNNEL, name);
    m_shupito->setTunnelState(false, true);
    m_shupito->setTunnelState(true);
}

void LorrisShupito::modeSelected(int idx)
{
    if(!m_modes[idx])
        Utils::ThrowException(tr("This mode is unsupported by Lorris, for now."));
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
        return;

    m_prog_speed_hz = speed;
    sConfig.set(CFG_QUINT32_SHUPITO_PRG_SPEED, m_prog_speed_hz);
}

void LorrisShupito::log(const QString &text)
{
    ui->logText->appendPlainText(text);
}

void LorrisShupito::status(const QString &text)
{
    emit statusBarMsg(text, 5000);
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
            Utils::ThrowException(tr("No voltage present"));
        else
            Utils::ThrowException(tr("Output voltage detected!"));
    }
    return !error;
}

// avrflash::chip_definition update_chip_description(std::string const & chip_id), avr232client.cpp
void LorrisShupito::update_chip_description(chip_definition& cd)
{
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
}

void LorrisShupito::showProgressDialog(const QString& text, QObject *sender)
{
    Q_ASSERT(!m_progress_dialog);

    m_progress_dialog = new ProgressDialog(text, this);
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

void LorrisShupito::hideLogBtn(bool checked)
{
    ui->hideLogBtn->setChecked(checked);
    ui->logText->setVisible(checked);
    sConfig.set(CFG_BOOL_SHUPITO_SHOW_LOG, checked);
}

void LorrisShupito::hideFusesBtn(bool checked)
{
    ui->hideFusesBtn->setChecked(checked);
    m_fuse_widget->setVisible(checked);
    sConfig.set(CFG_BOOL_SHUPITO_SHOW_FUSES, checked);
}

void LorrisShupito::hideSettingsBtn(bool checked)
{
    ui->progBox->setVisible(checked);
    ui->tunnelBox->setVisible(checked);
    ui->overvccBox->setVisible(checked);
    ui->settingsBtn->setChecked(checked);
    sConfig.set(CFG_BOOL_SHUPITO_SHOW_SETTINGS, checked);
}

chip_definition LorrisShupito::switchToFlashAndGetId()
{
    log("Swithing to flash mode");

    m_modes[m_cur_mode]->switchToFlashMode(m_prog_speed_hz);

    log("Reading device id");

    chip_definition chip = m_modes[m_cur_mode]->readDeviceId();
    m_shupito->setChipId(chip);

    log("Got device id: " % chip.getSign());

    update_chip_description(chip);

    if(ui->chipIdLabel->text().contains("<"))
        throw QString(tr("Unsupported chip: %1")).arg(chip.getSign());

    postFlashSwitchCheck(chip);

    return chip;
}

//void post_flash_switch_check(), avrclient232.cpp
void LorrisShupito::postFlashSwitchCheck(chip_definition& chip)
{
    if(m_fuse_widget->getChipDef().getSign() != chip.getSign())
        m_fuse_widget->clear(true);
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

void LorrisShupito::startstopChip()
{
    if (m_chipStopped)
        startChip();
    else
        stopChip();
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
        Utils::ThrowException(ex);
    }

    updateStartStopUi(false);
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
        Utils::ThrowException(ex);

        // The chip has not been stopped.
        return;
    }

    updateStartStopUi(true);
}

void LorrisShupito::updateStartStopUi(bool stopped)
{
    if (stopped)
    {
        m_start_act->setEnabled(true);
        m_stop_act->setEnabled(false);
        ui->startStopBtn->setIcon(QIcon(":/icons/start"));
        ui->startStopBtn->setText(tr("Start"));
    }
    else
    {
        m_start_act->setEnabled(false);
        m_stop_act->setEnabled(true);
        ui->startStopBtn->setIcon(QIcon(":/icons/stop"));
        ui->startStopBtn->setText(tr("Stop"));
    }

    m_chipStopped = stopped;
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
        QString filename = QFileDialog::getOpenFileName(this, QObject::tr("Import data"),
                                                        sConfig.get(CFG_STRING_SHUPITO_HEX_FOLDER),
                                                        filters);
        if(filename.isEmpty())
            return;

        sConfig.set(CFG_STRING_SHUPITO_HEX_FOLDER, filename);

        loadFromFile(memId, filename);
    }
    catch(QString ex)
    {
        Utils::ThrowException(ex);
    }
}

void LorrisShupito::loadFromFile(int memId, const QString& filename)
{
    status("");

    QDateTime loadTimestamp = QDateTime::currentDateTime();

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
    m_hexFilenames[memId] = filename;
    m_hexWriteTimes[memId] = loadTimestamp;

    if(memId == MEM_FLASH)
    {
        ui->filename->setText(filename);
        ui->filename->setToolTip(filename);

        QDateTime lastMod = QFileInfo(filename).lastModified();
        QString time = lastMod.toString(tr(" | h:mm:ss d.M.yyyy"));
        ui->filedate->setText(time);
        ui->filedate->setToolTip(time);
    }

    status(tr("File loaded"));
}

void LorrisShupito::openFile(const QString &filename)
{
    try
    {
        loadFromFile(MEM_FLASH, filename);
        sConfig.set(CFG_STRING_SHUPITO_HEX_FOLDER, filename);
    }
    catch(QString ex)
    {
        Utils::ThrowException(ex);
    }
}

void LorrisShupito::saveToFile(int memId)
{
    try
    {
        if(m_cur_def.getName().isEmpty())
            restartChip();

        QString filename = QFileDialog::getSaveFileName(this, QObject::tr("Export data"),
                                                        sConfig.get(CFG_STRING_SHUPITO_HEX_FOLDER),
                                                        filters);
        if(filename.isEmpty())
            return;

        sConfig.set(CFG_STRING_SHUPITO_HEX_FOLDER, filename);

        status("");

        HexFile file;
        file.setData(m_hexAreas[memId]->data());
        file.SaveToFile(filename);

        m_hexAreas[memId]->setBackgroundColor(colorFromFile);
        m_hexAreas[memId]->clearDataChanged();
        m_hexFilenames[memId] = filename;

        status(tr("File saved"));
    }
    catch(QString ex)
    {
        Utils::ThrowException(ex);
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
        Utils::ThrowException(ex);
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

        ui->memTabs->setCurrentIndex(memId);
    }
    catch(QString ex)
    {
        updateProgressDialog(-1);

        Utils::ThrowException(ex);
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
        Utils::ThrowException(ex);
    }
}

void LorrisShupito::readMem(quint8 memId, chip_definition &chip)
{
    log("Reading memory");

    showProgressDialog(tr("Reading memory"), m_modes[m_cur_mode]);
    QByteArray mem = m_modes[m_cur_mode]->readMemory(memNames[memId], chip);
    m_hexAreas[memId]->setData(mem);
    m_hexAreas[memId]->setBackgroundColor(colorFromDevice);
    m_hexFilenames[memId].clear();

    updateProgressDialog(-1);
}

void LorrisShupito::readFuses(chip_definition& chip)
{
    log("Reading fuses");

    m_fuse_widget->setUpdatesEnabled(false);
    m_fuse_widget->clear(false);

    std::vector<quint8>& data = m_fuse_widget->getFuseData();
    data.clear();

    m_modes[m_cur_mode]->readFuses(data, chip);
    m_fuse_widget->setFuses(chip);

    m_fuse_widget->setUpdatesEnabled(true);

    hideFusesBtn(true);
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
        Utils::ThrowException(ex);
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

        m_hexAreas[memId]->clearDataChanged();

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
        Utils::ThrowException(ex);
    }
}

void LorrisShupito::writeFusesInFlash()
{
    if(!checkVoltage(true))
        return;

    status("");

    if(!m_fuse_widget->isLoaded())
    {
        Utils::ThrowException(tr("Fuses had not been read yet"));
        return;
    }

    if(m_fuse_widget->isChanged())
    {
        Utils::ThrowException(tr("You have to \"Remember\" fuses prior to writing"));
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
        Utils::ThrowException(ex);
    }
}

void LorrisShupito::writeMem(quint8 memId, chip_definition &chip)
{
    tryFileReload(memId);

    chip_definition::memorydef *memdef = chip.getMemDef(memId);

    if(!memdef)
        throw QString(tr("Unknown memory id"));

    QByteArray data = m_hexAreas[memId]->data();
    if((quint32)data.size() > memdef->size)
        throw QString(tr("Somethings wrong, data in tab: %1, chip size: %2")).arg(data.size()).arg(memdef->size);

    log("Writing memory");
    showProgressDialog(tr("Writing memory"), m_modes[m_cur_mode]);

    QDateTime lastMod = m_hexFlashTimes[memId];
    if(!m_hexFilenames[memId].isEmpty())
    {
        QFileInfo info(m_hexFilenames[memId]);
        if(ui->flashWarnBox->isChecked() && info.exists() && m_hexFlashTimes[memId] == info.lastModified())
            new ToolTipWarn(tr("You have flashed this file already, and it was not changed since."), ui->writeButton, this);
        lastMod = info.lastModified();
    }

    HexFile file;
    file.setData(data);

    m_modes[m_cur_mode]->flashRaw(file, memId, chip, m_verify_mode);
    m_hexAreas[memId]->setBackgroundColor(colorFromDevice);

    updateProgressDialog(-1);

    m_hexFlashTimes[memId] = lastMod;
}

void LorrisShupito::writeFuses(chip_definition &chip)
{
    if(!m_fuse_widget->isLoaded())
        throw QString(tr("Fuses had not been read yet"));

    log("Writing fuses");
    std::vector<quint8>& data = m_fuse_widget->getFuseData();
    m_modes[m_cur_mode]->writeFuses(data, chip, m_verify_mode);
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

        chip_definition cd = switchToFlashAndGetId();

        log("Erasing device");
        showProgressDialog(tr("Erasing chip..."));
        m_modes[m_cur_mode]->erase_device(cd);

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
        Utils::ThrowException(ex);
        return;
    }

    QMessageBox succesBox(this);
    succesBox.setIcon(QMessageBox::Information);
    succesBox.setWindowTitle(tr("Succes!"));
    succesBox.setText(tr("Chip was succesfuly erased!"));
    succesBox.exec();
}

void LorrisShupito::verifyChanged(int mode)
{
    for(quint8 i = 0; i < VERIFY_MAX; ++i)
        m_verify[i]->setChecked(i == mode);

    sConfig.set(CFG_QUINT32_SHUPITO_VERIFY, mode);

    m_verify_mode = mode;
}

void LorrisShupito::tryFileReload(quint8 memId)
{
    if (m_hexFilenames[memId].isEmpty() || m_hexAreas[memId]->hasDataChanged())
        return;

    QDateTime writeTime = QFileInfo(m_hexFilenames[memId]).lastModified();
    if (writeTime <= m_hexWriteTimes[memId])
        return;

    try
    {
        loadFromFile(memId, m_hexFilenames[memId]);
    }
    catch (QString const &)
    {
        // Ignore errors.
    }
}

void LorrisShupito::focusChanged(QWidget *prev, QWidget */*curr*/)
{
    if(prev == NULL)
        tryFileReload(getMemIndex());
}

int LorrisShupito::getMemIndex()
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

void LorrisShupito::setConnection(PortConnection *con)
{
    if (m_con)
        disconnect(m_con, 0, this, 0);
    this->PortConnWorkTab::setConnection(con);
    m_connectButton->setConn(con);
    if (m_con)
        connect(m_con, SIGNAL(disconnecting()), this, SLOT(connDisconnecting()));
}

void LorrisShupito::saveTermSettings()
{
    sConfig.set(CFG_STRING_SHUPITO_TERM_SET, m_terminal->getSettingsData());
}

void LorrisShupito::overvoltageSwitched(bool enabled)
{
    sConfig.set(CFG_BOOL_SHUPITO_OVERVOLTAGE, enabled);
    m_enable_overvcc = enabled;

    if(!enabled && m_overvcc_dialog)
        delete m_overvcc_dialog;
}

void LorrisShupito::overvoltageChanged(double val)
{
    sConfig.set(CFG_FLOAT_SHUPITO_OVERVOLTAGE_VAL, val);
    m_overvcc = val;

    disableOvervoltVDDs();
}

void LorrisShupito::disableOvervoltVDDs()
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
        (*itr)->setEnabled(val < m_overvcc);
    }
}

void LorrisShupito::checkOvervoltage()
{
    if(!m_enable_overvcc)
        return;

    if(m_vcc >= m_overvcc && !m_overvcc_dialog)
    {
        if(ui->over_turnoff->isChecked())
            shutdownVcc();

        m_overvcc_dialog = new OverVccDialog(ui->over_turnoff->isChecked(), this);
        m_overvcc_dialog->show();
    }
    else if(m_overvcc_dialog && m_vcc < m_overvcc)
    {
        if(!ui->over_turnoff->isChecked())
            delete m_overvcc_dialog;
        m_overvcc_dialog = NULL;
    }
}

void LorrisShupito::shutdownVcc()
{
    if(m_vdd_setup.empty())
        return;

    for(size_t i = 0; i < m_vdd_setup[0].drives.size() && i < m_vdd_radios.size(); ++i)
    {
        if(m_vdd_setup[0].drives[i] == "<hiz>")
        {
            vddIndexChanged(i);
            Utils::printToStatusBar(tr("VCC was turned off due to overvoltage!"));
            return;
        }
    }
}

void LorrisShupito::overvoltageTurnOffVcc(bool enabled)
{
    sConfig.set(CFG_BOOL_SHUPITO_TURNOFF_VCC, enabled);
}

void LorrisShupito::flashWarnBox(bool checked)
{
    ui->flashWarnBox->setChecked(checked);
    sConfig.set(CFG_BOOL_SHUPITO_SHOW_FLASH_WARN, checked);
}
