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

    QLabel *flashText = new QLabel(this);
    flashText->setMinimumWidth(100);
    flashText->setObjectName("FlashLabel");

    layout_buttons->addWidget(con);
    layout_buttons->addWidget(stop);
    layout_buttons->addWidget(flash);
    layout_buttons->addWidget(hexLine);
    layout_buttons->addWidget(browse);
    layout_buttons->addWidget(flashText);
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
    hexLine->setText(QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Intel hex file (*.hex)")));
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

void LorrisTerminal::connectButton()
{
    QPushButton *button = findChild<QPushButton *>("StopButton");
    if(!(m_state & STATE_DISCONNECTED))
    {

        m_con->Close();
    }
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
    }
    else
    {
        m_state |= STATE_DISCONNECTED;
        m_state &= ~(STATE_STOPPING1 | STATE_STOPPING2 | STATE_STOPPED);

        button->setText(tr("Connect"));

        button = findChild<QPushButton *>("StopButton");
        button->setEnabled(false);
        button->setText(tr("Stop"));

        button = findChild<QPushButton *>("FlashButton");
        button->setEnabled(false);
        button->setText(tr("Flash"));
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

void LorrisTerminal::readData(QByteArray data)
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
        button->setEnabled(true);
        button = findChild<QPushButton *>("FlashButton");
        button->setEnabled(true);
        return;
    }
    else if(m_state & STATE_AWAITING_ID)
    {
        m_state &= ~(STATE_AWAITING_ID);
        flash_prepare(QString(data));
        return;
    }
    else if(m_state & STATE_FLASHING)
    {
        if(!SendNextPage())
            m_state &= ~(STATE_FLASHING);
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

        stop = findChild<QPushButton *>("FlashButton");
        stop->setEnabled(false);

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
    QPushButton *button = findChild<QPushButton *>("FlashButton");
    button->setEnabled(false);
    button->setText(tr("Flashing..."));
    button = findChild<QPushButton *>("StopButton");
    button->setEnabled(false);

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
        return;

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

        QPushButton *button = findChild<QPushButton *>("FlashButton");
        button->setEnabled(true);
        button->setText(tr("Flash"));
        button = findChild<QPushButton *>("StopButton");
        button->setEnabled(true);
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

        QPushButton *button = findChild<QPushButton *>("FlashButton");
        button->setEnabled(true);
        button->setText(tr("Flash"));
        button = findChild<QPushButton *>("StopButton");
        button->setEnabled(true);

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
    delete hex;
    hex = NULL;
    flashTimeoutTimer = NULL;

    m_state &= ~(STATE_FLASHING);

    QProgressBar *bar =  findChild<QProgressBar *>("FlashProgress");
    layout->removeWidget(bar);
    delete bar;

    QPushButton *button = findChild<QPushButton *>("FlashButton");
    button->setEnabled(true);
    button->setText(tr("Flash"));
    button = findChild<QPushButton *>("StopButton");
    button->setEnabled(true);

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
    delete hex;
    flashTimeoutTimer = NULL;
    hex = NULL;

    QPushButton *button = findChild<QPushButton *>("FlashButton");
    button->setEnabled(true);
    button->setText(tr("Flash"));
    button = findChild<QPushButton *>("StopButton");
    button->setEnabled(true);

    QMessageBox *box = new QMessageBox(this);
    box->setWindowTitle(tr("Error!"));
    box->setText(tr("Can't get device id!"));
    box->setIcon(QMessageBox::Critical);
    box->exec();
    delete box;
}

void LorrisTerminal::sendKeyEvent(QByteArray key)
{
    if(!(m_state & STATE_DISCONNECTED))
        m_con->SendData(key);
}
