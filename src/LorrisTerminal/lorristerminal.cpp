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
#include <QMessageBox>
#include <QProgressBar>
#include <QLabel>
#include <QKeyEvent>
#include <QProgressDialog>

#include "lorristerminal.h"
#include "hexfile.h"
#include "deviceinfo.h"
#include "terminal.h"
#include "eeprom.h"

LorrisTerminal::LorrisTerminal() : WorkTab()
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
    hexLine = NULL;
    m_eeprom = NULL;
    initUI();
}

void LorrisTerminal::initUI()
{
    layout = new QVBoxLayout(this);
    QHBoxLayout *layout_buttons = new QHBoxLayout;
    terminal = new Terminal(this);
    connect(terminal, SIGNAL(keyPressedASCII(QByteArray)), this, SLOT(sendKeyEvent(QByteArray)));

    hexLine = new QLineEdit(this);
    QPushButton *browse = new QPushButton(tr("Browse..."), this);
    connect(browse, SIGNAL(clicked()), this, SLOT(browseForHex()));

    QPushButton *con = new QPushButton(tr("Disconnect"), this);
    con->setObjectName("ConnectButton");
    connect(con, SIGNAL(clicked()), this, SLOT(connectButton()));

    QPushButton *stop = new QPushButton(tr("Stop"), this);
    stop->setObjectName("StopButton");
    connect(stop, SIGNAL(clicked()), this, SLOT(stopButton()));

    QPushButton *flash = new QPushButton(tr("Flash"), this);
    flash->setEnabled(false);
    flash->setObjectName("FlashButton");
    connect(flash, SIGNAL(clicked()), this, SLOT(flashButton()));

    QPushButton *pause = new QPushButton(tr("Pause"), this);
    pause->setObjectName("PauseButton");
    connect(pause, SIGNAL(clicked()), this, SLOT(pauseButton()));

    QPushButton *clear = new QPushButton(tr("Clear"), this);
    connect(clear, SIGNAL(clicked()), this, SLOT(clearButton()));

    QPushButton *eeprom = new QPushButton(tr("Export EEPROM"), this);
    eeprom->setObjectName("eepromExportButton");
    eeprom->setEnabled(false);
    connect(eeprom, SIGNAL(clicked()), this, SLOT(eepromButton()));

    QPushButton *eeprom_im = new QPushButton(tr("Import EEPROM"), this);
    eeprom_im->setObjectName("eepromImportButton");
    eeprom_im->setEnabled(false);
    connect(eeprom_im, SIGNAL(clicked()), this, SLOT(eepromImportButton()));

    QLabel *flashText = new QLabel(this);
    flashText->setMinimumWidth(100);
    flashText->setObjectName("FlashLabel");

    layout_buttons->addWidget(con);
    layout_buttons->addWidget(stop);
    layout_buttons->addWidget(flash);
    layout_buttons->addWidget(hexLine);
    layout_buttons->addWidget(browse);
    layout_buttons->addWidget(flashText);
    layout_buttons->addWidget(eeprom_im);
    layout_buttons->addWidget(eeprom);
    layout_buttons->addWidget(pause);
    layout_buttons->addWidget(clear);
    layout->addLayout(layout_buttons);
    layout->addWidget(terminal);
}

LorrisTerminal::~LorrisTerminal()
{
    WorkTab::DeleteAllMembers(layout);
    delete layout;
}

void LorrisTerminal::browseForHex()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    sConfig.get(CFG_STRING_HEX_FOLDER),
                                                    tr("Intel hex file (*.hex)"));
    hexLine->setText(filename);
    if(filename.length() != 0)
        sConfig.set(CFG_STRING_HEX_FOLDER, filename.left(filename.lastIndexOf(QRegExp("[\\/]"))));
}

void LorrisTerminal::clearButton()
{
    terminal->setTextTerm("");
}

void LorrisTerminal::pauseButton()
{
    QPushButton *button = findChild<QPushButton *>("PauseButton");
    if(!(m_state & STATE_PAUSED))
    {
        m_state |= STATE_PAUSED;
        button->setText(tr("Unpause"));
    }
    else
    {
        terminal->updateEditText();
        m_state &= ~(STATE_PAUSED);
        button->setText(tr("Pause"));
    }
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

    DeviceInfo *info = new DeviceInfo(id);
    if(!info->isSet())
    {
        QMessageBox *box = new QMessageBox(this);
        box->setWindowTitle(tr("Error!"));
        box->setText(tr("Unsupported chip: ") + id);
        box->setIcon(QMessageBox::Critical);
        box->exec();
        delete box;
        delete info;

        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);
        return;
    }

    m_eeprom = new EEPROM(this, info);
    if(!m_eeprom->Import())
    {
        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);
        return;
    }

    QProgressBar *bar = new QProgressBar(this);
    bar->setMaximum(info->eeprom_size);
    bar->setObjectName("FlashProgress");
    layout->insertWidget(1, bar);

    flashTimeoutTimer = new QTimer();
    connect(flashTimeoutTimer, SIGNAL(timeout()), this, SLOT(flashTimeout()));
    flashTimeoutTimer->start(1500);

    eeprom_send_page();
}

bool LorrisTerminal::eeprom_send_page()
{
    Page *page = m_eeprom->getNextPage();
    QProgressBar *bar = findChild<QProgressBar *>("FlashProgress");
    if(!page)
    {
        delete flashTimeoutTimer;
        flashTimeoutTimer = NULL;
        layout->removeWidget(bar);
        delete bar;
        delete m_eeprom;

        m_state &= ~(STATE_EEPROM_WRITE);
        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);
        return false;
    }

    flashTimeoutTimer->start(1500);
    bar->setValue(page->address);

    QByteArray data;
    data[0] = 0x14;
    m_con->SendData(data);

    data[0] = (page->address >> 8);
    data[1] = quint8(page->address);
    data[2] = 2;
    m_con->SendData(data);
    data.clear();
    for(quint8 i = 0; i < page->data.size(); ++i)
        data[i] = page->data[i];
    m_con->SendData(data);
    return true;
}

void LorrisTerminal::eeprom_read(QString id)
{
    delete flashTimeoutTimer;
    flashTimeoutTimer = NULL;

    DeviceInfo *info = new DeviceInfo(id);
    if(!info->isSet())
    {
        QMessageBox *box = new QMessageBox(this);
        box->setWindowTitle(tr("Error!"));
        box->setText(tr("Unsupported chip: ") + id);
        box->setIcon(QMessageBox::Critical);
        box->exec();
        delete box;
        delete info;

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

    m_eeprom = new EEPROM(this, info);

    QProgressBar *bar = new QProgressBar(this);
    bar->setMaximum(info->eeprom_size);
    bar->setObjectName("FlashProgress");
    layout->insertWidget(1, bar);

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

        layout->removeWidget(bar);
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
    QPushButton *button = findChild<QPushButton *>("StopButton");
    if(!(m_state & STATE_DISCONNECTED))
        m_con->Close();
    else
    {
        button = findChild<QPushButton *>("ConnectButton");
        button->setText(tr("Connecting..."));
        button->setEnabled(false);

        connect(m_con, SIGNAL(connectResult(Connection*,bool)), this, SLOT(connectionResult(Connection*,bool)));
        m_con->OpenConcurrent();
    }
}

void LorrisTerminal::connectedStatus(bool connected)
{
    QPushButton *button = findChild<QPushButton *>("ConnectButton");
    if(connected)
    {
        m_state &= ~(STATE_DISCONNECTED);
        button->setText(tr("Disconnect"));

        button = findChild<QPushButton *>("StopButton");
        button->setEnabled(true);
        button->setText("Stop");
    }
    else
    {
        m_state |= STATE_DISCONNECTED;
        m_state &= ~(STATE_STOPPING1 | STATE_STOPPING2 | STATE_STOPPED);

        button->setText(tr("Connect"));

        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), false);
    }
}

void LorrisTerminal::connectionResult(Connection */*con*/,bool result)
{
    disconnect(m_con, SIGNAL(connectResult(Connection*,bool)), this, 0);
    QPushButton *button = findChild<QPushButton *>("ConnectButton");
    button->setEnabled(true);
    if(!result)
    {
        button->setText(tr("Connect"));

        QMessageBox *box = new QMessageBox(this);
        box->setIcon(QMessageBox::Critical);
        box->setWindowTitle(tr("Error!"));
        box->setText(tr("Can't open serial port!"));
        box->exec();
        delete box;
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
        QPushButton *button = findChild<QPushButton *>("StopButton");
        button->setText(tr("Start"));
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

    terminal->appendText(QString(data), !(m_state & STATE_PAUSED));

}

void LorrisTerminal::stopButton()
{
    QPushButton *stop = findChild<QPushButton *>("StopButton");
    if(m_state & STATE_STOPPED)
    {
        QByteArray data;
        data[0] = 0x11;
        m_con->SendData(data);

        stop->setText(tr("Stop"));
        m_state &= ~(STATE_STOPPED);

        EnableButtons((BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), false);

        terminal->setFocus();
    }
    else
    {
        m_state |= STATE_STOPPING1;
        stop->setText(tr("Stopping.."));
        stop->setEnabled(false);

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
        QPushButton *button = findChild<QPushButton *>("StopButton");
        button->setText("Stop");
        button->setEnabled(true);

        QMessageBox *box = new QMessageBox(this);
        box->setIcon(QMessageBox::Critical);
        box->setWindowTitle(tr("Error!"));
        box->setText(tr("Timeout on stopping chip!"));
        box->exec();
        delete box;
    }
}

void LorrisTerminal::flashButton()
{
    if(hex) delete hex;
    hex = new HexFile();

    QString load = hex->load(hexLine->text());
    if(load != "")
    {
        QMessageBox *box = new QMessageBox(this);
        box->setWindowTitle(tr("Error!"));
        box->setText(tr("Error loading hex file: ") + load);
        box->setIcon(QMessageBox::Critical);
        box->exec();
        delete box;
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

    DeviceInfo *info = new DeviceInfo(deviceId);
    if(!info->isSet())
    {
        QMessageBox *box = new QMessageBox(this);
        box->setWindowTitle(tr("Error!"));
        box->setText(tr("Unsupported chip: ") + deviceId);
        box->setIcon(QMessageBox::Critical);
        box->exec();
        delete box;
        delete hex;
        delete info;
        hex = NULL;

        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);
        return;
    }

    QString make = hex->makePages(info);
    if(make != "")
    {
        QMessageBox *box = new QMessageBox(this);
        box->setWindowTitle(tr("Error!"));
        box->setText(tr("Error making pages: ") + make);
        box->setIcon(QMessageBox::Critical);
        box->exec();
        delete box;
        delete hex;
        delete info;
        hex = NULL;

        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);
        return;
    }
    QProgressBar *bar = new QProgressBar(this);
    bar->setMaximum(hex->getPagesCount());
    bar->setObjectName("FlashProgress");
    layout->insertWidget(1, bar);

    QLabel *label = findChild<QLabel *>("FlashLabel");
    label->setText(tr("Flashing into ") + info->name + "...");

    flashTimeoutTimer = new QTimer();
    connect(flashTimeoutTimer, SIGNAL(timeout()), this, SLOT(flashTimeout()));
    flashTimeoutTimer->start(1500);

    delete info;

    m_state |= STATE_FLASHING;
    SendNextPage();
}

bool LorrisTerminal::SendNextPage()
{
    flashTimeoutTimer->start(1500);
    QProgressBar *bar = findChild<QProgressBar *>("FlashProgress");
    Page *page = hex->getNextPage();
    if(!page)
    {
        layout->removeWidget(bar);
        delete bar;

        QLabel *label = findChild<QLabel *>("FlashLabel");
        label->setText("");

        EnableButtons((BUTTON_STOP | BUTTON_FLASH | BUTTON_EEPROM_READ | BUTTON_EEPROM_WRITE), true);

        delete hex;
        delete flashTimeoutTimer;
        hex = NULL;
        flashTimeoutTimer = NULL;
        return false;
    }
    QByteArray data;
    data[0] = 0x10;
    m_con->SendData(data);

    data[0] = quint8(page->address >> 8);
    data[1] = quint8(page->address);
    m_con->SendData(data);

    for(quint16 i = 0; i < page->data.size(); ++i)
        data[i] = page->data[i];
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
    layout->removeWidget(bar);
    delete bar;

    if(m_state & STATE_EEPROM_READ)
    {
        m_state &= ~(STATE_EEPROM_READ);
        delete m_eeprom;

        QMessageBox *box = new QMessageBox(this);
        box->setWindowTitle(tr("Error!"));
        box->setText(tr("Timeout during reading EEPROM!"));
        box->setIcon(QMessageBox::Critical);
        box->exec();
        delete box;
        return;
    }

    if(m_state & STATE_EEPROM_WRITE)
    {
        m_state &= ~(STATE_EEPROM_WRITE);
        delete m_eeprom;

        QMessageBox *box = new QMessageBox(this);
        box->setWindowTitle(tr("Error!"));
        box->setText(tr("Timeout during writing EEPROM!"));
        box->setIcon(QMessageBox::Critical);
        box->exec();
        delete box;
        return;
    }

    delete hex;
    hex = NULL;

    m_state &= ~(STATE_FLASHING);

    QLabel *label = findChild<QLabel *>("FlashLabel");
    label->setText("");

    QMessageBox *box = new QMessageBox(this);
    box->setWindowTitle(tr("Error!"));
    box->setText(tr("Timeout during flashing!"));
    box->setIcon(QMessageBox::Critical);
    box->exec();
    delete box;
}

void LorrisTerminal::deviceIdTimeout()
{
    m_state &= ~(STATE_AWAITING_ID);

    delete flashTimeoutTimer;
    flashTimeoutTimer = NULL;

    QMessageBox *box = new QMessageBox(this);
    box->setWindowTitle(tr("Error!"));
    box->setText(tr("Can't get device id!"));
    box->setIcon(QMessageBox::Critical);
    box->exec();
    delete box;


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
    QPushButton *button = NULL;
    if(buttons & BUTTON_DISCONNECT)
    {
        button = findChild<QPushButton *>("ConnectButton");
        button->setEnabled(enable);
    }

    if(buttons & BUTTON_STOP)
    {
        button = findChild<QPushButton *>("StopButton");
        button->setEnabled(enable);
    }

    if(buttons & BUTTON_FLASH)
    {
        button = findChild<QPushButton *>("FlashButton");
        button->setEnabled(enable);
    }

    if(buttons & BUTTON_EEPROM_READ)
    {
        button = findChild<QPushButton *>("eepromExportButton");
        button->setEnabled(enable);
    }

    if(buttons & BUTTON_EEPROM_WRITE)
    {
        button = findChild<QPushButton *>("eepromImportButton");
        button->setEnabled(enable);
    }
}
