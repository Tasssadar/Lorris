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
#include <QByteArray>
#include <QProgressDialog>
#include <QSignalMapper>
#include <QFileInfo>
#include <QStringBuilder>
#include <QInputDialog>
#include <QFileDialog>

#include "ui/progressdialog.h"
#include "shupito.h"
#include "lorrisshupito.h"
#include "modes/shupitomode.h"
#include "../shared/hexfile.h"
#include "../shared/chipdefs.h"
#include "../connection/shupitoconn.h"
#include "ui/overvccdialog.h"
#include "../ui/tooltipwarn.h"
#include "../WorkTab/WorkTabMgr.h"
#include "../connection/connectionmgr2.h"
#include "programmers/flipprogrammer.h"
#include "programmers/shupitoprogrammer.h"

// When no packet from shupito is received for TIMEOUT_INTERVAL ms,
// warning will appear
#define TIMEOUT_INTERVAL 3000

static const QString colorFromDevice = "#C0FFFF";
static const QString colorFromFile   = "#C0FFC0";
static const QString colorSavedToFile= "#FFE0E0";
static const QString memNames[] = { "", "flash", "eeprom" };

static const QString filters = QObject::tr("Intel HEX file (*.hex)");

LorrisShupito::LorrisShupito()
    : WorkTab()
{
    m_connectButton = NULL;
    ui = NULL;
    m_chipStopped = false;
    m_overvcc_dialog = NULL;
    m_overvcc = 0.0;
    m_enable_overvcc = false;
    m_overvcc_turnoff = false;
    m_vcc = 0;
    lastVccIndex = 0;
    m_progress_dialog = NULL;
    m_state = 0;

    m_mode_act_signalmap = new QSignalMapper(this);
    connect(m_mode_act_signalmap, SIGNAL(mapped(int)), SLOT(modeSelected(int)));

    m_prog_speed_hz = sConfig.get(CFG_QUINT32_SHUPITO_PRG_SPEED);
    if(m_prog_speed_hz < 1)
        m_prog_speed_hz = 2000000;

    initMenus();
    setUiType(UI_FULL);

    m_timeout_timer.setInterval(TIMEOUT_INTERVAL);

    connect(qApp,                SIGNAL(focusChanged(QWidget*,QWidget*)), SLOT(focusChanged(QWidget*,QWidget*)));
    connect(&m_timeout_timer,    SIGNAL(timeout()),                SLOT(timeout()));

    connectedStatus(false);
}

LorrisShupito::~LorrisShupito()
{
    if (m_con)
        m_con->releaseTab();

    stopAll(false);
    delete ui;
}

void LorrisShupito::updateModeBar()
{
    Q_ASSERT(m_modeBar);
    Q_ASSERT(!m_modeBar->actions().empty());

    for (size_t i = 0; i < m_mode_acts.size(); ++i)
        delete m_mode_acts[i];
    m_mode_acts.clear();

    if (!m_programmer)
        return;

    QStringList modeList = m_programmer->getAvailableModes();
    if (modeList.empty())
        return;

    QAction * before = static_cast<QAction *>(m_modeBar->actions()[0]);

    for(quint8 i = 0; i < modeList.size(); ++i)
    {
        QAction * mode_act = new QAction(modeList[i], m_modeBar);

        m_modeBar->insertAction(before, mode_act);
        mode_act->setCheckable(true);
        m_mode_acts.push_back(mode_act);

        m_mode_act_signalmap->setMapping(mode_act, i);
        connect(mode_act, SIGNAL(triggered()), m_mode_act_signalmap, SLOT(map()));
    }
 
    int currentMode = m_programmer->getMode();
    Q_ASSERT(currentMode < m_mode_acts.size());
    m_mode_acts[currentMode]->setChecked(true);

    m_mode_acts.push_back(m_modeBar->insertSeparator(before));
}

void LorrisShupito::initMenus()
{
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

    m_modeBar = new QMenu(tr("Mode"), this);
    addTopMenu(m_modeBar);

    QMenu *verifyMenu = m_modeBar->addMenu(tr("Verify write"));
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

    QAction *setTunnelName = m_modeBar->addAction(tr("Set RS232 tunnel name..."));
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

    m_miniUi = new QAction(tr("Minimal UI"), this);
    m_miniUi->setCheckable(true);
    addTopAction(m_miniUi);

    connect(m_miniUi, SIGNAL(triggered(bool)), SLOT(setMiniUi(bool)));
}

void LorrisShupito::connDisconnecting()
{
    stopAll(false);
}

void LorrisShupito::connectedStatus(bool connected)
{
    if(connected)
    {
        Q_ASSERT(m_programmer.data());
        m_state &= ~(STATE_DISCONNECTED);

        if (!m_programmer->supportsVdd())
            setEnableButtons(true);
    }
    else
    {
        m_state |= STATE_DISCONNECTED;
        updateStartStopUi(false);
        m_timeout_timer.stop();

        setEnableButtons(false);
    }
    ui->connectedStatus(connected);
}

void LorrisShupito::stopAll(bool wait)
{
    if (m_programmer)
        m_programmer->stopAll(wait);
    ui->tunnelStop(true);
}

void LorrisShupito::onTabShow(const QString& filename)
{
    if(!filename.isEmpty())
        openFile(filename);

    if (!m_con)
    {
        m_connectButton->choose();
        if (m_con && !m_con->isOpen())
            m_con->OpenConcurrent();
    }

    if(m_con && m_con->getType() == CONNECTION_SERIAL_PORT)
        sConfig.set(CFG_STRING_SHUPITO_PORT, m_con->GetIDString());
}

void LorrisShupito::vccValueChanged(quint8 id, double value)
{
    m_timeout_timer.start();
    if(m_timeout_warn)
        delete m_timeout_warn;

    if(id == 0 && !m_vdd_setup.empty() && m_vcc != value)
    {
        if((value < 0 ? -value : value) < 0.03)
            value = 0;

        setEnableButtons(value != 0);
        ui->vccValChanged(value);

        m_vcc = value;
        checkOvervoltage();
    }
}

//void do_vdd_setup(avrflash::device<comm>::vdd_setup const & vs)
void LorrisShupito::vddSetup(const vdd_setup &vs)
{
    m_vdd_setup = vs;

    ui->clearVCC();

    if(vs.empty())
        return;

    ui->vddSetup(vs);

    lastVccIndex = vs[0].current_drive;
}

void LorrisShupito::vddIndexChanged(int index)
{
    if(index == -1)
        return;

    if(lastVccIndex == 0 && index > 0 && m_vcc != 0)
    {
        ui->vddIndexChanged(0);
        Utils::showErrorBox(tr("Can't set output VCC, voltage detected!"));
        return;
    }

    lastVccIndex = index;

    if (m_programmer)
        m_programmer->setVddIndex(index);
}

void LorrisShupito::tunnelSpeedChanged(const QString &text)
{
    bool ok = false;
    quint32 speed = 0;
    speed = text.toInt(&ok);
    if(ok && m_programmer)
        m_programmer->setTunnelSpeed(speed);
}

void LorrisShupito::tunnelToggled(bool enable)
{
    if (m_programmer)
    {
        if (!m_programmer->supportsTunnel())
        {
            if(enable)
                Utils::showErrorBox(tr("It looks like your Shupito does not support RS232 tunnel!"));
            return;
        }

        m_programmer->setTunnelState(enable);
    }

    sConfig.set(CFG_BOOL_SHUPITO_TUNNEL, enable);
}

void LorrisShupito::tunnelStateChanged(bool opened)
{
    ui->tunnelStateChanged(opened);
    ui->log(tr("RS232 tunnel %1").arg(opened ? tr("enabled") : tr("disabled")));
}

void LorrisShupito::setTunnelName()
{
    QString name = QInputDialog::getText(this, tr("Set tunnel name"), tr("Tunnel name:"),
                              QLineEdit::Normal, sConfig.get(CFG_STRING_SHUPITO_TUNNEL));

    if(name.isEmpty())
        return;

    sConfig.set(CFG_STRING_SHUPITO_TUNNEL, name);
    if (m_programmer)
    {
        m_programmer->setTunnelState(false, true);
        m_programmer->setTunnelState(true);
    }
}

void LorrisShupito::modeSelected(int idx)
{
    Q_ASSERT(m_programmer);
    m_programmer->setMode(idx);

    for (size_t i = 1; i < m_mode_acts.size(); ++i)
        m_mode_acts[i-1]->setChecked(i-1 == idx);
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

void LorrisShupito::status(const QString &text)
{
    emit statusBarMsg(text, 5000);
}

bool LorrisShupito::checkVoltage(bool active)
{
    Q_ASSERT(m_programmer);
    if (!m_programmer->supportsVdd())
        return true;

    bool error = !active ^ (m_vcc == 0.0);
    if(error)
    {
        if(active)
            Utils::showErrorBox(tr("No voltage present"));
        else
            Utils::showErrorBox(tr("Output voltage detected!"));
    }
    return !error;
}

// avrflash::chip_definition update_chip_description(std::string const & chip_id), avr232client.cpp
void LorrisShupito::update_chip_description(chip_definition& cd)
{
    if(cd.getName().isEmpty())
    {
        if(cd.getSign().isEmpty())
            ui->setChipId(tr("<unknown>"));
        else
            ui->setChipId("<" % cd.getSign() % ">");
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

        ui->setChipId(name);
        m_cur_def = cd;
    }
}

void LorrisShupito::showProgressDialog(const QString& text, QObject *sender)
{
    Q_ASSERT(!m_progress_dialog);

    m_progress_dialog = new ProgressDialog(sWorkTabMgr.getWindow(getWindowId())->winId(), text, this);
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

chip_definition LorrisShupito::switchToFlashAndGetId()
{
    Q_ASSERT(m_programmer);

    ui->log("Swithing to flash mode");

    m_programmer->switchToFlashMode(m_prog_speed_hz);

    ui->log("Reading device id");

    chip_definition chip = m_programmer->readDeviceId();

    ui->log("Got device id: " % chip.getSign());

    update_chip_description(chip);

    if(chip.getName().isEmpty())
        throw QString(tr("Unsupported chip: %1")).arg(chip.getSign());

    ui->postFlashSwitchCheck(chip);

    return chip;
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
    Q_ASSERT(m_programmer);

    if(!checkVoltage(true))
        return;

    status("");
    try
    {
        ui->log("Switching to run mode");
        m_programmer->switchToRunMode();

        status(tr("Chip has been started"));
    }
    catch(QString ex)
    {
        Utils::showErrorBox(ex);
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
        ui->log("Swithing to flash mode");
        switchToFlashAndGetId();

        status(tr("Chip has been stopped"));
    }
    catch(QString ex)
    {
        Utils::showErrorBox(ex);

        // The chip has not been stopped.
        return;
    }

    updateStartStopUi(true);
}

void LorrisShupito::updateStartStopUi(bool stopped)
{
    ui->setStartStopBtn(stopped);

    m_start_act->setEnabled(stopped);
    m_stop_act->setEnabled(!stopped);

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
        Utils::showErrorBox(ex);
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

    ui->setHexData(memId, file.getDataArray(len));
    ui->setHexColor(memId, colorFromFile);

    m_hexFilenames[memId] = filename;
    m_hexWriteTimes[memId] = loadTimestamp;

    if(memId == MEM_FLASH)
        ui->setFileAndTime(filename, QFileInfo(filename).lastModified());

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
        Utils::showErrorBox(ex);
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
        file.setData(ui->getHexData(memId));
        file.SaveToFile(filename);

        ui->setHexColor(memId, colorFromFile);
        ui->clearHexChanged(memId);

        m_hexFilenames[memId] = filename;

        status(tr("File saved"));
    }
    catch(QString ex)
    {
        Utils::showErrorBox(ex);
    }
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
    if (m_hexFilenames[memId].isEmpty() || ui->hasHexChanged(memId))
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
        tryFileReload(ui->getMemIndex());
}

void LorrisShupito::setConnection(ConnectionPointer<Connection> const & con)
{
    if (m_con)
        m_con->releaseTab();

    m_con = con;
    m_programmer.reset();

    if (ConnectionPointer<ShupitoConnection> sc = con.dynamicCast<ShupitoConnection>())
    {
        m_programmer.reset(new ShupitoProgrammer(sc));
    }
    else if (ConnectionPointer<FlipConnection> fc = con.dynamicCast<FlipConnection>())
    {
        m_programmer.reset(new FlipProgrammer(fc));
    }
    else
    {
        return;
    }

    connect(m_programmer.data(), SIGNAL(vccValueChanged(quint8,double)),  SLOT(vccValueChanged(quint8,double)));
    connect(m_programmer.data(), SIGNAL(vddDesc(vdd_setup)),              SLOT(vddSetup(vdd_setup)));
    connect(m_programmer.data(), SIGNAL(tunnelStatus(bool)),              SLOT(tunnelStateChanged(bool)));

    ui->connectProgrammer(m_programmer.data());
    this->updateModeBar();

    connect(m_con.data(), SIGNAL(connected(bool)), this, SLOT(connectedStatus(bool)));
    m_con->addTabRef();

    m_connectButton->setConn(m_con, false);
    connect(m_con.data(), SIGNAL(disconnecting()), this, SLOT(connDisconnecting()));
}

void LorrisShupito::checkOvervoltage()
{
    if(!m_enable_overvcc)
        return;

    if(m_vcc >= m_overvcc && !m_overvcc_dialog)
    {
        if(m_overvcc_turnoff)
            shutdownVcc();

        m_overvcc_dialog = new OverVccDialog(m_overvcc_turnoff, this);
        m_overvcc_dialog->show();
    }
    else if(m_overvcc_dialog && m_vcc < m_overvcc)
    {
        if(!m_overvcc_turnoff)
            delete m_overvcc_dialog;
        m_overvcc_dialog = NULL;
    }
}

void LorrisShupito::shutdownVcc()
{
    if(m_vdd_setup.empty())
        return;

    for(size_t i = 0; i < m_vdd_setup[0].drives.size(); ++i)
    {
        if(m_vdd_setup[0].drives[i] == "<hiz>")
        {
            vddIndexChanged(i);
            emit statusBarMsg(tr("VCC was turned off due to overvoltage!"));
            return;
        }
    }
}

void LorrisShupito::timeout()
{
    m_timeout_timer.stop();
    if(!m_timeout_warn)
    {
        m_timeout_warn = new ToolTipWarn(tr("Shupito is not responding, try to re-plug it into computer!"),
                                         m_connectButton->btn(), this, -1);
    }
}

QString LorrisShupito::GetIdString()
{
    return "LorrisShupito";
}

void LorrisShupito::saveData(DataFileParser *file)
{
    WorkTab::saveData(file);

    file->writeBlockIdentifier("LorrShupitoFiles");
    for(int i = 1; i < MEM_FUSES; ++i)
        file->writeString(m_hexFilenames[i]);

    ui->saveData(file);

    ConnectionPointer<PortShupitoConnection> sc = m_con.dynamicCast<PortShupitoConnection>();
    if(sc)
    {
        file->writeBlockIdentifier("LorrShupitoConn");
        file->writeConn(sc->port().data());
    }
}

void LorrisShupito::loadData(DataFileParser *file)
{
    WorkTab::loadData(file);

    if(file->seekToNextBlock("LorrShupitoFiles", BLOCK_WORKTAB))
    {
        for(int i = 1; i < MEM_FUSES; ++i)
        {
            try {
                loadFromFile(i, file->readString());
            } catch(const QString&) {}
        }
    }

    if(file->seekToNextBlock("LorrShupitoUItype", BLOCK_WORKTAB))
        setUiType(file->readVal<int>());

    ui->loadData(file);

    if(file->seekToNextBlock("LorrShupitoConn", BLOCK_WORKTAB))
    {
        quint8 type = 0;
        QHash<QString, QVariant> cfg;

        if(file->readConn(type, cfg))
        {
            ConnectionPointer<PortConnection> pc = sConMgr2.getConnWithConfig(type, cfg);
            if(pc)
            {
                ConnectionPointer<ShupitoConnection> sc = sConMgr2.createAutoShupito(pc.data());
                m_connectButton->setConn(sc);

                if(!sc->isOpen() && sConfig.get(CFG_BOOL_SESSION_CONNECT))
                    sc->OpenConcurrent();
            }
        }
    }
}

void LorrisShupito::setEnableButtons(bool enable)
{
    if(!(enable ^ m_buttons_enabled))
        return;

    m_buttons_enabled = enable;
    emit enableButtons(enable);
}

void LorrisShupito::createConnBtn(QToolButton *btn)
{
    Q_ASSERT(!m_connectButton);

    m_connectButton = new ConnectButton(btn);
    m_connectButton->setConnectionTypes(pct_programmable);
    m_connectButton->setConn(m_con, false);
    connect(m_connectButton, SIGNAL(connectionChosen(ConnectionPointer<Connection>)), this, SLOT(setConnection(ConnectionPointer<Connection>)));
}

void LorrisShupito::setUiType(int type)
{
    if(ui && ui->getType() == type)
        return;

    delete m_connectButton;
    m_connectButton = NULL;

    delete ui;

    ui = ShupitoUI::createUI((ui_type)type, this);
    ui->setupUi(this);
    ui->vddSetup(m_vdd_setup);

    m_miniUi->setChecked(type == UI_MINIMAL);
}

void LorrisShupito::setMiniUi(bool mini)
{
    int type = mini ? UI_MINIMAL : UI_FULL;
    if(ui->getType() == type)
        return;

    QByteArray hexData[MEM_FUSES];
    for(quint8 i = MEM_FLASH; i < MEM_FUSES; ++i)
        hexData[i] = ui->getHexData(i);

    QByteArray data;
    DataFileParser parser(&data, QIODevice::ReadWrite);

    ui->saveData(&parser);
    parser.seek(0);

    setUiType(type);

    ui->loadData(&parser);

    for(quint8 i = MEM_FLASH; i < MEM_FUSES; ++i)
    {
        if(!hexData[i].isEmpty())
        {
            ui->setHexColor(i, colorFromFile);
            ui->setHexData(i, hexData[i]);
        }
    }

    ui->connectedStatus(!(m_state & STATE_DISCONNECTED));

    emit enableButtons(m_buttons_enabled);

    if(!m_hexFilenames[MEM_FLASH].isEmpty())
        ui->setFileAndTime(m_hexFilenames[MEM_FLASH], QFileInfo(m_hexFilenames[MEM_FLASH]).lastModified());
}
