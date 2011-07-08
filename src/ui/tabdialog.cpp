#include <QHBoxLayout>
#include <QComboBox>
#include <QFileDialog>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

#include "qserialdeviceenumerator/serialdeviceenumerator.h"
#include "qserialdevice/abstractserial.h"
#include "WorkTabMgr.h"
#include "WorkTabInfo.h"
#include "WorkTab.h"
#include "tabdialog.h"
#include "mainwindow.h"
#include "connection/serialport.h"

TabDialog::TabDialog(QWidget *parent) : QDialog(parent, Qt::WindowFlags(0))
{
    m_sde = SerialDeviceEnumerator::instance();
    setFixedSize(500, 200);

    firstLine = new QHBoxLayout;
    secondLine = new QHBoxLayout;

    pluginsBox = new QComboBox;
    std::vector<WorkTabInfo*> *tabs = sWorkTabMgr.GetWorkTabInfos();
    for(std::vector<WorkTabInfo*>::iterator i = tabs->begin(); i != tabs->end(); ++i)
        pluginsBox->addItem((*i)->GetName());
    connect(pluginsBox, SIGNAL(currentIndexChanged(int)), this, SLOT(PluginSelected(int)));

    conBox = new QComboBox;
    connect(conBox, SIGNAL(currentIndexChanged(int)), this, SLOT(FillConOptions(int)));
    PluginSelected(pluginsBox->currentIndex());

    QPushButton *create = new QPushButton("Create", NULL);
    connect(create, SIGNAL(clicked()), this, SLOT(CreateTab()));

    firstLine->addWidget(pluginsBox);
    firstLine->addWidget(conBox);

    layout = new QVBoxLayout(this);
    layout->addLayout(firstLine);
    layout->addLayout(secondLine);
    layout->addWidget(create);
    setLayout(layout);
}

TabDialog::~TabDialog()
{
    WorkTab::DeleteAllMembers(layout);
    delete layout;
    m_sde->setEnabled(false);
    SerialDeviceEnumerator::destroyInstance();
}

void TabDialog::PluginSelected(int index)
{
    conBox->clear();
    std::vector<WorkTabInfo*> *tabs = sWorkTabMgr.GetWorkTabInfos();
    uint8_t conn = tabs->at(index)->GetConType();

    if(conn & CON_MSK(CONNECTION_SOCKET))      conBox->addItem("Socket");
    if(conn & CON_MSK(CONNECTION_SERIAL_PORT)) conBox->addItem("Serial port");
    if(conn & CON_MSK(CONNECTION_FILE))        conBox->addItem("File");
}

void TabDialog::FillConOptions(int index)
{
    WorkTab::DeleteAllMembers(secondLine);

    if(conBox->itemText(index) == "Socket")
    {

    }
    else if(conBox->itemText(index) == "Serial port")
    {
        QLabel *portLabel = new QLabel("Port: ", NULL, Qt::WindowFlags(0));
        QComboBox *portBox = new QComboBox(NULL);
        portBox->setEditable(true);
        portBox->setObjectName("PortBox");
        foreach (QString s, m_sde->devicesAvailable())
        {
            m_sde->setDeviceName(s);
            portBox->addItem(m_sde->name(), m_sde->name());
        }
        secondLine->addWidget(portLabel);
        secondLine->addWidget(portBox);

        QLabel *rateLabel = new QLabel("Baud Rate: ", NULL, Qt::WindowFlags(0));
        QComboBox *rateBox = new QComboBox(NULL);
        rateBox->addItem("38400",   AbstractSerial::BaudRate38400);
        rateBox->addItem("19200",   AbstractSerial::BaudRate19200);
        rateBox->addItem("57600",   AbstractSerial::BaudRate57600);
        rateBox->addItem("115200",  AbstractSerial::BaudRate115200);
        rateBox->addItem("500000",  AbstractSerial::BaudRate500000);
        rateBox->addItem("1000000", AbstractSerial::BaudRate1000000);
        rateBox->addItem("1500000", AbstractSerial::BaudRate1500000);
        rateBox->addItem("2000000", AbstractSerial::BaudRate2000000);
        rateBox->setObjectName("RateBox");
        secondLine->addWidget(rateLabel);
        secondLine->addWidget(rateBox);
    }
    else if(conBox->itemText(index) == "File")
    {

    }

}

void TabDialog::CreateTab()
{
    WorkTabInfo *info = sWorkTabMgr.GetWorkTabInfos()->at(pluginsBox->currentIndex());
    WorkTab *tab = info->GetNewTab();
    if(conBox->itemText(conBox->currentIndex()) == "Serial port")
        if(!ConnectSP(tab))
            return;
    ((MainWindow*)parent())->AddTab(tab, info->GetName());
    close();
}

bool TabDialog::ConnectSP(WorkTab *tab)
{
    QString portName("");
    AbstractSerial::BaudRate rate;

    for(uint8_t i = 0; i < secondLine->count(); ++i)
    {
        QWidget *wi = secondLine->itemAt(i)->widget();
        if(wi->objectName() == "PortBox")
            portName = ((QComboBox*)wi)->currentText();
        if(wi->objectName() == "RateBox")
            rate = AbstractSerial::BaudRate(((QComboBox*)wi)->itemData(((QComboBox*)wi)->currentIndex()).Int);
    }

    SerialPort *port = (SerialPort*)sConMgr.FindConnection(CONNECTION_SERIAL_PORT, portName);
    if(port)
    {
        sWorkTabMgr.AddWorkTab(tab);
        tab->setConnection(port);
    }
    else
    {
        port = new SerialPort();
        port->SetNameAndRate(portName, rate);
        if(port->Open())
        {
            sWorkTabMgr.AddWorkTab(tab);
            tab->setConnection(port);
            sConMgr.AddCon(CONNECTION_SERIAL_PORT, port);
        }
        else
        {
            QMessageBox *box = new QMessageBox(this);
            box->setWindowTitle("Error!");
            box->setText("Error opening " + portName);
            box->exec();
            delete box;
            delete tab;
            delete port;
            return false;
        }
    }
    return true;
}

