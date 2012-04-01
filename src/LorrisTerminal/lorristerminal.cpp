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

#include <QTextEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QScrollBar>
#include <QProgressBar>
#include <QLabel>
#include <QKeyEvent>
#include <QProgressDialog>
#include <QSignalMapper>

#include "lorristerminal.h"
#include "terminal.h"
#include "eeprom.h"
#include "shared/hexfile.h"
#include "shared/chipdefs.h"
#include "ui_lorristerminal.h"

LorrisTerminal::LorrisTerminal() : WorkTab(), ui(new Ui::LorrisTerminal)
{
    stopCmd.resize(4);
    stopCmd[0] = 0x74;
    stopCmd[1] = 0x7E;
    stopCmd[2] = 0x7A;
    stopCmd[3] = 0x33;

    m_state = 0;
    stopTimer = NULL;
    hex = NULL;
    terminal = NULL;
    m_eeprom = NULL;

    chip_definition::parse_default_chipsets(m_chip_defs);

    initUI();
}

void LorrisTerminal::initUI()
{
    ui->setupUi(this);

    terminal = new Terminal(this);
    ui->mainLayout->addWidget(terminal, 4);

    QMenu *eepromBar = new QMenu(tr("EEPROM"), this);
    addTopMenu(eepromBar);

    m_export_eeprom = eepromBar->addAction(tr("Export EEPROM"));
    m_import_eeprom = eepromBar->addAction(tr("Import EEPROM"));
    EnableButtons((BUTTON_EEPROM_READ |  BUTTON_EEPROM_WRITE), false);

    QMenu *fmtBar = new QMenu(tr("Format"), this);
    addTopMenu(fmtBar);

    QSignalMapper *fmtMap = new QSignalMapper(this);
    for(quint8 i = 0; i < FMT_MAX; ++i)
    {
        static const QString fmtText[] = { tr("Text"), tr("Hex dump") };

        m_fmt_act[i] = fmtBar->addAction(fmtText[i]);
        m_fmt_act[i]->setCheckable(true);
        fmtMap->setMapping(m_fmt_act[i], i);
        connect(m_fmt_act[i], SIGNAL(triggered()), fmtMap, SLOT(map()));
    }

    fmtAction(sConfig.get(CFG_QUINT32_TERMINAL_FMT));

    QMenu *dataMenu = new QMenu(tr("Terminal"), this);
    addTopMenu(dataMenu);
    QAction *termLoad = dataMenu->addAction(tr("Load text file into terminal"));
    QAction *termSave = dataMenu->addAction(tr("Save terminal content to text file"));
    termSave->setShortcut(QKeySequence("Ctrl+S"));

    dataMenu->addSeparator();

    QMenu *inputMenu = dataMenu->addMenu(tr("Input handling"));
    QSignalMapper *inputMap = new QSignalMapper(this);
    for(quint8 i = 0; i < INPUT_MAX; ++i)
    {
        static const QString inputText[] = { tr("Just send key presses"), tr("TCP-terminal-like") };

        m_input[i] = inputMenu->addAction(inputText[i]);
        m_input[i]->setCheckable(true);
        inputMap->setMapping(m_input[i], i);
        connect(m_input[i], SIGNAL(triggered()), inputMap, SLOT(map()));
    }

    inputAct(sConfig.get(CFG_QUINT32_TERMINAL_INPUT));

    connect(inputMap,          SIGNAL(mapped(int)),                 SLOT(inputAct(int)));
    connect(fmtMap,            SIGNAL(mapped(int)),                 SLOT(fmtAction(int)));
    connect(terminal,          SIGNAL(keyPressedASCII(QByteArray)), SLOT(sendKeyEvent(QByteArray)));
    connect(ui->browseBtn,     SIGNAL(clicked()),                   SLOT(browseForHex()));
    connect(ui->connectButton, SIGNAL(clicked()),                   SLOT(connectButton()));
    connect(ui->stopButton,    SIGNAL(clicked()),                   SLOT(stopButton()));
    connect(ui->flashButton,   SIGNAL(clicked()),                   SLOT(flashButton()));
    connect(ui->pauseButton,   SIGNAL(clicked()),                   SLOT(pauseButton()));
    connect(ui->clearButton,   SIGNAL(clicked()),                   SLOT(clearButton()));
    connect(m_export_eeprom,   SIGNAL(triggered()),                 SLOT(eepromButton()));
    connect(m_import_eeprom,   SIGNAL(triggered()),                 SLOT(eepromImportButton()));
    connect(termLoad,          SIGNAL(triggered()),                 SLOT(loadText()));
    connect(termSave,          SIGNAL(triggered()),                 SLOT(saveText()));
}

LorrisTerminal::~LorrisTerminal()
{
    delete terminal;
    delete ui;
}

void LorrisTerminal::browseForHex()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    sConfig.get(CFG_STRING_HEX_FOLDER),
                                                    tr("Intel hex file (*.hex)"));
    ui->hexFile->setText(filename);
    if(filename.length() != 0)
        sConfig.set(CFG_STRING_HEX_FOLDER, filename.left(filename.lastIndexOf(QRegExp("[\\/]"))));
}

void LorrisTerminal::clearButton()
{
    terminal->clear();
}

void LorrisTerminal::pauseButton()
{
    if(!(m_state & STATE_PAUSED))
    {
        m_state |= STATE_PAUSED;
        ui->pauseButton->setText(tr("Unpause"));
    }
    else
    {
        m_state &= ~(STATE_PAUSED);
        ui->pauseButton->setText(tr("Pause"));
    }

    terminal->pause(m_state & STATE_PAUSED);
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

    chip_definition cd;
    cd.setSign("avr232boot:" + id);
    chip_definition::update_chipdef(m_chip_defs, cd);

    if(cd.getName().isEmpty() || !cd.getMemDef(MEM_EEPROM))
    {
        m_state &= ~(STATE_EEPROM_WRITE);

        showErrorBox(tr("Unsupported chip: ") + id);

        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);
        return;
    }

    m_eeprom = new EEPROM(this, cd);
    if(!m_eeprom->Import())
    {
        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);
        return;
    }

    QProgressBar *bar = new QProgressBar(this);
    bar->setMaximum(m_eeprom->GetEEPROMSize());
    bar->setObjectName("FlashProgress");
    ui->mainLayout->insertWidget(1, bar);

    flashTimeoutTimer = new QTimer();
    connect(flashTimeoutTimer, SIGNAL(timeout()), this, SLOT(flashTimeout()));
    flashTimeoutTimer->start(1500);

    eeprom_send_page();
}

bool LorrisTerminal::eeprom_send_page()
{
    page *p = m_eeprom->getNextPage();
    QProgressBar *bar = findChild<QProgressBar *>("FlashProgress");
    if(!p)
    {
        delete flashTimeoutTimer;
        flashTimeoutTimer = NULL;
        ui->mainLayout->removeWidget(bar);
        delete bar;
        delete m_eeprom;

        m_state &= ~(STATE_EEPROM_WRITE);
        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);
        return false;
    }

    flashTimeoutTimer->start(1500);
    bar->setValue(p->address);

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

    chip_definition cd;
    cd.setSign("avr232boot:" + id);
    chip_definition::update_chipdef(m_chip_defs, cd);

    if(cd.getName().isEmpty() || !cd.getMemDef(MEM_EEPROM))
    {
        m_state &= ~(STATE_EEPROM_READ);

        showErrorBox(tr("Unsupported chip: ") + id);

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

    QProgressBar *bar = new QProgressBar(this);
    bar->setMaximum(m_eeprom->GetEEPROMSize());
    bar->setObjectName("FlashProgress");
    ui->mainLayout->insertWidget(1, bar);

    flashTimeoutTimer = new QTimer();
    connect(flashTimeoutTimer, SIGNAL(timeout()), this, SLOT(flashTimeout()));
    flashTimeoutTimer->start(1500);
}

void LorrisTerminal::eeprom_read_block(QByteArray data)
{
    flashTimeoutTimer->start(1500);
    m_eepromItr += data.count();
    m_eeprom->AddData(data);

    QProgressBar *bar = findChild<QProgressBar *>("FlashProgress");

    if(m_eepromItr >= m_eeprom->GetEEPROMSize())
    {
        delete flashTimeoutTimer;
        flashTimeoutTimer = NULL;

        ui->mainLayout->removeWidget(bar);
        delete bar;

        m_state &= ~(STATE_EEPROM_READ);
        m_eeprom->Export();
        delete m_eeprom;

        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);
        return;
    }
    bar->setValue(m_eepromItr);

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

void LorrisTerminal::connectButton()
{
    if(!(m_state & STATE_DISCONNECTED))
        m_con->Close();
    else
    {
        ui->connectButton->setText(tr("Connecting..."));
        ui->connectButton->setEnabled(false);

        connect(m_con, SIGNAL(connectResult(Connection*,bool)), this, SLOT(connectionResult(Connection*,bool)));
        m_con->OpenConcurrent();
    }
}

void LorrisTerminal::connectedStatus(bool connected)
{
    if(connected)
    {
        m_state &= ~(STATE_DISCONNECTED);
        ui->connectButton->setText(tr("Disconnect"));

        ui->stopButton->setEnabled(true);
        ui->stopButton->setText(tr("Stop"));

        terminal->setFocus();
    }
    else
    {
        m_state |= STATE_DISCONNECTED;
        m_state &= ~(STATE_STOPPING1 | STATE_STOPPING2 | STATE_STOPPED);

        ui->connectButton->setText(tr("Connect"));

        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), false);
    }
}

void LorrisTerminal::connectionResult(Connection */*con*/,bool result)
{
    disconnect(m_con, SIGNAL(connectResult(Connection*,bool)), this, 0);

    ui->connectButton->setEnabled(true);

    if(!result)
    {
        ui->connectButton->setText(tr("Connect"));

        showErrorBox(tr("Can't open serial port!"));
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

    if(!terminal)
        return;

    terminal->appendText(data);
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

        terminal->setFocus();
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

        showErrorBox(tr("Timeout on stopping chip!"));
    }
}

void LorrisTerminal::flashButton()
{
    if(hex) delete hex;
    hex = new HexFile();
    try
    {
        hex->LoadFromFile(ui->hexFile->text());
    }
    catch(QString ex)
    {
        showErrorBox(tr("Error loading hex file: ") + ex);

        delete hex;
        hex = NULL;
        return;
    }

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

    chip_definition cd;
    cd.setSign("avr232boot:" + deviceId);
    chip_definition::update_chipdef(m_chip_defs, cd);

    if(cd.getName().isEmpty())
    {
        showErrorBox(tr("Unsupported chip: ") + deviceId);
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
        showErrorBox(tr("Error making pages: ") + ex);

        delete hex;
        hex = NULL;

        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);
        return;
    }

    QProgressBar *bar = new QProgressBar(this);
    bar->setMaximum(m_pages.size());
    bar->setObjectName("FlashProgress");
    ui->mainLayout->insertWidget(1, bar);

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
    QProgressBar *bar = findChild<QProgressBar *>("FlashProgress");

    if(m_cur_page >= m_pages.size())
    {
        ui->mainLayout->removeWidget(bar);
        delete bar;

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
    bar->setValue(bar->value()+1);
    return true;
}

void LorrisTerminal::flashTimeout()
{
    flashTimeoutTimer->stop();
    delete flashTimeoutTimer;
    flashTimeoutTimer = NULL;

    EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);

    QProgressBar *bar =  findChild<QProgressBar *>("FlashProgress");
    ui->mainLayout->removeWidget(bar);
    delete bar;

    if(m_state & STATE_EEPROM_READ)
    {
        m_state &= ~(STATE_EEPROM_READ);
        delete m_eeprom;

        showErrorBox(tr("Timeout during reading EEPROM!"));
        return;
    }

    if(m_state & STATE_EEPROM_WRITE)
    {
        m_state &= ~(STATE_EEPROM_WRITE);
        delete m_eeprom;

        showErrorBox(tr("Timeout during writing EEPROM!"));
        return;
    }

    delete hex;
    hex = NULL;

    m_state &= ~(STATE_FLASHING);

    ui->flashText->setText("");

    showErrorBox(tr("Timeout during flashing!"));
}

void LorrisTerminal::deviceIdTimeout()
{
    m_state &= ~(STATE_AWAITING_ID);

    delete flashTimeoutTimer;
    flashTimeoutTimer = NULL;

    showErrorBox(tr("Can't get device id!"));

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

void LorrisTerminal::sendKeyEvent(QByteArray key)
{
    if(!(m_state & STATE_DISCONNECTED))
        m_con->SendData(key);
}

void LorrisTerminal::EnableButtons(quint16 buttons, bool enable)
{
    if(buttons & BUTTON_DISCONNECT)
        ui->connectButton->setEnabled(enable);

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
    for(quint8 i = 0; i < FMT_MAX; ++i)
        m_fmt_act[i]->setChecked(i == act);

    sConfig.set(CFG_QUINT32_TERMINAL_FMT, act);
    terminal->setFmt(act);
}

void LorrisTerminal::loadText()
{
    static const QString filters = tr("Text file (*.txt);;Any file (*.*)");
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    sConfig.get(CFG_STRING_HEX_FOLDER),
                                                    filters);
    if(filename.isEmpty())
        return;

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Utils::ThrowException(tr("Can't open file \"%1\"!").arg(filename), this);
        return;
    }

    terminal->appendText(file.readAll());
    file.close();

    sConfig.set(CFG_STRING_HEX_FOLDER, filename.left(filename.lastIndexOf(QRegExp("[\\/]"))));
}

void LorrisTerminal::saveText()
{
    static const QString filters = tr("Text file (*.txt);;Any file (*.*)");
    QString filename = QFileDialog::getSaveFileName(this, tr("Save data"), sConfig.get(CFG_STRING_HEX_FOLDER), filters);

    if(filename.isEmpty())
        return;

    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        Utils::ThrowException(tr("Can't open/create file \"%1\"!").arg(filename), this);
        return;
    }

    terminal->writeToFile(&file);
    file.close();

    sConfig.set(CFG_STRING_HEX_FOLDER, filename.left(filename.lastIndexOf(QRegExp("[\\/]"))));
}

void LorrisTerminal::inputAct(int act)
{
    for(quint8 i = 0; i < INPUT_MAX; ++i)
        m_input[i]->setChecked(i == act);

    sConfig.set(CFG_QUINT32_TERMINAL_INPUT, act);
    terminal->setInput(act);
}
