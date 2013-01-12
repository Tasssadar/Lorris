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
#include <QInputDialog>

#include "lorristerminal.h"
#include "../ui/terminal.h"
#include "eeprom.h"
#include "../WorkTab/WorkTabMgr.h"
#include "../shared/hexfile.h"
#include "../shared/defmgr.h"
#include "../ui/ui_lorristerminal.h"
#include "../ui/chooseconnectiondlg.h"
#include "../ui/tooltipwarn.h"

LorrisTerminal::LorrisTerminal()
    : ui(new Ui::LorrisTerminal)
{
    m_stopped = false;

    initUI();

    EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), false);
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
    connect(ui->sendBtn,       SIGNAL(clicked()),                   SLOT(sendButton()));
    connect(m_export_eeprom,   SIGNAL(triggered()),                 SLOT(eepromExportButton()));
    connect(m_import_eeprom,   SIGNAL(triggered()),                 SLOT(eepromImportButton()));
    connect(termLoad,          SIGNAL(triggered()),                 SLOT(loadText()));
    connect(termSave,          SIGNAL(triggered()),                 SLOT(saveText()));
    connect(binSave,           SIGNAL(triggered()),                 SLOT(saveBin()));
    connect(chgSettings,       SIGNAL(triggered()),   ui->terminal, SLOT(showSettings()));
    connect(bootloaderAct,     SIGNAL(triggered(bool)),             SLOT(showBootloader(bool)));
    connect(showWarnAct,       SIGNAL(triggered(bool)),             SLOT(showWarn(bool)));
    connect(ui->terminal,      SIGNAL(fmtSelected(int)),            SLOT(checkFmtAct(int)));
    connect(ui->terminal,      SIGNAL(paused(bool)),                SLOT(setPauseBtnText(bool)));
    connect(qApp,              SIGNAL(focusChanged(QWidget*,QWidget*)), SLOT(focusChanged(QWidget*,QWidget*)));

    m_connectButton = new ConnectButton(ui->connectButton2);
    connect(m_connectButton, SIGNAL(connectionChosen(ConnectionPointer<Connection>)), this, SLOT(setConnection(ConnectionPointer<Connection>)));

    fmtAction(fmt);
}

LorrisTerminal::~LorrisTerminal()
{
    delete ui;
}

QString LorrisTerminal::GetIdString()
{
    return "LorrisTerminal";
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
    ui->terminal->pause(!ui->terminal->isPaused());
}

void LorrisTerminal::setPauseBtnText(bool pause)
{
    if(pause)
        ui->pauseButton->setText(tr("Unpause"));
    else
        ui->pauseButton->setText(tr("Pause"));
}

void LorrisTerminal::eepromExportButton()
{
    if(!m_stopped)
        stopButton();
    else
    {
        if(!m_bootloader.stopSequence())
            return;
    }

    if(!m_stopped)
        return;

    EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), false);

    if(m_bootloader.getChipId())
        m_bootloader.readEEPROM(ui);

    EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);
}

void LorrisTerminal::eepromImportButton()
{
    stopButton();

    if(!m_stopped)
        return;

    EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), false);

    if(m_bootloader.getChipId())
        m_bootloader.writeEEPROM(ui);

    EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);
}

void LorrisTerminal::connectedStatus(bool connected)
{
    if(connected)
    {
        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);
        ui->stopButton->setText(tr("Stop"));
        m_stopped = false;
        m_bootloader.setStopStatus(false);
        ui->terminal->setFocus();
    }
    else
    {
        m_stopped = false;
        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), false);
    }
}

void LorrisTerminal::readData(const QByteArray& data)
{
    if(!m_bootloader.dataRead(data))
        ui->terminal->appendText(data);
}

void LorrisTerminal::stopButton()
{
    ui->stopButton->setEnabled(false);
    bool res = m_bootloader.startStop();
    ui->stopButton->setEnabled(true);

    if(!res)
    {
        Utils::showErrorBox(tr("Timeout on stopping chip!"));
        return;
    }

    if(m_stopped)
        ui->stopButton->setText(tr("Stop"));
    else
        ui->stopButton->setText(tr("Start"));

    ui->terminal->setFocus();
    m_stopped = !m_stopped;
}

void LorrisTerminal::flashButton()
{
    bool restart = !m_stopped;

    if(!m_stopped)
        stopButton();

    if(!m_stopped)
        return;

    setHexName();

    if(actions()[1]->isChecked() && m_filedate.isValid() && m_filedate == m_flashdate)
    {
        new ToolTipWarn(tr("You have flashed this file already, and it was not changed since."), ui->flashButton, this);
        Utils::playErrorSound();
    }

    try
    {
        m_bootloader.getHex()->LoadFromFile(m_filename);
    }
    catch(QString ex)
    {
        Utils::showErrorBox(tr("Error loading hex file: ") + ex);
        return;
    }

    EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), false);

    if(m_bootloader.getChipId() && m_bootloader.flash(ui))
    {
        m_flashdate = m_filedate;

        if(restart)
            stopButton();
    }

    EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);
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
        Utils::showErrorBox(tr("Can't open file \"%1\"!").arg(filename), this);
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
        Utils::showErrorBox(tr("Can't open/create file \"%1\"!").arg(filename), this);
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
        Utils::showErrorBox(tr("Can't open/create file \"%1\"!").arg(filename), this);
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

void LorrisTerminal::setPortConnection(ConnectionPointer<PortConnection> const & con)
{
    this->PortConnWorkTab::setPortConnection(con);
    m_connectButton->setConn(con, false);
    m_bootloader.setCon(con.data());
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

    if(!info.exists())
        return;

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

void LorrisTerminal::saveData(DataFileParser *file)
{
    PortConnWorkTab::saveData(file);

    file->writeBlockIdentifier("LorrTermData");
    {
        QByteArray termData = ui->terminal->getData();
        file->writeVal(termData.size());
        file->write(termData);
    }

    file->writeBlockIdentifier("LorrTermFilename");
    file->writeString(m_filename);

    file->writeBlockIdentifier("LorrTermSettings");
    file->writeString(ui->terminal->getSettingsData());

    file->writeBlockIdentifier("LorrTermFmtInput");
    file->writeVal(ui->terminal->getFmt());
    file->writeVal(ui->terminal->getInput());

    file->writeBlockIdentifier("LorrTermActions");
    file->writeVal(actions()[0]->isChecked());
    file->writeVal(actions()[1]->isChecked());
}

void LorrisTerminal::loadData(DataFileParser *file)
{
    PortConnWorkTab::loadData(file);

    if(file->seekToNextBlock("LorrTermData", BLOCK_WORKTAB))
    {
        int size = file->readVal<int>();
        QByteArray termData = file->read(size);
        ui->terminal->appendText(termData);
    }

    if(file->seekToNextBlock("LorrTermFilename", BLOCK_WORKTAB))
    {
        QString name = file->readString();
        setHexName(name);
    }

    if(file->seekToNextBlock("LorrTermSettings", BLOCK_WORKTAB))
       ui->terminal->loadSettings(file->readString());

    if(file->seekToNextBlock("LorrTermFmtInput", BLOCK_WORKTAB))
    {
        ui->terminal->setFmt(file->readVal<int>());
        inputAct(file->readVal<int>());
    }

    if(file->seekToNextBlock("LorrTermActions", BLOCK_WORKTAB))
    {
        showBootloader(file->readVal<bool>());
        showWarn(file->readVal<bool>());
    }
}

void LorrisTerminal::focusChanged(QWidget *prev, QWidget *curr)
{
    if(!prev && curr)
        setHexName();
}

void LorrisTerminal::sendButton()
{
    static QString lastText;
    QString text = QInputDialog::getText(this, tr("Send data"), tr("Enter bytes to send:\n - Numbers from 0 to 255,"
                                         "-127 to 128 or 0x00 to 0xFF\n - Separated by space"), QLineEdit::Normal, lastText);
    if(text.isEmpty())
        return;

    QStringList nums = text.split(" ", QString::SkipEmptyParts);
    QByteArray data;
    bool ok = false;
    for(int i = 0; i < nums.size(); ++i)
    {
        int base;
        if(nums[i].contains(QChar('x'), Qt::CaseInsensitive))
            base = 16;
        else
            base = 10;

        char num = nums[i].toInt(&ok, base);
        if(ok)
            data.push_back(num);
    }

    if(data.isEmpty())
        return;

    if(m_con)
        m_con->SendData(data);

    lastText = text;
}

void LorrisTerminal::setWindowId(quint32 id)
{
    WorkTab::setWindowId(id);
    ui->progressBar->setWindowId(sWorkTabMgr.getWindow(id)->winId());
}
