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
#include "../WorkTab/WorkTabMgr.h"
#include "../shared/hexfile.h"
#include "../shared/defmgr.h"
#include "../ui/ui_lorristerminal.h"
#include "../ui/chooseconnectiondlg.h"
#include "../ui/tooltipwarn.h"
#include "../connection/serialport.h"

LorrisTerminal::LorrisTerminal()
    : ui(new Ui::LorrisTerminal)
{
    initUI();
}

void LorrisTerminal::initUI()
{
    ui->setupUi(this);

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
    termSave->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    binSave->setShortcut(QKeySequence("Ctrl+S"));
    binSave->setShortcutContext(Qt::WidgetWithChildrenShortcut);

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

    inputAct(sConfig.get(CFG_QUINT32_TERMINAL_INPUT));

    ui->spRts->setVisible(false);
    ui->spDtr->setVisible(false);

    ui->terminal->loadSettings(sConfig.get(CFG_STRING_TERMINAL_SETTINGS));

    connect(inputMap,          SIGNAL(mapped(int)),                 SLOT(inputAct(int)));
    connect(fmtMap,            SIGNAL(mapped(int)),                 SLOT(fmtAction(int)));
    connect(ui->terminal,      SIGNAL(keyPressed(QString)),         SLOT(sendKeyEvent(QString)));
    connect(ui->pauseButton,   SIGNAL(clicked()),                   SLOT(pauseButton()));
    connect(ui->clearButton,   SIGNAL(clicked()),     ui->terminal, SLOT(clear()));
    connect(ui->terminal,      SIGNAL(settingsChanged()),           SLOT(saveTermSettings()));
    connect(ui->fmtBox,        SIGNAL(activated(int)),              SLOT(fmtAction(int)));
    connect(ui->sendBtn,       SIGNAL(clicked()),                   SLOT(sendButton()));
    connect(termLoad,          SIGNAL(triggered()),                 SLOT(loadText()));
    connect(termSave,          SIGNAL(triggered()),                 SLOT(saveText()));
    connect(binSave,           SIGNAL(triggered()),                 SLOT(saveBin()));
    connect(chgSettings,       SIGNAL(triggered()),   ui->terminal, SLOT(showSettings()));
    connect(ui->terminal,      SIGNAL(fmtSelected(int)),            SLOT(checkFmtAct(int)));
    connect(ui->terminal,      SIGNAL(paused(bool)),                SLOT(setPauseBtnText(bool)));
    connect(ui->spRts,         SIGNAL(toggled(bool)),               SLOT(spRtsToggled(bool)));
    connect(ui->spDtr,         SIGNAL(toggled(bool)),               SLOT(spDtrToggled(bool)));

    fmtAction(fmt);

#ifndef Q_OS_MAC
    m_connectButton = new ConnectButton(ui->connectButton2);
    connect(m_connectButton, SIGNAL(connectionChosen(ConnectionPointer<Connection>)), this, SLOT(setConnection(ConnectionPointer<Connection>)));
#else
    QMacToolBarItem *connectBtn = new QMacToolBarItem;
    connectBtn->setIcon(QIcon(":/actions/wire"));
    connectBtn->setText("Connect");
    m_macBarItems.push_back(connectBtn);

    QMacToolBarItem *chooseConnection = new QMacToolBarItem;
    chooseConnection->setIcon(QIcon(":/actions/wire"));
    chooseConnection->setText("Choose connection");
    m_macBarItems.push_back(chooseConnection);
    ui->connectButton2->hide();
    ui->horizontalSpacer->changeSize(0,0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_connectButton = new ConnectButton(ui->connectButton2, connectBtn, chooseConnection);
    connect(m_connectButton, SIGNAL(connectionChosen(ConnectionPointer<Connection>)), this, SLOT(setConnection(ConnectionPointer<Connection>)));
#endif
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

    if (!m_con && sConfig.get(CFG_BOOL_CONN_ON_NEW_TAB))
    {
        m_connectButton->choose();
        if (m_con && !m_con->isOpen())
            m_con->OpenConcurrent();
    }
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

void LorrisTerminal::connectedStatus(bool connected)
{
    if(connected)
        ui->terminal->setFocus();
}

void LorrisTerminal::readData(const QByteArray& data)
{
    ui->terminal->appendText(data);
}

void LorrisTerminal::sendKeyEvent(const QString &key)
{
    if(m_con && m_con->isOpen())
        m_con->SendData(key.toUtf8());
    else
        ui->terminal->blink(Qt::darkRed);
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
    if(m_con && m_con->getType() == CONNECTION_SERIAL_PORT) {
        disconnect(m_con.data(), SIGNAL(changed()), this, SLOT(serialPortConnectionChanged()));
    }

    this->PortConnWorkTab::setPortConnection(con);
    m_connectButton->setConn(con, false);
    connectedStatus(con && con->isOpen());

    if(con && con->getType() == CONNECTION_SERIAL_PORT) {
        SerialPort *sp = static_cast<SerialPort*>(con.data());
        ui->spDtr->setVisible(true);
        ui->spDtr->setChecked(sp->dtr());
        ui->spRts->setVisible(true);
        ui->spRts->setChecked(sp->rts());
        ui->spRts->setEnabled(sp->flowControl() != FLOW_HARDWARE);
        connect(m_con.data(), SIGNAL(changed()), this, SLOT(serialPortConnectionChanged()));
    } else {
        ui->spDtr->setVisible(false);
        ui->spRts->setVisible(false);
    }
}

void LorrisTerminal::saveTermSettings()
{
    sConfig.set(CFG_STRING_TERMINAL_SETTINGS, ui->terminal->getSettingsData());
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

    file->writeBlockIdentifier("LorrTermSettings");
    file->writeString(ui->terminal->getSettingsData());
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

    if(file->seekToNextBlock("LorrTermSettings", BLOCK_WORKTAB))
       ui->terminal->loadSettings(file->readString());

    if(file->seekToNextBlock("LorrTermFmtInput", BLOCK_WORKTAB))
    {
        ui->terminal->setFmt(file->readVal<int>());
        inputAct(file->readVal<int>());
    }
}

void LorrisTerminal::sendButton()
{
    static QString lastText;
    QString text = QInputDialog::getText(this, tr("Send data"),
                                         tr("Send bytes (Separated by space):\n  * numbers from 0 to 255\n"
                                         "  * -127 to 128\n  * 0x00 to 0xFF"
                                         "\nString:\n  * enclose it by quotation marks:\n"
                                         "     \"string to send as utf8\"\n"
                                         "  * do not escape anything in the string"), QLineEdit::Normal, lastText);
    if(text.isEmpty())
        return;

    lastText = text;

    QByteArray data;
    if(text.length() >= 3 && text.startsWith(QChar('"')) && text.endsWith(QChar('"'))) {
        data = text.mid(1, text.length()-2).toUtf8();
    } else {
        QStringList nums = text.split(" ", QString::SkipEmptyParts);
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

        if(data.isEmpty() && text.length() >= 3)
            data = text.toUtf8();
    }

    if(data.isEmpty())
        return;

    if(m_con)
        m_con->SendData(data);
}

void LorrisTerminal::spRtsToggled(bool set) {
    if(m_con && m_con->getType() == CONNECTION_SERIAL_PORT) {
        SerialPort *sp = static_cast<SerialPort*>(m_con.data());
        sp->setRts(set);
    }
}

void LorrisTerminal::spDtrToggled(bool set) {
    if(m_con && m_con->getType() == CONNECTION_SERIAL_PORT) {
        SerialPort *sp = static_cast<SerialPort*>(m_con.data());
        sp->setDtr(set);
    }
}

void LorrisTerminal::serialPortConnectionChanged() {
    if(m_con && m_con->getType() == CONNECTION_SERIAL_PORT) {
        SerialPort *sp = static_cast<SerialPort*>(m_con.data());
        ui->spRts->setEnabled(sp->flowControl() != FLOW_HARDWARE);
    }
}
