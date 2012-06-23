/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QTextEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QScrollBar>
#include <QLabel>
#include <QKeyEvent>
#include <QProgressDialog>
#include <QSignalMapper>

#include "lorristerminal.h"
#include "../shared/terminal.h"
#include "eeprom.h"
#include "../shared/hexfile.h"
#include "../shared/defmgr.h"
#include "../ui/ui_lorristerminal.h"
#include "../ui/chooseconnectiondlg.h"
#include "../ui/tooltipwarn.h"

LorrisTerminal::LorrisTerminal()
    : ui(new Ui::LorrisTerminal)
{
    stopCmd.resize(4);
    stopCmd[0] = 0x74;
    stopCmd[1] = 0x7E;
    stopCmd[2] = 0x7A;
    stopCmd[3] = 0x33;

    m_state = 0;
    stopTimer = NULL;
    hex = NULL;
    m_eeprom = NULL;

    initUI();
}

void LorrisTerminal::initUI()
{
    ui->setupUi(this);

    QMenu *eepromBar = new QMenu(tr("EEPROM"), this);
    addTopMenu(eepromBar);

    m_export_eeprom = eepromBar->addAction(tr("Export EEPROM"));
    m_import_eeprom = eepromBar->addAction(tr("Import EEPROM"));
    EnableButtons((BUTTON_EEPROM_READ |  BUTTON_EEPROM_WRITE), false);

    QMenu *fmtBar = new QMenu(tr("Format"), this);
    addTopMenu(fmtBar);

    QSignalMapper *fmtMap = new QSignalMapper(this);
    quint32 fmt = sConfig.get(CFG_QUINT32_TERMINAL_FMT);
    for(quint8 i = 0; i < FMT_MAX; ++i)
    {
        static const QString fmtText[] = { tr("Text"), tr("Hex dump") };

        m_fmt_act[i] = fmtBar->addAction(fmtText[i]);
        m_fmt_act[i]->setCheckable(true);
        m_fmt_act[i]->setChecked(i == fmt);
        fmtMap->setMapping(m_fmt_act[i], i);
        connect(m_fmt_act[i], SIGNAL(triggered()), fmtMap, SLOT(map()));
    }

    fmtAction(fmt);

    QMenu *dataMenu = new QMenu(tr("Terminal"), this);
    addTopMenu(dataMenu);
    QAction *termLoad = dataMenu->addAction(tr("Load text file into terminal"));

    dataMenu->addSeparator();

    QAction *termSave = dataMenu->addAction(tr("Save terminal content to text file"));
    QAction *binSave = dataMenu->addAction(tr("Save received data to binary file"));
    termSave->setShortcut(QKeySequence("Ctrl+Shift+S"));
    binSave->setShortcut(QKeySequence("Ctrl+S"));

    dataMenu->addSeparator();

    QMenu *inputMenu = new QMenu(tr("Input handling"), this);
    addTopMenu(inputMenu);
    QSignalMapper *inputMap = new QSignalMapper(this);
    for(quint8 i = 0; i < INPUT_MAX; ++i)
    {
        static const QString inputText[] = { tr("Just send key presses"), tr("TCP-terminal-like") };
        static const QString inputTip[] =
        {
            tr("Send key code immediately after press"),
            tr("Show pressed keys in terminal and send after pressing return")
        };

        m_input[i] = inputMenu->addAction(inputText[i]);
        m_input[i]->setStatusTip(inputTip[i]);
        m_input[i]->setCheckable(true);
        inputMap->setMapping(m_input[i], i);
        connect(m_input[i], SIGNAL(triggered()), inputMap, SLOT(map()));
    }

    QAction *chgSettings = dataMenu->addAction(tr("Change settings..."));

    QAction *bootloaderAct = new QAction(tr("Show bootloader controls"), this);
    bootloaderAct->setCheckable(true);
    addAction(bootloaderAct);
    dataMenu->addAction(bootloaderAct);

    QAction *showWarnAct = new QAction(tr("Show warn when flashing the same file twice"), this);
    showWarnAct->setCheckable(true);
    addAction(showWarnAct);

    showBootloader(sConfig.get(CFG_BOOL_TERMINAL_SHOW_BOOTLOADER));
    showWarn(sConfig.get(CFG_BOOL_TERMINAL_SHOW_WARN));

    inputAct(sConfig.get(CFG_QUINT32_TERMINAL_INPUT));

    setHexName(sConfig.get(CFG_STRING_HEX_FOLDER));
    ui->terminal->loadSettings(sConfig.get(CFG_STRING_TERMINAL_SETTINGS));
    ui->progressBar->hide();

    connect(inputMap,          SIGNAL(mapped(int)),                 SLOT(inputAct(int)));
    connect(fmtMap,            SIGNAL(mapped(int)),                 SLOT(fmtAction(int)));
    connect(ui->terminal,      SIGNAL(keyPressed(QString)),         SLOT(sendKeyEvent(QString)));
    connect(ui->browseBtn,     SIGNAL(clicked()),                   SLOT(browseForHex()));
    connect(ui->stopButton,    SIGNAL(clicked()),                   SLOT(stopButton()));
    connect(ui->flashButton,   SIGNAL(clicked()),                   SLOT(flashButton()));
    connect(ui->pauseButton,   SIGNAL(clicked()),                   SLOT(pauseButton()));
    connect(ui->clearButton,   SIGNAL(clicked()),     ui->terminal, SLOT(clear()));
    connect(ui->terminal,      SIGNAL(settingsChanged()),           SLOT(saveTermSettings()));
    connect(ui->fmtBox,        SIGNAL(activated(int)),              SLOT(fmtAction(int)));
    connect(m_export_eeprom,   SIGNAL(triggered()),                 SLOT(eepromButton()));
    connect(m_import_eeprom,   SIGNAL(triggered()),                 SLOT(eepromImportButton()));
    connect(termLoad,          SIGNAL(triggered()),                 SLOT(loadText()));
    connect(termSave,          SIGNAL(triggered()),                 SLOT(saveText()));
    connect(binSave,           SIGNAL(triggered()),                 SLOT(saveBin()));
    connect(chgSettings,       SIGNAL(triggered()),   ui->terminal, SLOT(showSettings()));
    connect(bootloaderAct,     SIGNAL(triggered(bool)),             SLOT(showBootloader(bool)));
    connect(showWarnAct,       SIGNAL(triggered(bool)),             SLOT(showWarn(bool)));
    connect(ui->terminal,      SIGNAL(fmtSelected(int)),            SLOT(checkFmtAct(int)));
    connect(ui->terminal,      SIGNAL(paused(bool)),                SLOT(setPauseBtnText(bool)));

    m_connectButton = new ConnectButton(ui->connectButton2);
    connect(m_connectButton, SIGNAL(connectionChosen(PortConnection*)), this, SLOT(setConnection(PortConnection*)));
}

LorrisTerminal::~LorrisTerminal()
{
    delete ui;
}

void LorrisTerminal::onTabShow(const QString&)
{
    this->connectedStatus(m_con && m_con->isOpen());

    if (!m_con)
    {
        m_connectButton->choose();
        if (m_con && !m_con->isOpen())
            m_con->OpenConcurrent();
    }
}

void LorrisTerminal::browseForHex()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    sConfig.get(CFG_STRING_HEX_FOLDER),
                                                    tr("Intel hex file (*.hex)"));
    if(filename.isEmpty())
        return;

    setHexName(filename);
    sConfig.set(CFG_STRING_HEX_FOLDER, filename);
}

void LorrisTerminal::pauseButton()
{
    m_state ^= STATE_PAUSED;
    ui->terminal->pause(m_state & STATE_PAUSED);
}

void LorrisTerminal::setPauseBtnText(bool pause)
{
    if(pause)
        ui->pauseButton->setText(tr("Unpause"));
    else
        ui->pauseButton->setText(tr("Pause"));
}

void LorrisTerminal::eepromButton()
{
    m_state |= (STATE_EEPROM_READ | STATE_AWAITING_ID);

    flashTimeoutTimer = new QTimer();
    connect(flashTimeoutTimer, SIGNAL(timeout()), this, SLOT(deviceIdTimeout()));
    flashTimeoutTimer->start(1500);

    QByteArray data;
    data[0] = 0x12;
    m_con->SendData(data);

    EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), false);
}

void LorrisTerminal::eepromImportButton()
{
    m_state |= (STATE_EEPROM_WRITE | STATE_AWAITING_ID);
    flashTimeoutTimer = new QTimer();
    connect(flashTimeoutTimer, SIGNAL(timeout()), this, SLOT(deviceIdTimeout()));
    flashTimeoutTimer->start(1500);

    QByteArray data;
    data[0] = 0x12;
    m_con->SendData(data);

    EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), false);
}

void LorrisTerminal::eeprom_write(QString id)
{
    delete flashTimeoutTimer;
    flashTimeoutTimer = NULL;

    chip_definition cd = sDefMgr.findChipdef("avr232boot:" + id);
    if(cd.getName().isEmpty() || !cd.getMemDef(MEM_EEPROM))
    {
        m_state &= ~(STATE_EEPROM_WRITE);

        Utils::ThrowException(tr("Unsupported chip: ") + id);

        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);
        return;
    }

    m_eeprom = new EEPROM(this, cd);
    if(!m_eeprom->Import())
    {
        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);
        return;
    }

    ui->progressBar->show();
    ui->progressBar->setMaximum(m_eeprom->GetEEPROMSize());

    flashTimeoutTimer = new QTimer();
    connect(flashTimeoutTimer, SIGNAL(timeout()), this, SLOT(flashTimeout()));
    flashTimeoutTimer->start(1500);

    eeprom_send_page();
}

bool LorrisTerminal::eeprom_send_page()
{
    page *p = m_eeprom->getNextPage();

    if(!p)
    {
        delete flashTimeoutTimer;
        flashTimeoutTimer = NULL;
        delete m_eeprom;

        ui->progressBar->setValue(0);
        ui->progressBar->hide();
        m_state &= ~(STATE_EEPROM_WRITE);
        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);
        return false;
    }

    flashTimeoutTimer->start(1500);
    ui->progressBar->setValue(p->address);

    QByteArray data;
    data[0] = 0x14;
    m_con->SendData(data);

    data[0] = (p->address >> 8);
    data[1] = quint8(p->address);
    data[2] = 2;
    m_con->SendData(data);

    data.clear();
    for(quint8 i = 0; i < p->data.size(); ++i)
        data[i] = p->data[i];
    m_con->SendData(data);
    return true;
}

void LorrisTerminal::eeprom_read(QString id)
{
    delete flashTimeoutTimer;
    flashTimeoutTimer = NULL;

    chip_definition cd = sDefMgr.findChipdef("avr232boot:" + id);
    if(cd.getName().isEmpty() || !cd.getMemDef(MEM_EEPROM))
    {
        m_state &= ~(STATE_EEPROM_READ);

        Utils::ThrowException(tr("Unsupported chip: ") + id);

        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);
        return;
    }
    m_eepromItr = 0;

    QByteArray data;
    data[0] = 0x13;
    m_con->SendData(data);

    data[0] = (m_eepromItr >> 8);
    data[1] = quint8(m_eepromItr);
    data[2] = 128;
    m_con->SendData(data);

    m_eeprom = new EEPROM(this, cd);

    ui->progressBar->show();
    ui->progressBar->setMaximum(m_eeprom->GetEEPROMSize());

    flashTimeoutTimer = new QTimer();
    connect(flashTimeoutTimer, SIGNAL(timeout()), this, SLOT(flashTimeout()));
    flashTimeoutTimer->start(1500);
}

void LorrisTerminal::eeprom_read_block(QByteArray data)
{
    flashTimeoutTimer->start(1500);
    m_eepromItr += data.count();
    m_eeprom->AddData(data);

    if(m_eepromItr >= m_eeprom->GetEEPROMSize())
    {
        delete flashTimeoutTimer;
        flashTimeoutTimer = NULL;

        ui->progressBar->setValue(0);
        ui->progressBar->hide();

        m_state &= ~(STATE_EEPROM_READ);
        m_eeprom->Export();
        delete m_eeprom;

        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);
        return;
    }
    ui->progressBar->setValue(m_eepromItr);

    if(m_eepromItr%128 == 0)
    {
        QByteArray dataSend;
        dataSend[0] = 0x13;
        m_con->SendData(dataSend);

        dataSend[0] = (m_eepromItr >> 8);
        dataSend[1] = quint8(m_eepromItr);
        dataSend[2] = 128;
        m_con->SendData(dataSend);
    }
}

void LorrisTerminal::connectedStatus(bool connected)
{
    if(connected)
    {
        m_state &= ~(STATE_DISCONNECTED);

        ui->stopButton->setEnabled(true);
        ui->stopButton->setText(tr("Stop"));

        ui->terminal->setFocus();
    }
    else
    {
        m_state |= STATE_DISCONNECTED;
        m_state &= ~(STATE_STOPPING1 | STATE_STOPPING2 | STATE_STOPPED);

        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), false);
    }
}

void LorrisTerminal::connectionResult(Connection */*con*/,bool result)
{
    disconnect(m_con, SIGNAL(connectResult(Connection*,bool)), this, 0);

    if(!result)
    {
        Utils::ThrowException(tr("Can't open serial port!"));
    }
}

void LorrisTerminal::readData(const QByteArray& data)
{
    if((m_state & STATE_STOPPING1) && data[0] == char(20))
    {
        if(stopTimer)
        {
            delete stopTimer;
            stopTimer = NULL;
        }
        stopTimer = new QTimer(this);
        connect(stopTimer, SIGNAL(timeout()), this, SLOT(stopTimerSig()));
        stopTimer->start(500);

        m_state &= ~(STATE_STOPPING1);
        m_state |= STATE_STOPPING2;

        m_con->SendData(stopCmd);
        return;
    }
    else if((m_state & STATE_STOPPING2) && data[0] == char(20))
    {
        m_state &= ~(STATE_STOPPING2);
        m_state |= STATE_STOPPED;
        if(stopTimer)
        {
            delete stopTimer;
            stopTimer = NULL;
        }
        ui->stopButton->setText(tr("Start"));
        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);
        return;
    }
    else if(m_state & STATE_AWAITING_ID)
    {
        m_state &= ~(STATE_AWAITING_ID);
        if(m_state & STATE_EEPROM_READ)
            eeprom_read(QString(data));
        else if(m_state & STATE_EEPROM_WRITE)
            eeprom_write(QString(data));
        else
            flash_prepare(QString(data));
        return;
    }
    else if(m_state & STATE_FLASHING && data[0] == char(20))
    {
        if(!SendNextPage())
            m_state &= ~(STATE_FLASHING);
        return;
    }
    else if(m_state & STATE_EEPROM_WRITE && data[0] == char(20))
    {
        eeprom_send_page();
        return;
    }
    else if(m_state & STATE_EEPROM_READ)
    {
        eeprom_read_block(data);
        return;
    }

    if(!ui->terminal)
        return;

    ui->terminal->appendText(data);
}

void LorrisTerminal::stopButton()
{
    if(m_state & STATE_STOPPED)
    {
        QByteArray data;
        data[0] = 0x11;
        m_con->SendData(data);

        ui->stopButton->setText(tr("Stop"));
        m_state &= ~(STATE_STOPPED);

        EnableButtons((BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), false);

        ui->terminal->setFocus();
    }
    else
    {
        m_state |= STATE_STOPPING1;
        ui->stopButton->setText(tr("Stopping.."));
        ui->stopButton->setEnabled(false);

        m_con->SendData(stopCmd);

        stopTimer = new QTimer(this);
        connect(stopTimer, SIGNAL(timeout()), this, SLOT(stopTimerSig()));
        stopTimer->start(500);
    }
}

void LorrisTerminal::stopTimerSig()
{
    delete stopTimer;
    stopTimer = NULL;
    if(m_state & STATE_STOPPING1)
    {
        stopTimer = new QTimer(this);
        connect(stopTimer, SIGNAL(timeout()), this, SLOT(stopTimerSig()));
        stopTimer->start(500);

        m_state &= ~(STATE_STOPPING1);
        m_state |= STATE_STOPPING2;
        m_con->SendData(stopCmd);
    }
    else if(m_state & STATE_STOPPING2)
    {
        m_state &= ~(STATE_STOPPING2);
        ui->stopButton->setText(tr("Stop"));
        ui->stopButton->setEnabled(true);

        Utils::ThrowException(tr("Timeout on stopping chip!"));
    }
}

void LorrisTerminal::flashButton()
{
    if(hex) delete hex;
    hex = new HexFile();

    setHexName();

    if(actions()[1]->isChecked() && m_filedate.isValid() && m_filedate == m_flashdate)
        new ToolTipWarn(tr("You have flashed this file already, and it was not changed since."), ui->flashButton, this);

    try
    {
        hex->LoadFromFile(m_filename);
    }
    catch(QString ex)
    {
        Utils::ThrowException(tr("Error loading hex file: ") + ex);

        delete hex;
        hex = NULL;
        return;
    }

    m_flashdate = m_filedate;

    EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), false);

    flashTimeoutTimer = new QTimer();
    connect(flashTimeoutTimer, SIGNAL(timeout()), this, SLOT(deviceIdTimeout()));
    flashTimeoutTimer->start(1500);

    QByteArray data;
    data[0] = 0x12;
    m_con->SendData(data);
    m_state |= STATE_AWAITING_ID;
}

void LorrisTerminal::flash_prepare(QString deviceId)
{
    delete flashTimeoutTimer;
    flashTimeoutTimer = NULL;

    chip_definition cd = sDefMgr.findChipdef("avr232boot:" + deviceId);
    if(cd.getName().isEmpty())
    {
        Utils::ThrowException(tr("Unsupported chip: ") + deviceId);
        delete hex;
        hex = NULL;

        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);
        return;
    }

    try
    {
        m_cur_page = 0;
        m_pages.clear();
        hex->makePages(m_pages, MEM_FLASH, cd, NULL);
    }
    catch(QString ex)
    {
        Utils::ThrowException(tr("Error making pages: ") + ex);

        delete hex;
        hex = NULL;

        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);
        return;
    }

    ui->progressBar->show();
    ui->progressBar->setMaximum(m_pages.size());

    ui->flashText->setText(tr("Flashing into ") + cd.getName() + "...");

    flashTimeoutTimer = new QTimer();
    connect(flashTimeoutTimer, SIGNAL(timeout()), this, SLOT(flashTimeout()));
    flashTimeoutTimer->start(1500);

    m_state |= STATE_FLASHING;
    SendNextPage();
}

bool LorrisTerminal::SendNextPage()
{
    flashTimeoutTimer->start(1500);

    if(m_cur_page >= m_pages.size())
    {
        ui->progressBar->setValue(0);
        ui->progressBar->hide();

        ui->flashText->setText("");

        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);

        delete hex;
        delete flashTimeoutTimer;
        hex = NULL;
        flashTimeoutTimer = NULL;
        m_pages.clear();
        return false;
    }

    page& p = m_pages[m_cur_page++];
    QByteArray data;
    data[0] = 0x10;
    m_con->SendData(data);

    data[0] = quint8(p.address >> 8);
    data[1] = quint8(p.address);
    m_con->SendData(data);

    for(quint16 i = 0; i < p.data.size(); ++i)
        data[i] = p.data[i];
    m_con->SendData(data);
    ui->progressBar->setValue(ui->progressBar->value()+1);
    return true;
}

void LorrisTerminal::flashTimeout()
{
    flashTimeoutTimer->stop();
    delete flashTimeoutTimer;
    flashTimeoutTimer = NULL;

    EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);

    ui->progressBar->setValue(0);
    ui->progressBar->hide();

    if(m_state & STATE_EEPROM_READ)
    {
        m_state &= ~(STATE_EEPROM_READ);
        delete m_eeprom;

        Utils::ThrowException(tr("Timeout during reading EEPROM!"));
        return;
    }

    if(m_state & STATE_EEPROM_WRITE)
    {
        m_state &= ~(STATE_EEPROM_WRITE);
        delete m_eeprom;

        Utils::ThrowException(tr("Timeout during writing EEPROM!"));
        return;
    }

    delete hex;
    hex = NULL;

    m_state &= ~(STATE_FLASHING);

    ui->flashText->setText("");

    Utils::ThrowException(tr("Timeout during flashing!"));
}

void LorrisTerminal::deviceIdTimeout()
{
    m_state &= ~(STATE_AWAITING_ID);

    delete flashTimeoutTimer;
    flashTimeoutTimer = NULL;

    Utils::ThrowException(tr("Can't get device id!"));

    EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);

    if(m_state & STATE_EEPROM_READ)
    {
        m_state &= ~(STATE_EEPROM_READ);
        return;
    }

    if(m_state & STATE_EEPROM_WRITE)
    {
        m_state &= ~(STATE_EEPROM_WRITE);
        return;
    }

    delete hex;
    hex = NULL;
}

void LorrisTerminal::sendKeyEvent(const QString &key)
{
    if(m_con && m_con->isOpen())
        m_con->SendData(key.toUtf8());
}

void LorrisTerminal::EnableButtons(quint16 buttons, bool enable)
{
    if(buttons & BUTTON_STOP)
        ui->stopButton->setEnabled(enable);

    if(buttons & BUTTON_FLASH)
        ui->flashButton->setEnabled(enable);

    if(buttons & BUTTON_EEPROM_READ)
        m_export_eeprom->setEnabled(enable);

    if(buttons & BUTTON_EEPROM_WRITE)
        m_import_eeprom->setEnabled(enable);
}

void LorrisTerminal::fmtAction(int act)
{
    if(ui->terminal->getFmt() == act)
        return;

    sConfig.set(CFG_QUINT32_TERMINAL_FMT, act);
    ui->terminal->setFmt(act);
}

void LorrisTerminal::checkFmtAct(int act)
{
    for(quint8 i = 0; i < FMT_MAX; ++i)
        m_fmt_act[i]->setChecked(i == act);
    ui->fmtBox->setCurrentIndex(act);
}

void LorrisTerminal::loadText()
{
    static const QString filters = tr("Text file (*.txt);;Any file (*.*)");
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    sConfig.get(CFG_STRING_TERMINAL_TEXTFILE),
                                                    filters);
    if(filename.isEmpty())
        return;

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Utils::ThrowException(tr("Can't open file \"%1\"!").arg(filename), this);
        return;
    }

    ui->terminal->appendText(file.readAll());
    file.close();

    sConfig.set(CFG_STRING_TERMINAL_TEXTFILE, filename);
}

void LorrisTerminal::saveText()
{
    static const QString filters = tr("Text file (*.txt);;Any file (*.*)");
    QString filename = QFileDialog::getSaveFileName(this, tr("Save text data"),
                                                    sConfig.get(CFG_STRING_TERMINAL_TEXTFILE), filters);

    if(filename.isEmpty())
        return;

    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        Utils::ThrowException(tr("Can't open/create file \"%1\"!").arg(filename), this);
        return;
    }

    ui->terminal->writeToFile(&file);
    file.close();

    sConfig.set(CFG_STRING_TERMINAL_TEXTFILE, filename);
}

void LorrisTerminal::saveBin()
{
    static const QString filters = tr("Any file (*.*)");
    QString filename = QFileDialog::getSaveFileName(this, tr("Save binary data"),
                                                    sConfig.get(CFG_STRING_TERMINAL_TEXTFILE), filters);

    if(filename.isEmpty())
        return;

    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly))
    {
        Utils::ThrowException(tr("Can't open/create file \"%1\"!").arg(filename), this);
        return;
    }
    file.write(ui->terminal->getData());
    file.close();

    sConfig.set(CFG_STRING_TERMINAL_TEXTFILE, filename);
}

void LorrisTerminal::inputAct(int act)
{
    for(quint8 i = 0; i < INPUT_MAX; ++i)
        m_input[i]->setChecked(i == act);

    sConfig.set(CFG_QUINT32_TERMINAL_INPUT, act);
    ui->terminal->setInput(act);
}

void LorrisTerminal::setConnection(PortConnection *con)
{
    this->PortConnWorkTab::setConnection(con);
    m_connectButton->setConn(con);
    connectedStatus(con && con->isOpen());
}

void LorrisTerminal::saveTermSettings()
{
    sConfig.set(CFG_STRING_TERMINAL_SETTINGS, ui->terminal->getSettingsData());
}

void LorrisTerminal::showBootloader(bool show)
{
    ui->stopButton->setVisible(show);
    ui->flashButton->setVisible(show);
    ui->fileDate->setVisible(show);
    ui->fileName->setVisible(show);
    ui->browseBtn->setVisible(show);
    actions()[0]->setChecked(show);
    sConfig.set(CFG_BOOL_TERMINAL_SHOW_BOOTLOADER, show);
}

void LorrisTerminal::setHexName(QString name)
{
    if(!name.isNull())
        m_filename = name;

    QFileInfo info(m_filename);
    m_filedate = info.lastModified();

    ui->fileName->setText(m_filename);
    ui->fileName->setToolTip(m_filename);
    ui->fileDate->setText(m_filedate.toString(tr(" | h:mm:ss d.M.yyyy")));
    ui->fileDate->setToolTip(ui->fileDate->text());
}

void LorrisTerminal::showWarn(bool show)
{
    actions()[1]->setChecked(show);
    sConfig.set(CFG_BOOL_TERMINAL_SHOW_WARN, show);
}
