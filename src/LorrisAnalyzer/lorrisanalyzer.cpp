#include <QDockWidget>
#include <QMdiArea>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QListView>
#include <QStringListModel>
#include <QSpinBox>
#include <QSlider>

#include "lorrisanalyzer.h"
#include "sourcedialog.h"
#include "ui_lorrisanalyzer.h"
#include "packet.h"
#include "analyzerdatastorage.h"
#include "devicetabwidget.h"
#include "analyzerdataarea.h"
#include "connection/connectionmgr.h"
#include "DataWidgets/datawidget.h"
#include "DataWidgets/numberwidget.h"

LorrisAnalyzer::LorrisAnalyzer() : WorkTab(),ui(new Ui::LorrisAnalyzer)
{
    ui->setupUi(this);

    m_storage = new AnalyzerDataStorage();

    QPushButton *connectButton = findChild<QPushButton*>("connectButton");
    connect(connectButton, SIGNAL(clicked()), this, SLOT(connectButton()));

    QPushButton *addSourceButton = findChild<QPushButton*>("addSourceButton");
    connect(addSourceButton, SIGNAL(clicked()), this, SLOT(onTabShow()));

    QPushButton *saveDataButton = findChild<QPushButton*>("saveDataButton");
    connect(saveDataButton, SIGNAL(clicked()), m_storage, SLOT(SaveToFile()));

    QPushButton *loadDataButton = findChild<QPushButton*>("loadDataButton");
    connect(loadDataButton, SIGNAL(clicked()), this, SLOT(loadDataButton()));

    timeSlider = findChild<QSlider*>("timeSlider");
    connect(timeSlider, SIGNAL(valueChanged(int)), this, SLOT(timeSliderMoved(int)));

    m_dev_tabs = new DeviceTabWidget(this);
    QVBoxLayout *leftVLayout = findChild<QVBoxLayout*>("leftVLayout");
    leftVLayout->insertWidget(1, m_dev_tabs);

    connect(this, SIGNAL(newData(analyzer_data*)), m_dev_tabs, SLOT(handleData(analyzer_data*)));
    connect(m_dev_tabs, SIGNAL(updateData()), this, SLOT(updateData()));

    m_data_area = new AnalyzerDataArea(this);
    QHBoxLayout *bottomHLayout = findChild<QHBoxLayout*>("bottomHLayout");
    bottomHLayout->insertWidget(0, m_data_area, 5);
    bottomHLayout->setStretch(1, 1);
    connect(m_data_area, SIGNAL(updateData()), this, SLOT(updateData()));

    QScrollArea *widgetsScrollArea = findChild<QScrollArea*>("widgetsScrollArea");
    QWidget *tmp = new QWidget(this);
    new NumberWidgetAddBtn(tmp);
    widgetsScrollArea->setWidget(tmp);

    m_packet = NULL;
    m_state = 0;
}

LorrisAnalyzer::~LorrisAnalyzer()
{
    delete ui;
    delete m_storage;
    if(m_packet)
    {
        delete m_packet->header;
        delete m_packet;
    }
    delete m_dev_tabs;
}

void LorrisAnalyzer::connectButton()
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

void LorrisAnalyzer::connectionResult(Connection */*con*/,bool result)
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

void LorrisAnalyzer::connectedStatus(bool connected)
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

void LorrisAnalyzer::readData(QByteArray data)
{
    if((m_state & STATE_DIALOG) != 0 || !m_packet)
        return;

    analyzer_data *a_data = m_storage->addData(data);
    if(!a_data)
        return;

    bool update = timeSlider->value() == timeSlider->maximum();
    timeSlider->setMaximum(m_storage->getSize());
    if(update)
        timeSlider->setValue(m_storage->getSize());
}

void LorrisAnalyzer::onTabShow()
{
    if(m_con->getType() == CONNECTION_FILE)
    {
        loadDataButton();
        return;
    }

    m_state |= STATE_DIALOG;
    SourceDialog *d = new SourceDialog(this);
    connect(this->m_con, SIGNAL(dataRead(QByteArray)), d, SLOT(readData(QByteArray)));

    analyzer_packet *packet = d->getStructure();
    delete d;

    if(!packet)
        return;

    if(m_packet)
    {
        delete m_packet->header;
        delete m_packet;
    }
    m_storage->Clear();

    m_dev_tabs->removeAll();
    m_dev_tabs->setHeader(packet->header);
    m_dev_tabs->addDevice();

    m_storage->setPacket(packet);
    m_packet = packet;
    m_state &= ~(STATE_DIALOG);
}

void LorrisAnalyzer::timeSliderMoved(int value)
{
    if(value != 0)
        updateData();
}

void LorrisAnalyzer::updateData()
{
    emit newData(m_storage->get(timeSlider->value()-1));
}

void LorrisAnalyzer::loadDataButton()
{
    analyzer_packet *packet = m_storage->loadFromFile();
    if(!packet)
        return;
    m_packet = packet;

    m_dev_tabs->removeAll();
    m_dev_tabs->setHeader(packet->header);
    m_dev_tabs->addDevice();

    timeSlider->setMaximum(m_storage->getSize());
    timeSlider->setValue(m_storage->getSize());
}
