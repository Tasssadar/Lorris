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
#include "connection/fileconnection.h"
#include "connection/shupitotunnel.h"
#include "LorrisShupito/shupito.h"

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

    pluginsBox->setCurrentRow(0);
}

TabDialog::~TabDialog()
{
   // WorkTab::DeleteAllMembers(layout);
    //delete layout;
}

void TabDialog::PluginSelected(int index)
{
    std::vector<WorkTabInfo*> *tabs = sWorkTabMgr.GetWorkTabInfos();
    quint8 conn = tabs->at(index)->GetConType();

    QLabel *desc = findChild<QLabel *>("pluginDesc");
    desc->setText(tabs->at(index)->GetDescription());

    conBox->clear();
    if(conn & CON_MSK(CONNECTION_SOCKET))      conBox->addItem(tr("Socket"), CONNECTION_SOCKET);
    if(conn & CON_MSK(CONNECTION_FILE))        conBox->addItem(tr("None (Load data from File)"), CONNECTION_FILE);
    if(conn & CON_MSK(CONNECTION_SERIAL_PORT))
    {
        conBox->addItem(tr("Serial port"), CONNECTION_SERIAL_PORT);
        if(sConMgr.isAnyShupito())
            conBox->addItem(tr("Shupito tunnel"), CONNECTION_SHUPITO);
    }
}

void TabDialog::FillConOptions(int index)
{
    WorkTab::DeleteAllMembers(conOptions);

    switch(conBox->itemData(index).toInt())
    {
        case CONNECTION_SOCKET:
        case CONNECTION_FILE: // Nothing to do
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
        case CONNECTION_SHUPITO:
        {
            QLabel *portLabel = new QLabel(tr("Port: "), NULL, Qt::WindowFlags(0));
            QComboBox *portBox = new QComboBox(this);
            portBox->setObjectName("PortBox");

            QStringList shupitoList;
            sConMgr.GetShupitoIds(shupitoList);
            portBox->addItems(shupitoList);

            conOptions->addWidget(portLabel);
            conOptions->addWidget(portBox);
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
        {
            tab = ConnectSP(info);
            if(!tab)
                return;
            tab->onTabShow();
            break;
        }
        case CONNECTION_FILE:
        {
            tab = info->GetNewTab();
            FileConnection *con = (FileConnection*)sConMgr.FindConnection(CONNECTION_FILE, "");
            if(!con)
            {
                con = new FileConnection();
                sConMgr.AddCon(CONNECTION_FILE, con);
            }
            tab->setConnection(con);
            sWorkTabMgr.AddWorkTab(tab, info->GetName());
            close();
            tab->onTabShow();
            break;
        }
        case CONNECTION_SHUPITO:
        {
            tab = ConnectShupito(info);
            if(!tab)
                return;
            tab->onTabShow();
            break;
        }
        default:    // TODO: other connection types
        {
            tab = info->GetNewTab();
            sWorkTabMgr.AddWorkTab(tab, info->GetName());
            close();
            tab->onTabShow();
            return;
        }
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
        tab->onTabShow();
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

WorkTab *TabDialog::ConnectShupito(WorkTabInfo *info)
{
    QString portName = findChild<QComboBox *>("PortBox")->currentText();

    Shupito *shupito = sConMgr.GetShupito(portName);
    if(!shupito)
        return NULL;

    ShupitoTunnel *tunnel = (ShupitoTunnel*)sConMgr.FindConnection(CONNECTION_SHUPITO, portName);
    if(!tunnel)
    {
        tunnel = new ShupitoTunnel();
        tunnel->setShupito(shupito, portName);
        if(!tunnel->Open())
        {
            delete tunnel;
            return NULL;
        }
        sConMgr.AddCon(CONNECTION_SHUPITO, tunnel);
    }
    else if(!tunnel->isOpen() && !tunnel->Open())
        return NULL;

    WorkTab *tab = info->GetNewTab();
    sWorkTabMgr.AddWorkTab(tab, info->GetName() + " - " + tunnel->GetIDString());
    tab->setConnection(tunnel);
    return tab;
}
