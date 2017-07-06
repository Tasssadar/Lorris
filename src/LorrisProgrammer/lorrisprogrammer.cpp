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
#include "lorrisprogrammer.h"
#include "modes/shupitomode.h"
#include "../shared/hexfile.h"
#include "../shared/chipdefs.h"
#include "../connection/shupitoconn.h"
#include "../connection/shupitotunnel.h"
#include "ui/overvccdialog.h"
#include "../ui/tooltipwarn.h"
#include "../WorkTab/WorkTabMgr.h"
#include "../connection/connectionmgr2.h"
#include "programmers/shupitoprogrammer.h"
#include "programmers/avr232bootprogrammer.h"
#include "programmers/atsamprogrammer.h"
#include "programmers/avr109programmer.h"
#include "programmers/arduinoprogrammer.h"
#include "programmers/zmodemprogrammer.h"

#ifdef HAVE_LIBYB
#include "programmers/flipprogrammer.h"
#include "programmers/stm32programmer.h"
#endif

// When no packet from shupito is received for TIMEOUT_INTERVAL ms,
// warning will appear
#define TIMEOUT_INTERVAL 3000

static const QString colorFromDevice = "#C0FFFF";
static const QString colorFromFile   = "#C0FFC0";
static const QString colorSavedToFile= "#FFE0E0";

LorrisProgrammer::LorrisProgrammer()
    : WorkTab(), m_logsink(this)
{
    m_connectButton = NULL;
    ui = NULL;
    m_chipStopped = false;
    m_overvcc_dialog = NULL;
    m_overvcc = 0.0;
    m_enable_overvcc = false;
    m_overvcc_turnoff = false;
    lastVccIndex = 0;
    m_progress_dialog = NULL;
    m_state = 0;
    m_buttons_enabled = false;

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

LorrisProgrammer::~LorrisProgrammer()
{
    if (m_con)
        m_con->releaseTab();

    stopAll(false);
    delete ui;
}

void LorrisProgrammer::updateModeBar()
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
 
    size_t currentMode = m_programmer->getMode();
    Q_ASSERT(currentMode < m_mode_acts.size());
    m_mode_acts[currentMode]->setChecked(true);

    m_mode_acts.push_back(m_modeBar->insertSeparator(before));
}

void LorrisProgrammer::initMenus()
{
    // top menu bar
    QMenu *chipBar = new QMenu(tr("Chip"), this);
    addTopMenu(chipBar);

    m_start_act = chipBar->addAction(QIcon(":/actions/start"), tr("Start chip"));
    m_stop_act = chipBar->addAction(QIcon(":/actions/stop"), tr("Stop chip"));
    m_restart_act = chipBar->addAction(QIcon(":/actions/refresh"), tr("Restart chip"));

    m_start_act->setEnabled(false);
    m_stop_act->setEnabled(false);
    m_restart_act->setShortcut(QKeySequence("R"));
    m_restart_act->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_restart_act->setEnabled(false);

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

    m_set_tunnel_name_act = m_modeBar->addAction(tr("Set RS232 tunnel name..."));
    m_set_tunnel_name_act->setVisible(false);
    connect(m_set_tunnel_name_act, SIGNAL(triggered()), SLOT(setTunnelName()));

    m_enableHardwareButton = m_modeBar->addAction(tr("Enable hardware button"));
    m_enableHardwareButton->setCheckable(true);
    m_enableHardwareButton->setChecked(sConfig.get(CFG_BOOL_SHUPITO_ENABLE_HW_BUTTON));
    connect(m_enableHardwareButton, SIGNAL(toggled(bool)), this, SLOT(enableHardwareButtonToggled(bool)));

    m_load_flash = new QAction(QIcon(":/actions/open"), tr("Load..."), this);
    m_load_flash->setShortcut(QKeySequence("Ctrl+O"));
    m_load_flash->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(m_load_flash,  SIGNAL(triggered()), this, SLOT(loadFromFile()));
    addTopAction(m_load_flash);

    m_save_flash = new QAction(QIcon(":/actions/save"), tr("Save..."), this);
    m_save_flash->setShortcut(QKeySequence("Ctrl+S"));
    m_save_flash->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(m_save_flash,  SIGNAL(triggered()), this, SLOT(saveToFile()));
    addTopAction(m_save_flash);

    m_blink_led = new QAction(tr("Blink LED"), this);
    m_blink_led->setEnabled(false);
    connect(m_blink_led, SIGNAL(triggered()), this, SLOT(blinkLed()));
    addTopAction(m_blink_led);

    m_miniUi = new QAction(tr("Minimal UI"), this);
    m_miniUi->setCheckable(true);
    addTopAction(m_miniUi);

    connect(m_miniUi, SIGNAL(triggered(bool)), SLOT(setMiniUi(bool)));
}

void LorrisProgrammer::enableHardwareButtonToggled(bool checked)
{
    sConfig.set(CFG_BOOL_SHUPITO_ENABLE_HW_BUTTON, checked);
}

void LorrisProgrammer::connDisconnecting()
{
    stopAll(false);
}

void LorrisProgrammer::connectedStatus(bool connected)
{
    if(connected)
    {
        m_vcc = -1;
        this->updateProgrammer();
        Q_ASSERT(m_programmer.data());
        m_state &= ~(STATE_DISCONNECTED);
        updateStartStopUi(m_programmer->isInFlashMode());

        if (!m_programmer->supportsVdd())
            setEnableButtons(true);
    }
    else
    {
        this->updateProgrammer();

        m_state |= STATE_DISCONNECTED;
        updateStartStopUi(false);
        m_timeout_timer.stop();

        setEnableButtons(false);
        m_blink_led->setEnabled(false);
    }
    ui->connectedStatus(connected);
}

void LorrisProgrammer::stopAll(bool wait)
{
    if (m_programmer)
        m_programmer->stopAll(wait);
    ui->tunnelStop(true);
}

void LorrisProgrammer::onTabShow(const QString& filename)
{
    if(!filename.isEmpty())
    {
        try
        {
            loadFromFile(MEM_FLASH, filename);
            sConfig.set(CFG_STRING_SHUPITO_HEX_FOLDER, filename);
        }
        catch(QString ex)
        {
        }
    }

    if (!m_con && sConfig.get(CFG_BOOL_CONN_ON_NEW_TAB))
    {
        m_connectButton->choose();
        if (m_con && !m_con->isOpen())
            m_con->OpenConcurrent();
    }

    if(m_con && m_con->getType() == CONNECTION_SERIAL_PORT)
        sConfig.set(CFG_STRING_SHUPITO_PORT, m_con->GetIDString());
}

void LorrisProgrammer::vccValueChanged(quint8 id, double value)
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
void LorrisProgrammer::vddSetup(const vdd_setup &vs)
{
    m_vdd_setup = vs;

    ui->clearVCC();

    if(vs.empty())
        return;

    ui->vddSetup(vs);

    lastVccIndex = vs[0].current_drive;
}

void LorrisProgrammer::vddIndexChanged(int index)
{
    if(index == -1)
        return;

    lastVccIndex = index;

    if (m_programmer)
        m_programmer->setVddIndex(index);
}

void LorrisProgrammer::tunnelSpeedChanged(const QString &text)
{
    bool ok = false;
    quint32 speed = 0;
    speed = text.toInt(&ok);
    if(ok && m_programmer)
        m_programmer->setTunnelSpeed(speed);
}

void LorrisProgrammer::tunnelToggled(bool enable)
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

void LorrisProgrammer::tunnelStateChanged(bool opened)
{
    ui->tunnelStateChanged(opened);
    ui->log(tr("RS232 tunnel %1").arg(opened ? tr("enabled") : tr("disabled")));
}

void LorrisProgrammer::setTunnelName()
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

void LorrisProgrammer::modeSelected(int idx)
{
    Q_ASSERT(m_programmer);
    m_programmer->setMode(idx);

    for (size_t i = 1; i < m_mode_acts.size(); ++i)
        m_mode_acts[i-1]->setChecked(i-1 == (size_t)idx);
}

void LorrisProgrammer::progSpeedChanged(QString text)
{
    bool ok;
    quint32 speed = text.toInt(&ok);
    if(!ok)
        return;

    m_prog_speed_hz = speed;
    sConfig.set(CFG_QUINT32_SHUPITO_PRG_SPEED, m_prog_speed_hz);
}

void LorrisProgrammer::status(const QString &text)
{
    emit statusBarMsg(text, 5000);
}

bool LorrisProgrammer::checkVoltage(bool active)
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
void LorrisProgrammer::update_chip_description(chip_definition& cd)
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
            name += " flash: " % QString::number(mem->size/1024) % " kB, page: " %
                    QString::number(mem->pagesize) % " bytes";

        mem = cd.getMemDef("eeprom");
        if(mem)
            name += "  EEPROM: " % QString::number(mem->size) % " bytes";

        ui->setChipId(name);
        m_cur_def = cd;
    }
}

void LorrisProgrammer::showProgressDialog(const QString& text, QObject *sender)
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

void LorrisProgrammer::updateProgressDialog(int value)
{
    if(!m_progress_dialog)
        return;

    if(value == -1)
    {
        m_progress_dialog->close();
        delete m_progress_dialog;
        m_progress_dialog = NULL;
        disconnect(this, SLOT(updateProgressDialog(int)));
        disconnect(this, SLOT(updateProgressLabel(QString)));
        return;
    }
    // 100 closes the dialog, we don't want that
    else if(value >= 100)
        value = 99;
    m_progress_dialog->setValue(value);
}

void LorrisProgrammer::updateProgressLabel(const QString &text)
{
    if(!m_progress_dialog)
        return;
    m_progress_dialog->setLabelText(text);
}

chip_definition LorrisProgrammer::switchToFlashAndGetId()
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

bool LorrisProgrammer::showContinueBox(const QString &title, const QString &text)
{
    QMessageBox box(this);
    box.setWindowTitle(title);
    box.setText(text);
    box.addButton(tr("Yes"), QMessageBox::YesRole);
    box.addButton(tr("No"), QMessageBox::NoRole);
    box.setIcon(QMessageBox::Question);
    return !((bool)box.exec());
}

void LorrisProgrammer::startstopChip()
{
    if (m_chipStopped)
        startChip();
    else
        stopChip();
}

void LorrisProgrammer::startChip()
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

void LorrisProgrammer::stopChip()
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

void LorrisProgrammer::updateStartStopUi(bool stopped)
{
    ui->setStartStopBtn(stopped);

    m_start_act->setEnabled(m_buttons_enabled && stopped);
    m_stop_act->setEnabled(m_buttons_enabled && !stopped);

    m_chipStopped = stopped;
}

void LorrisProgrammer::restartChip()
{
    if(!checkVoltage(true))
        return;

    status("");

    stopChip();
    startChip();
}

QString LorrisProgrammer::getFileDialogFilter(int memid) {
    if(this->m_programmer && m_programmer->getType() == programmer_zmodem) {
        return QObject::tr("All files (*)");
    } else if(memid == MEM_JTAG) {
        return QObject::tr("Serial Vector Format file (*.svf)");
    } else {
        return QObject::tr("All supported file types (*.hex *.bin);;Intel HEX file (*.hex);;Binary file (*.bin)");;
    }
}

void LorrisProgrammer::loadFromFile()
{
    try
    {
        int memid = ui->getMemIndex();

        QString filename = QFileDialog::getOpenFileName(this, QObject::tr("Import data"),
                                                        sConfig.get(CFG_STRING_SHUPITO_HEX_FOLDER),
                                                        getFileDialogFilter(memid));
        if(filename.isEmpty())
            return;

        sConfig.set(CFG_STRING_SHUPITO_HEX_FOLDER, filename);

        loadFromFile(memid, filename);
    }
    catch(QString ex)
    {
        Utils::showErrorBox(ex);
    }
}

void LorrisProgrammer::loadFromFile(int memId, const QString& filename)
{
    status("");

    QDateTime loadTimestamp = QDateTime::currentDateTime();

    if (memId != MEM_JTAG)
    {
        HexFile file;
        if (filename.endsWith(".hex"))
            file.LoadFromFile(filename);
        else
            file.LoadFromBin(filename);

        quint32 len = 0;
        if(!m_cur_def.getName().isEmpty())
        {
            chip_definition::memorydef *memdef = m_cur_def.getMemDef(memId);
            if(memdef)
                len = memdef->size;
        }

        ui->setHexData(memId, file.getDataArray(len));
        ui->setHexColor(memId, colorFromFile);
    }
    else
    {
        QFile fin(filename);
        if (!fin.open(QIODevice::ReadOnly))
            throw QString(QObject::tr("Can't open file \"%1\"!")).arg(filename);
        ui->setHexData(MEM_JTAG, fin.readAll());
    }

    m_hexFilenames[memId] = filename;
    m_hexWriteTimes[memId] = loadTimestamp;

    if(memId == MEM_FLASH || memId == MEM_JTAG)
    {
        ui->setFileAndTime(filename, QFileInfo(filename).lastModified());
        ui->setFileNeverFlashed(true);
    }

    status(tr("File loaded"));
}

void LorrisProgrammer::saveToFile()
{
    try
    {
        int memId = ui->getMemIndex();
        QString filename = QFileDialog::getSaveFileName(this, QObject::tr("Export data"),
                                                        sConfig.get(CFG_STRING_SHUPITO_HEX_FOLDER),
                                                        getFileDialogFilter(memId));
        if(filename.isEmpty())
            return;

        sConfig.set(CFG_STRING_SHUPITO_HEX_FOLDER, filename);

        status("");

        if(filename.endsWith(".hex"))
        {
            HexFile file;
            file.setData(ui->getHexData(memId));
            file.SaveToFile(filename);
        }
        else
        {
            QFile f(filename);
            if(!f.open(QIODevice::WriteOnly))
                throw tr("Failed to open %1 for writing!").arg(filename);
            f.write((ui->getHexData(memId)));
        }

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


void LorrisProgrammer::verifyChanged(int mode)
{
    for(quint8 i = 0; i < VERIFY_MAX; ++i)
        m_verify[i]->setChecked(i == mode);

    sConfig.set(CFG_QUINT32_SHUPITO_VERIFY, mode);

    m_verify_mode = (VerifyMode)mode;
}

void LorrisProgrammer::tryFileReload(quint8 memId)
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
        emit statusBarMsg(tr("Failed to reload file %1").arg(m_hexFilenames[memId]), 10000);
    }
}

void LorrisProgrammer::focusChanged(QWidget *prev, QWidget */*curr*/)
{
    if(prev == NULL)
        tryFileReload(ui->getMemIndex());
}

void LorrisProgrammer::updateProgrammer()
{
    m_programmer.reset();
    if (m_con)
    {
        if (ConnectionPointer<ShupitoConnection> sc = m_con.dynamicCast<ShupitoConnection>())
        {
            m_programmer.reset(new ShupitoProgrammer(sc, &m_logsink));
        }
#ifdef HAVE_LIBYB
        else if(ConnectionPointer<STM32Connection> fc = m_con.dynamicCast<STM32Connection>())
        {
            m_programmer.reset(new STM32Programmer(fc, &m_logsink));
        }
        else if (ConnectionPointer<GenericUsbConnection> fc = m_con.dynamicCast<GenericUsbConnection>())
        {
            if (fc->isFlipDevice())
                m_programmer.reset(new FlipProgrammer(fc, &m_logsink));
        }
#endif
        else if(ConnectionPointer<PortConnection> con = m_con.dynamicCast<PortConnection>())
        {
            switch(con->programmerType())
            {
            case programmer_shupito:
                break; // morphed to ShupitoConnection in ChooseConnectionDlg::choose
            case programmer_avr232boot:
                m_programmer.reset(new avr232bootProgrammer(con, &m_logsink));
                break;
            case programmer_atsam:
                m_programmer.reset(new AtsamProgrammer(con, &m_logsink));
                break;
            case programmer_avr109:
                m_programmer.reset(new avr109Programmer(con, &m_logsink));
                break;
            case programmer_arduino:
                if(con->getType() == CONNECTION_SERIAL_PORT) {
                    m_programmer.reset(new ArduinoProgrammer(m_con.dynamicCast<SerialPort>(), &m_logsink));
                } else {
                    Utils::showErrorBox(tr("Arduino programmer only works with serial port connection!"));
                }
                break;
            case programmer_zmodem:
                m_programmer.reset(new ZmodemProgrammer(con, &m_logsink));
                break;
            default:
                break;
            }
        }
    }

    if (!m_programmer)
    {
        m_set_tunnel_name_act->setVisible(false);
        return;
    }

    this->updateModeBar();

    connect(m_programmer.data(), SIGNAL(buttonPressed(int)),              SLOT(buttonPressed(int)));
    connect(m_programmer.data(), SIGNAL(modesChanged()),                  SLOT(updateModeBar()));
    connect(m_programmer.data(), SIGNAL(vccValueChanged(quint8,double)),  SLOT(vccValueChanged(quint8,double)));
    connect(m_programmer.data(), SIGNAL(vddDesc(vdd_setup)),              SLOT(vddSetup(vdd_setup)));
    connect(m_programmer.data(), SIGNAL(tunnelStatus(bool)),              SLOT(tunnelStateChanged(bool)));

    connect(m_programmer.data(), SIGNAL(blinkLedSupport(bool)), m_blink_led, SLOT(setEnabled(bool)));
    m_blink_led->setEnabled(m_programmer->canBlinkLed());

    m_set_tunnel_name_act->setVisible(m_programmer->supportsTunnel());
    ui->connectProgrammer(m_programmer.data());
}

void LorrisProgrammer::setConnection(ConnectionPointer<Connection> const & con)
{
    if (m_con)
    {
        m_con->disconnect(this);
        this->disconnect(m_con.data());
        m_con->releaseTab();
    }

    emit setConnId(con ? con->GetIDString() : QString(), m_con != NULL);

    m_con = con;
    m_programmer.reset();

    if (m_con)
    {
        connect(m_con.data(), SIGNAL(connected(bool)), this, SLOT(connectedStatus(bool)));
        m_con->addTabRef();

        m_connectButton->setConn(m_con, false);
        connect(m_con.data(), SIGNAL(disconnecting()), this, SLOT(connDisconnecting()));

        PortConnection *port = dynamic_cast<PortConnection *>(m_con.data());
        if(port)
            connect(port, SIGNAL(programmerTypeChanged(int)), this, SLOT(updateProgrammer()));
    }

    this->connectedStatus(m_con && m_con->isOpen());
}

void LorrisProgrammer::checkOvervoltage()
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

void LorrisProgrammer::shutdownVcc()
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

void LorrisProgrammer::timeout()
{
    m_timeout_timer.stop();
    if(!m_timeout_warn)
    {
        m_timeout_warn = new ToolTipWarn(tr("Shupito is not responding, try to re-plug it into computer!"),
                                         m_connectButton->btn(), this, -1);
        Utils::playErrorSound();
    }
}

QString LorrisProgrammer::GetIdString()
{
    // Keep as LorrisShupito because of saved sessions
    return "LorrisShupito";
}

void LorrisProgrammer::saveData(DataFileParser *file)
{
    WorkTab::saveData(file);

    file->writeBlockIdentifier("LorrShupitoFiles");
    for(int i = 1; i < MEM_FUSES; ++i)
        file->writeString(m_hexFilenames[i]);

    ui->saveData(file);

    if(m_con)
    {
        if(ConnectionPointer<PortShupitoConnection> sc = m_con.dynamicCast<PortShupitoConnection>())
        {
            file->writeBlockIdentifier("LorrShupitoConn2");
            file->writeString("Shupito2");
            file->writeConn(sc->port().data());
            file->writeVal<qint64>(sc->getCompanionId(ShupitoTunnel::getCompanionName()));
        }
        else if(ConnectionPointer<PortConnection> con = m_con.dynamicCast<PortConnection>())
        {
            file->writeBlockIdentifier("LorrShupitoConn2");
            file->writeString("Port");
            file->writeConn(con.data());
        }
        else if(m_con->getType() == CONNECTION_SHUPITO23)
        {
            file->writeBlockIdentifier("LorrShupitoConn2");
            file->writeString("Shupito23");
            file->writeConn(m_con.data());
        }
        else if(m_con->getType() == CONNECTION_STM32)
        {
            file->writeBlockIdentifier("LorrShupitoConn2");
            file->writeString("STM32");
            file->writeConn(m_con.data());
        }
    }
}

void LorrisProgrammer::loadData(DataFileParser *file)
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

    if(file->seekToNextBlock("LorrShupitoConn2", BLOCK_WORKTAB))
    {
        quint8 type = 0;
        QHash<QString, QVariant> cfg;

        QString typeStr = file->readString();

        if(file->readConn(type, cfg))
        {
            ConnectionPointer<Connection> conn = sConMgr2.getConnWithConfig(type, cfg);
            if(conn.data())
            {
                ConnectionPointer<Connection> progConn;
                if(typeStr == "Shupito" || typeStr == "Shupito2")
                {
                    PortConnection *pc = (PortConnection*)conn.data();
                    ConnectionPointer<ShupitoConnection> sc = sConMgr2.createAutoShupito(pc);
                    progConn = sc;

                    if(typeStr == "Shupito2")
                        sc->setCompanionId(ShupitoTunnel::getCompanionName(), file->readVal<qint64>());
                }
                else if(typeStr == "Port" || typeStr == "Shupito23" || typeStr == "STM32")
                    progConn = conn;

                if(progConn.data())
                {
                    m_connectButton->setConn(progConn);
                    if(!progConn->isOpen() && sConfig.get(CFG_BOOL_SESSION_CONNECT))
                        progConn->OpenConcurrent();
                }
            }
        }
    }
}

void LorrisProgrammer::setEnableButtons(bool enable)
{
    if(enable == m_buttons_enabled)
        return;

    m_buttons_enabled = enable;
    m_start_act->setEnabled(enable);
    m_stop_act->setEnabled(enable);
    m_restart_act->setEnabled(enable);
    ui->enableButtons(enable);
}

void LorrisProgrammer::createConnBtn(QToolButton *btn)
{
    Q_ASSERT(!m_connectButton);

    m_connectButton = new ConnectButton(btn);
    m_connectButton->setConnectionTypes(pct_programmable);
    m_connectButton->setConn(m_con, false);
    connect(m_connectButton, SIGNAL(connectionChosen(ConnectionPointer<Connection>)), this, SLOT(setConnection(ConnectionPointer<Connection>)));
}

void LorrisProgrammer::setUiType(int type)
{
    if(ui && ui->getType() == type)
        return;

    QByteArray hexData[MEM_FUSES], svfData;
    if (ui)
    {
        for(quint8 i = MEM_FLASH; i < MEM_FUSES; ++i)
            hexData[i] = ui->getHexData(i);
        svfData = ui->getHexData(MEM_JTAG);
    }

    delete m_connectButton;
    m_connectButton = NULL;

    delete ui;

    ui = ProgrammerUI::createUI((ui_type)type, this);
    ui->setupUi(this);
    ui->vddSetup(m_vdd_setup);

    m_miniUi->setChecked(type == UI_MINIMAL);

    if(!m_hexFilenames[MEM_FLASH].isEmpty())
        ui->setFileAndTime(m_hexFilenames[MEM_FLASH], QFileInfo(m_hexFilenames[MEM_FLASH]).lastModified());

    for(quint8 i = MEM_FLASH; i < MEM_FUSES; ++i)
    {
        if(!hexData[i].isEmpty())
        {
            ui->setHexColor(i, colorFromFile);
            ui->setHexData(i, hexData[i]);
        }
    }
    ui->setHexData(MEM_JTAG, svfData);

    if (m_programmer)
        ui->connectProgrammer(m_programmer.data());

    updateStartStopUi(m_chipStopped);
    update_chip_description(m_cur_def);
}

void LorrisProgrammer::setMiniUi(bool mini)
{
    int type = mini ? UI_MINIMAL : UI_FULL;
    if(ui->getType() == type)
        return;

    QByteArray data;
    DataFileParser parser(&data, QIODevice::ReadWrite);

    ui->saveData(&parser);
    parser.seek(0);

    setUiType(type);

    ui->loadData(&parser);

    ui->connectedStatus(!(m_state & STATE_DISCONNECTED));
    ui->enableButtons(m_buttons_enabled);
}

void LorrisProgrammer::buttonPressed(int btnid)
{
    // FIXME: the button should be ignored if an action is in progress
    if (btnid == 0 && m_buttons_enabled && m_enableHardwareButton->isChecked())
        ui->writeSelectedMem();
}

void LorrisProgrammer::blinkLed()
{
    if (m_programmer)
        m_programmer->blinkLed();
}
