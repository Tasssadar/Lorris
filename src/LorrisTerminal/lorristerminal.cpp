#include <QTextEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QScrollBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QLabel>

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
    terminal = new Terminal();
    mainWidget = NULL;
    text = NULL;
    hexLine = NULL;
}

void LorrisTerminal::initUI()
{
    mainWidget = new QWidget();
    layout = new QVBoxLayout(mainWidget);
    QHBoxLayout *layout_buttons = new QHBoxLayout;
    text = new QTextEdit(mainWidget);
    text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    text->setShown(true);
    text->setReadOnly(true);

    QColor color_black(0, 0, 0);\
    QColor color_white(255, 255, 255);\
    QPalette palette;
    palette.setColor(QPalette::Base, color_black);
    palette.setColor(QPalette::Text, color_white);
    text->setPalette(palette);

    hexLine = new QLineEdit(mainWidget);
    QPushButton *browse = new QPushButton("Browse...", mainWidget);
    connect(browse, SIGNAL(clicked()), this, SLOT(browseForHex()));

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

    layout_buttons->addWidget(stop);
    layout_buttons->addWidget(flash);
    layout_buttons->addWidget(hexLine);
    layout_buttons->addWidget(browse);
    layout_buttons->addWidget(flashText);
    layout_buttons->addWidget(pause);
    layout_buttons->addWidget(clear);
    layout->addLayout(layout_buttons);
    layout->addWidget(text);
}

LorrisTerminal::~LorrisTerminal()
{
    delete terminal;
    terminal = NULL;
    text = NULL;
    if(mainWidget)
    {
        WorkTab::DeleteAllMembers(layout);
        delete layout;
        delete mainWidget;
    }
}

QWidget *LorrisTerminal::GetTab(QWidget *parent)
{
    initUI();
    mainWidget->setParent(parent);
    return mainWidget;
}

void LorrisTerminal::browseForHex()
{
    hexLine->setText(QFileDialog::getOpenFileName(mainWidget, tr("Open File"), "", tr("Intel hex file (*.hex)")));
}

void LorrisTerminal::clearButton()
{
    text->setText("");
    terminal->setText("");
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
        text->setText(terminal->getText());
        m_state &= ~(STATE_PAUSED);
        button->setText("Pause");
        QScrollBar *sb = text->verticalScrollBar();
        sb->setValue(sb->maximum());
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
        m_state &= ~(STATE_STOPPING1);
        m_state |= STATE_STOPPING2;

        m_con->SendData(stopCmd);
        return;
    }
    else if((m_state & STATE_STOPPING2) && data[0] == char(20))
    {
        QPushButton *button = mainWidget->findChild<QPushButton *>("StopButton");
        button->setText("Start");
        button->setEnabled(true);
        button = mainWidget->findChild<QPushButton *>("FlashButton");
        button->setEnabled(true);

        m_state &= ~(STATE_STOPPING2);
        m_state |= STATE_STOPPED;
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

    if(!text)
        return;

    terminal->appendText(QString(data));
    if(!(m_state & STATE_PAUSED))
    {
        text->moveCursor(QTextCursor::End);
        text->insertPlainText(QString(data));
        QScrollBar *sb = text->verticalScrollBar();
        sb->setValue(sb->maximum());
    }
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
        stopTimer->start(1000);
    }
}

void LorrisTerminal::stopTimerSig()
{
    delete stopTimer;
    stopTimer = NULL;
    m_state &= ~(STATE_STOPPING1);
    m_state |= STATE_STOPPING2;

    m_con->SendData(stopCmd);
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

    data[0] = uint8_t(page->address >> 8);
    data[1] = uint8_t(page->address);
    m_con->SendData(data);

    for(uint16_t i = 0; i < page->data.size(); ++i)
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
    box->exec();
    delete box;
}
