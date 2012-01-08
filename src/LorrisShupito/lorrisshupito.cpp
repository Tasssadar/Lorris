#define QT_USE_FAST_CONCATENATION

#include <QMessageBox>
#include <QTimer>
#include <stdio.h>
#include <QComboBox>

#include "shupito.h"
#include "lorrisshupito.h"
#include "ui_lorrisshupito.h"


LorrisShupito::LorrisShupito() : WorkTab(),ui(new Ui::LorrisShupito)
{
    ui->setupUi(this);

    QPushButton *connectButton = findChild<QPushButton*>("connectButton");
    connect(connectButton, SIGNAL(clicked()), this, SLOT(connectButton()));

    vccLabel = findChild<QLabel*>("vccLabel");
    vddBox = findChild<QComboBox*>("vddBox");

    m_shupito = new Shupito(this);
    m_desc = new ShupitoDesc();

    connect(m_shupito, SIGNAL(descRead()), this, SLOT(descRead()));
    connect(m_shupito, SIGNAL(responseReceived(char)), this, SLOT(responseReceived(char)));
    connect(m_shupito, SIGNAL(vccValueChanged(quint8,double)), this, SLOT(vccValueChanged(quint8,double)));
    connect(m_shupito, SIGNAL(vddDesc(vdd_setup)), this, SLOT(vddSetup(vdd_setup)));
    m_response = RESPONSE_NONE;

    responseTimer = NULL;
    m_vcc = 0;
    lastVccIndex = 0;
}

LorrisShupito::~LorrisShupito()
{
    delete ui;
    delete m_shupito;
    delete m_desc;
}

void LorrisShupito::connectButton()
{
    QPushButton *connectButt = findChild<QPushButton *>("connectButton");
    if(m_state & STATE_DISCONNECTED)
    {
        connectButt->setText(tr("Connecting..."));
        connectButt->setEnabled(false);
        connect(m_con, SIGNAL(connectResult(Connection*,bool)), this, SLOT(connectionResult(Connection*,bool)));
        m_con->OpenConcurrent();
    }
    else
    {
        m_con->Close();
        m_state |= STATE_DISCONNECTED;

        connectButt->setText(tr("Connect"));
    }
}

void LorrisShupito::connectionResult(Connection */*con*/,bool result)
{
    disconnect(m_con, SIGNAL(connectResult(Connection*,bool)), this, 0);
    QPushButton *button = findChild<QPushButton *>("connectButton");
    button->setEnabled(true);
    if(!result)
    {
        button->setText(tr("Connect"));

        QMessageBox *box = new QMessageBox(this);
        box->setIcon(QMessageBox::Critical);
        box->setWindowTitle(tr("Error!"));
        box->setText(tr("Can't open connection!"));
        box->exec();
        delete box;
    }
}

void LorrisShupito::connectedStatus(bool connected)
{
    QPushButton *button = findChild<QPushButton *>("connectButton");
    if(connected)
    {
        m_state &= ~(STATE_DISCONNECTED);
        button->setText(tr("Disconnect"));
    }
    else
    {
        m_state |= STATE_DISCONNECTED;
        button->setText(tr("Connect"));
    }
}

void LorrisShupito::readData(const QByteArray &data)
{
    QByteArray dta = data;
    m_shupito->readData(data);
}

void LorrisShupito::sendAndWait(const QByteArray &data)
{
    Q_ASSERT(responseTimer == NULL);

    responseTimer = new QTimer;
    responseTimer->start(1000);
    connect(responseTimer, SIGNAL(timeout()), this, SIGNAL(responseChanged()));

    m_response = RESPONSE_WAITING;

    QEventLoop loop;
    loop.connect(this, SIGNAL(responseChanged()), SLOT(quit()));

    m_con->SendData(data);

    loop.exec();

    delete responseTimer;
    responseTimer = NULL;

    if(m_response == RESPONSE_WAITING)
        m_response = RESPONSE_BAD;
}

void LorrisShupito::responseReceived(char error_code)
{
    if(responseTimer)
        responseTimer->stop();
    if(error_code == 0)
        m_response = RESPONSE_GOOD;
    else
        m_response = RESPONSE_BAD;
    emit responseChanged();
}

void LorrisShupito::onTabShow()
{
    m_shupito->init(m_con, m_desc);
}

void LorrisShupito::descRead()
{
    QPlainTextEdit *edit = findChild<QPlainTextEdit*>("logText");
    edit->appendPlainText("Device GUID: " % m_desc->getGuid());

    ShupitoDesc::intf_map map = m_desc->getInterfaceMap();
    for(ShupitoDesc::intf_map::iterator itr = map.begin(); itr != map.end(); ++itr)
        edit->appendPlainText("Got interface GUID: " % itr->first);

    ShupitoDesc::config *vddConfig = m_desc->getConfig("1d4738a0-fc34-4f71-aa73-57881b278cb1");
    if(vddConfig)
    {
        if(!vddConfig->always_active())
        {
            sendAndWait(vddConfig->getStateChangeCmd(true).getData(false));
            if(m_response == RESPONSE_GOOD)
                edit->appendPlainText("VDD started!");
            else
                edit->appendPlainText("Could not start VDD!");
            m_response = RESPONSE_NONE;
        }
        ShupitoPacket packet(vddConfig->cmd, 2, 0, 0);
        m_con->SendData(packet.getData(false));
    }
}

void LorrisShupito::vccValueChanged(quint8 id, double value)
{
    if(id == 0 && vccText.length() != 0)
    {
        if((value < 0 ? -value : value) < 0.03)
            value = 0;
        m_vcc = value;
        char buff[24];
        sprintf(buff, " %4.2f", value);
        vccLabel->setText(vccText % QString(buff));
    }
}

//void do_vdd_setup(avrflash::device<comm>::vdd_setup const & vs)
void LorrisShupito::vddSetup(const vdd_setup &vs)
{
    m_vdd_setup = vs;

    disconnect(vddBox, SIGNAL(currentIndexChanged(int)), 0, 0);

    vddBox->clear();

    if(vs.empty())
    {
        vccText = "";
        return;
    }


    for(quint8 i = 0; i < vs[0].drives.size(); ++i)
        vddBox->addItem(vs[0].drives[i]);
    lastVccIndex = vs[0].current_drive;
    vddBox->setCurrentIndex(vs[0].current_drive);
    vccText = vs[0].name;

    connect(vddBox, SIGNAL(currentIndexChanged(int)), this, SLOT(vddIndexChanged(int)));
}

void LorrisShupito::vddIndexChanged(int index)
{
    if(index == -1)
        return;

    if(lastVccIndex == 0 && index > 0 && m_vcc != 0)
    {
        vddBox->setCurrentIndex(0);

        QMessageBox *box = new QMessageBox(this);
        box->setIcon(QMessageBox::Critical);
        box->setWindowTitle(tr("Error!"));
        box->setText(tr("Can't set output VCC, voltage detected!"));
        box->exec();
        delete box;
        return;
    }

    lastVccIndex = index;

    ShupitoPacket p(MSG_VCC, 3, 1, 2, quint8(index));
    m_con->SendData(p.getData(false));
}
