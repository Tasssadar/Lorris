#include <QHBoxLayout>
#include <QComboBox>
#include <QFileDialog>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

#include "WorkTab/WorkTabMgr.h"
#include "WorkTab/WorkTabInfo.h"
#include "WorkTab/WorkTab.h"
#include "tabdialog.h"
#include "mainwindow.h"
#include "connection/serialport.h"

TabDialog::TabDialog(QWidget *parent) : QDialog(parent, Qt::WindowFlags(0))
{
    setFixedSize(600, 250);

    columns = new QHBoxLayout;
    secondCol = new QVBoxLayout;
    conOptions = new QHBoxLayout;

    pluginsBox = new QListWidget(this);
    pluginsBox->setFixedWidth(150);
    pluginsBox->setSelectionMode(QAbstractItemView::SingleSelection);
    std::vector<WorkTabInfo*> *tabs = sWorkTabMgr.GetWorkTabInfos();
    for(std::vector<WorkTabInfo*>::iterator i = tabs->begin(); i != tabs->end(); ++i)
        pluginsBox->addItem((*i)->GetName());
    connect(pluginsBox, SIGNAL(currentRowChanged(int)), this, SLOT(PluginSelected(int)));

    QLabel *desc = new QLabel(this);
    desc->setObjectName("pluginDesc");
    desc->setFixedWidth(430);
    desc->setWordWrap(true);
    desc->setAlignment(Qt::AlignTop);
    secondCol->addWidget(desc, Qt::AlignTop);

    QHBoxLayout *conLayout = new QHBoxLayout;
    QLabel *qConLabel = new QLabel(tr("Connection: "), this);

    conBox = new QComboBox;
    connect(conBox, SIGNAL(currentIndexChanged(int)), this, SLOT(FillConOptions(int)));
    PluginSelected(0);
    conLayout->addWidget(qConLabel);
    conLayout->addWidget(conBox, 1);

    QPushButton *create = new QPushButton(tr("Create tab"), this);
    create->setObjectName("CreateButton");
    connect(create, SIGNAL(clicked()), this, SLOT(CreateTab()));

    columns->addWidget(pluginsBox);
    secondCol->addLayout(conLayout);

    layout = new QVBoxLayout(this);
    layout->addLayout(columns);
    columns->addLayout(secondCol);
    secondCol->addLayout(conOptions);
    layout->addWidget(create);
    setLayout(layout);
}

TabDialog::~TabDialog()
{
    WorkTab::DeleteAllMembers(layout);
    delete layout; 
}

void TabDialog::PluginSelected(int index)
{
    std::vector<WorkTabInfo*> *tabs = sWorkTabMgr.GetWorkTabInfos();
    quint8 conn = tabs->at(index)->GetConType();

    QLabel *desc = findChild<QLabel *>("pluginDesc");
    desc->setText(tabs->at(index)->GetDescription());

    conBox->clear();
    if(conn & CON_MSK(CONNECTION_SOCKET))      conBox->addItem(tr("Socket"), CONNECTION_SOCKET);
    if(conn & CON_MSK(CONNECTION_SERIAL_PORT)) conBox->addItem(tr("Serial port"), CONNECTION_SERIAL_PORT);
    if(conn & CON_MSK(CONNECTION_FILE))        conBox->addItem(tr("File"), CONNECTION_FILE);
}

void TabDialog::FillConOptions(int index)
{
    WorkTab::DeleteAllMembers(conOptions);

    switch(conBox->itemData(index).toInt())
    {
        case CONNECTION_SOCKET:
        case CONNECTION_FILE:
        default:   // TODO
            return;
        case CONNECTION_SERIAL_PORT:
        {
            QLabel *portLabel = new QLabel(tr("Port: "), NULL, Qt::WindowFlags(0));
            QComboBox *portBox = new QComboBox(this);
            portBox->setEditable(true);
            portBox->setObjectName("PortBox");

            m_sde = SerialDeviceEnumerator::instance();
            foreach (QString s, m_sde->devicesAvailable())
            {
                m_sde->setDeviceName(s);
                portBox->addItem(m_sde->name(), m_sde->name());
            }
            m_sde->setEnabled(false);
            SerialDeviceEnumerator::destroyInstance();
            m_sde = NULL;

            conOptions->addWidget(portLabel);
            conOptions->addWidget(portBox);

            QLabel *rateLabel = new QLabel(tr("Baud Rate: "), NULL, Qt::WindowFlags(0));
            QComboBox *rateBox = new QComboBox(this);

            rateBox->addItem("38400",   AbstractSerial::BaudRate38400);
            rateBox->addItem("9600",    AbstractSerial::BaudRate9600);
            rateBox->addItem("19200",   AbstractSerial::BaudRate19200);
            rateBox->addItem("57600",   AbstractSerial::BaudRate57600);
            rateBox->addItem("115200",  AbstractSerial::BaudRate115200);
            rateBox->addItem("500000",  AbstractSerial::BaudRate500000);
            rateBox->addItem("1000000", AbstractSerial::BaudRate1000000);
            rateBox->addItem("1500000", AbstractSerial::BaudRate1500000);
            rateBox->addItem("2000000", AbstractSerial::BaudRate2000000);
            rateBox->setObjectName("RateBox");
            conOptions->addWidget(rateLabel);
            conOptions->addWidget(rateBox);
            break;
        }
    }
}

void TabDialog::CreateTab()
{
    WorkTabInfo *info = sWorkTabMgr.GetWorkTabInfos()->at(pluginsBox->currentIndex().row());
    WorkTab *tab = NULL;

    switch(conBox->itemData(conBox->currentIndex()).toInt())
    {
        case CONNECTION_SERIAL_PORT:
            tab = ConnectSP(info);
            if(!tab)
                return;
            break;
        default:    // TODO: other connection types
            sWorkTabMgr.AddWorkTab(info->GetNewTab(), info->GetName());
            close();
            return;
    }
    close();
}

WorkTab *TabDialog::ConnectSP(WorkTabInfo *info)
{
    QString portName("");
    AbstractSerial::BaudRate rate;

    QComboBox *combo = findChild<QComboBox *>("PortBox");
    portName = combo->currentText();
    combo = findChild<QComboBox *>("RateBox");
    rate =  AbstractSerial::BaudRate(combo->itemData(combo->currentIndex()).toInt());

    SerialPort *port = (SerialPort*)sConMgr.FindConnection(CONNECTION_SERIAL_PORT, portName);
    if(port && port->isOpen())
    {
        WorkTab *tab = info->GetNewTab();
        sWorkTabMgr.AddWorkTab(tab, info->GetName() + " - " +port->GetIDString());
        tab->setConnection(port);
        return tab;
    }
    else
    {
        QPushButton *create = findChild<QPushButton *>("CreateButton");
        create->setText(tr("Connecting..."));
        create->setEnabled(false);

        tmpTabInfo = info;

        if(!port)
        {
            port = new SerialPort();
            port->SetNameAndRate(portName, rate);
        }

        connect(port, SIGNAL(connectResult(Connection*,bool)), this, SLOT(serialConResult(Connection*,bool)));
        port->OpenConcurrent();
        return NULL;
    }
}

void TabDialog::serialConResult(Connection *con, bool result)
{
    QPushButton *create = findChild<QPushButton *>("CreateButton");
    disconnect(con, SIGNAL(connectResult(Connection*,bool)), this, 0);

    if(result)
    {
        WorkTab *tab = tmpTabInfo->GetNewTab();
        sWorkTabMgr.AddWorkTab(tab, tmpTabInfo->GetName() + " - " + con->GetIDString());
        tab->setConnection(con);
        sConMgr.AddCon(CONNECTION_SERIAL_PORT, con);
        close();
    }
    else
    {
        create->setText(tr("Create tab"));
        create->setEnabled(true);

        QMessageBox *box = new QMessageBox(this);
        box->setWindowTitle(tr("Error!"));
        box->setText(tr("Error opening serial port!"));
        box->setIcon(QMessageBox::Critical);
        box->exec();
        delete box;
        delete con;
    }
    tmpTabInfo = NULL;
}

