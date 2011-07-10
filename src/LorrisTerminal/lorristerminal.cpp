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
    mainWidget = NULL;
    hexLine = NULL;
}

void LorrisTerminal::initUI()
{
    mainWidget = new QWidget();
    layout = new QVBoxLayout(mainWidget);
    QHBoxLayout *layout_buttons = new QHBoxLayout;
    terminal = new Terminal(mainWidget);
    connect(terminal, SIGNAL(keyPressedASCII(QByteArray)), this, SLOT(sendKeyEvent(QByteArray)));
    terminal->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    terminal->setShown(true);
    terminal->setReadOnly(true);

    QColor color_black(0, 0, 0);\
    QColor color_white(255, 255, 255);\
    QPalette palette;
    palette.setColor(QPalette::Base, color_black);
    palette.setColor(QPalette::Text, color_white);
    terminal->setPalette(palette);

    hexLine = new QLineEdit(mainWidget);
    QPushButton *browse = new QPushButton("Browse...", mainWidget);
    connect(browse, SIGNAL(clicked()), this, SLOT(browseForHex()));

    QPushButton *con = new QPushButton("Disconnect", mainWidget);
    con->setObjectName("ConnectButton");
    connect(con, SIGNAL(clicked()), this, SLOT(connectButton()));

    QPushButton *stop = new QPushButton("Stop", mainWidget);
    stop->setObjectName("StopButton");
    connect(stop, SIGNAL(clicked()), this, SLOT(stopButton()));

    QPushButton *flash = new QPushButton("Flash", mainWidget);
    flash->setEnabled(false);
    flash->setObjectName("FlashButton");
    connect(flash, SIGNAL(clicked()), this, SLOT(flashButton()));

    QPushButton *pause = new QPushButton("Pause", mainWidget);
    pause->setObjectName("PauseButton");
    connect(pause, SIGNAL(clicked()), this, SLOT(pauseButton()));

    QPushButton *clear = new QPushButton("Clear", mainWidget);
    connect(clear, SIGNAL(clicked()), this, SLOT(clearButton()));

    QLabel *flashText = new QLabel(mainWidget);
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
    terminal = NULL;
    if(mainWidget)
    { 
        WorkTab::DeleteAllMembers(layout);
        delete layout;
        delete mainWidget;
    }
}

QWidget *LorrisTerminal::GetTab(QWidget *parent)
{
    if(!mainWidget)
    {
        initUI();
        mainWidget->setParent(parent);
    }
    return mainWidget;
}

void LorrisTerminal::browseForHex()
{
    hexLine->setText(QFileDialog::getOpenFileName(mainWidget, tr("Open File"), "", tr("Intel hex file (*.hex)")));
}

void LorrisTerminal::clearButton()
{
    terminal->setTextTerm("");
}

void LorrisTerminal::pauseButton()
{
    QPushButton *button = mainWidget->findChild<QPushButton *>("PauseButton");
    if(!(m_state & STATE_PAUSED))
    {
        m_state |= STATE_PAUSED;
        button->setText("Unpause");
    }
    else
    {
        terminal->updateEditText();
        m_state &= ~(STATE_PAUSED);
        button->setText("Pause");
    }
}

void LorrisTerminal::connectButton()
{
    QPushButton *button = mainWidget->findChild<QPushButton *>("StopButton");
    if(!(m_state & STATE_DISCONNECTED))
    {

        m_con->Close();
    }
    else
    {
        button = mainWidget->findChild<QPushButton *>("ConnectButton");
        button->setText("Connecting...");
        button->setEnabled(false);

        connect(m_con, SIGNAL(connectResult(Connection*,bool)), this, SLOT(connectionResult(Connection*,bool)));
        m_con->OpenConcurrent();
    }
}

void LorrisTerminal::connectedStatus(bool connected)
{
    QPushButton *button = mainWidget->findChild<QPushButton *>("ConnectButton");
    if(connected)
    {
        m_state &= ~(STATE_DISCONNECTED);
        button->setText("Disconnect");

        button = mainWidget->findChild<QPushButton *>("StopButton");
        button->setEnabled(true);
    }
    else
    {
        m_state |= STATE_DISCONNECTED;
        m_state &= ~(STATE_STOPPING1 | STATE_STOPPING2 | STATE_STOPPED);

        button->setText("Connect");

        button = mainWidget->findChild<QPushButton *>("StopButton");
        button->setEnabled(false);
        button->setText("Stop");

        button = mainWidget->findChild<QPushButton *>("FlashButton");
        button->setEnabled(false);
        button->setText("Flash");
    }
}

void LorrisTerminal::connectionResult(Connection */*con*/,bool result)
{
    disconnect(m_con, SIGNAL(connectResult(Connection*,bool)), this, 0);
    QPushButton *button = mainWidget->findChild<QPushButton *>("ConnectButton");
    button->setEnabled(true);
    if(!result)
    {
        button->setText("Connect");

        QMessageBox *box = new QMessageBox(mainWidget);
        box->setIcon(QMessageBox::Critical);
        box->setWindowTitle("Error!");
        box->setText("Can't open serial port!");
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
        QPushButton *button = mainWidget->findChild<QPushButton *>("StopButton");
        button->setText("Start");
        button->setEnabled(true);
        button = mainWidget->findChild<QPushButton *>("FlashButton");
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
    QPushButton *stop = mainWidget->findChild<QPushButton *>("StopButton");
    if(m_state & STATE_STOPPED)
    {
        QByteArray data;
        data[0] = 0x11;
        m_con->SendData(data);

        stop->setText("Stop");
        m_state &= ~(STATE_STOPPED);

        stop = mainWidget->findChild<QPushButton *>("FlashButton");
        stop->setEnabled(false);
    }
    else
    {
        m_state |= STATE_STOPPING1;
        stop->setText("Stopping..");
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
        QPushButton *button = mainWidget->findChild<QPushButton *>("StopButton");
        button->setText("Stop");
        button->setEnabled(true);

        QMessageBox *box = new QMessageBox(mainWidget);
        box->setIcon(QMessageBox::Critical);
        box->setWindowTitle("Error!");
        box->setText("Timeout on stopping chip!");
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
        QMessageBox *box = new QMessageBox(mainWidget);
        box->setWindowTitle("Error!");
        box->setText("Error loading hex file: " + load);
        box->setIcon(QMessageBox::Critical);
        box->exec();
        delete box;
        delete hex;
        hex = NULL;
        return;
    }
    QPushButton *button = mainWidget->findChild<QPushButton *>("FlashButton");
    button->setEnabled(false);
    button->setText("Flashing...");
    button = mainWidget->findChild<QPushButton *>("StopButton");
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
        QMessageBox *box = new QMessageBox(mainWidget);
        box->setWindowTitle("Error!");
        box->setText("Error making pages: " + make);
        box->setIcon(QMessageBox::Critical);
        box->exec();
        delete box;
        delete hex;
        delete info;
        hex = NULL;

        QPushButton *button = mainWidget->findChild<QPushButton *>("FlashButton");
        button->setEnabled(true);
        button->setText("Flash");
        button = mainWidget->findChild<QPushButton *>("StopButton");
        button->setEnabled(true);
        return;
    }
    QProgressBar *bar = new QProgressBar(mainWidget);
    bar->setMaximum(hex->getPagesCount());
    bar->setObjectName("FlashProgress");
    layout->insertWidget(1, bar);

    QLabel *label = mainWidget->findChild<QLabel *>("FlashLabel");
    label->setText("Flashing into " + info->name + "...");

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
    QProgressBar *bar = mainWidget->findChild<QProgressBar *>("FlashProgress");
    Page *page = hex->getNextPage();
    if(!page)
    {
        layout->removeWidget(bar);
        delete bar;

        QLabel *label = mainWidget->findChild<QLabel *>("FlashLabel");
        label->setText("");

        QPushButton *button = mainWidget->findChild<QPushButton *>("FlashButton");
        button->setEnabled(true);
        button->setText("Flash");
        button = mainWidget->findChild<QPushButton *>("StopButton");
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

    QProgressBar *bar =  mainWidget->findChild<QProgressBar *>("FlashProgress");
    layout->removeWidget(bar);
    delete bar;

    QPushButton *button = mainWidget->findChild<QPushButton *>("FlashButton");
    button->setEnabled(true);
    button->setText("Flash");
    button = mainWidget->findChild<QPushButton *>("StopButton");
    button->setEnabled(true);

    QLabel *label = mainWidget->findChild<QLabel *>("FlashLabel");
    label->setText("");

    QMessageBox *box = new QMessageBox(mainWidget);
    box->setWindowTitle("Error!");
    box->setText("Timeout during flashing!");
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

    QPushButton *button = mainWidget->findChild<QPushButton *>("FlashButton");
    button->setEnabled(true);
    button->setText("Flash");
    button = mainWidget->findChild<QPushButton *>("StopButton");
    button->setEnabled(true);

    QMessageBox *box = new QMessageBox(mainWidget);
    box->setWindowTitle("Error!");
    box->setText("Can't get device id!");
    box->setIcon(QMessageBox::Critical);
    box->exec();
    delete box;
}

void LorrisTerminal::sendKeyEvent(QByteArray key)
{
    if(!(m_state & STATE_DISCONNECTED))
        m_con->SendData(key);
}
