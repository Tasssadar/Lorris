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
#include <qextserialenumerator.h>
#include <qextserialport.h>
#include <QSpinBox>

#include "common.h"
#include "WorkTab/WorkTabMgr.h"
#include "WorkTab/WorkTabInfo.h"
#include "WorkTab/WorkTab.h"
#include "tabdialog.h"
#include "mainwindow.h"
#include "connection/serialport.h"
#include "connection/fileconnection.h"
#include "connection/shupitotunnel.h"
#include "connection/tcpsocket.h"
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

    WorkTabMgr::InfoList *tabs = sWorkTabMgr.GetWorkTabInfos();
    for(WorkTabMgr::InfoList::iterator i = tabs->begin(); i != tabs->end(); ++i)
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

    quint32 lastSelected = sConfig.get(CFG_QUINT32_TAB_TYPE);
    if(lastSelected >= (quint32)sWorkTabMgr.GetWorkTabInfos()->size())
        lastSelected = 0;
    pluginsBox->setCurrentRow(lastSelected);
}

TabDialog::~TabDialog()
{
   // WorkTab::DeleteAllMembers(layout);
    //delete layout;
}

void TabDialog::PluginSelected(int index)
{
    WorkTabMgr::InfoList *tabs = sWorkTabMgr.GetWorkTabInfos();
    quint8 conn = tabs->at(index)->GetConType();

    QLabel *desc = findChild<QLabel *>("pluginDesc");
    desc->setText(tabs->at(index)->GetDescription());

    conBox->clear();

    if(conn & CON_MSK(CONNECTION_SERIAL_PORT))
    {
        conBox->addItem(tr("Serial port"), CONNECTION_SERIAL_PORT);
        if(sConMgr.isAnyShupito())
            conBox->addItem(tr("Shupito tunnel"), CONNECTION_SHUPITO);
    }

    if(conn & CON_MSK(CONNECTION_TCP_SOCKET))
        conBox->addItem(tr("TCP socket"), CONNECTION_TCP_SOCKET);

    if(conn & CON_MSK(CONNECTION_FILE))
        conBox->addItem(tr("None (Load data from File)"), CONNECTION_FILE);

    quint32 lastConn = sConfig.get(CFG_QUINT32_CONNECTION_TYPE);
    if(lastConn != MAX_CON_TYPE)
    {
        for(quint8 i = 0; i < conBox->count(); ++i)
        {
            if((quint32)conBox->itemData(i).toInt() == lastConn)
            {
                conBox->setCurrentIndex(i);
                break;
            }
        }
    }
}

void TabDialog::FillConOptions(int index)
{
    WorkTab::DeleteAllMembers(conOptions);

    switch(conBox->itemData(index).toInt())
    {
        case CONNECTION_FILE: // Nothing to do
        default:   // TODO
            return;
        case CONNECTION_SERIAL_PORT:
        {
            QLabel *portLabel = new QLabel(tr("Port: "), NULL, Qt::WindowFlags(0));
            QComboBox *portBox = new QComboBox(this);
            portBox->setEditable(true);
            portBox->setObjectName("PortBox");

            QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
            QStringList portNames;
            for (int i = 0; i < ports.size(); i++)
            {
#ifdef Q_OS_WIN
                QString name = ports.at(i).portName;
                name.replace(QRegExp("[^\\w]"), "");
                portNames.push_back(name);
#else
                portNames.push_back(ports.at(i).physName);
#endif
            }
            portNames.sort();
            portBox->addItems(portNames);

            conOptions->addWidget(portLabel);
            conOptions->addWidget(portBox);

            QLabel *rateLabel = new QLabel(tr("Baud Rate: "), NULL, Qt::WindowFlags(0));
            QComboBox *rateBox = new QComboBox(this);

            rateBox->addItem("38400",   BAUD38400);
            rateBox->addItem("2400",    BAUD2400);
            rateBox->addItem("4800",    BAUD4800);
            rateBox->addItem("19200",   BAUD19200);
            rateBox->addItem("57600",   BAUD57600);
            rateBox->addItem("115200",  BAUD115200);

            rateBox->setObjectName("RateBox");
            conOptions->addWidget(rateLabel);
            conOptions->addWidget(rateBox);

            int baud = sConfig.get(CFG_QUINT32_SERIAL_BAUD);
            for(quint8 i = 0; i < rateBox->count(); ++i)
            {
                if(baud == rateBox->itemData(i).toInt())
                {
                    rateBox->setCurrentIndex(i);
                    break;
                }
            }

            QString port = sConfig.get(CFG_STRING_SERIAL_PORT);
            if(port.length() != 0)
                portBox->setEditText(port);

            port = sConfig.get(CFG_STRING_SHUPITO_PORT);
            int idx = pluginsBox->currentIndex().row();
            if(idx >= 0 && idx < (int)sWorkTabMgr.GetWorkTabInfos()->size() && port.length() != 0)
            {
                WorkTabInfo *info = sWorkTabMgr.GetWorkTabInfos()->at(idx);
                if(info->GetName().contains("Shupito", Qt::CaseInsensitive))
                    portBox->setEditText(port);
            }
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
        case CONNECTION_TCP_SOCKET:
        {
            QLabel    *addressLabel = new QLabel(tr("Address:"), this);
            QLineEdit *address      = new QLineEdit(this);
            QLabel    *portLabel    = new QLabel(tr("Port:"), this);
            QSpinBox  *port         = new QSpinBox(this);
            address->setObjectName("tcpAddress");
            port->setObjectName("tcpPort");

            port->setMinimum(0);
            port->setMaximum(65536);
            port->setValue(sConfig.get(CFG_QUINT32_TCP_PORT));

            address->setText(sConfig.get(CFG_STRING_TCP_ADDR));

            conOptions->addWidget(addressLabel);
            conOptions->addWidget(address);
            conOptions->addWidget(portLabel);
            conOptions->addWidget(port);
            break;
        }
    }
}

void TabDialog::CreateTab()
{
    WorkTabInfo *info = sWorkTabMgr.GetWorkTabInfos()->at(pluginsBox->currentIndex().row());
    WorkTab *tab = NULL;

    int connectionType = conBox->itemData(conBox->currentIndex()).toInt();
    switch(connectionType)
    {
        case CONNECTION_SERIAL_PORT:
        {
            tab = ConnectSP(info);
            if(!tab)
                return;
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
            break;
        }
        case CONNECTION_SHUPITO:
        {
            tab = ConnectShupito(info);
            if(!tab)
                return;
            break;
        }
        case CONNECTION_TCP_SOCKET:
        {
            tab = ConnectTcp(info);
            if(!tab)
                return;
            break;
        }
        default:    // TODO: other connection types
        {
            tab = info->GetNewTab();
            sWorkTabMgr.AddWorkTab(tab, info->GetName());
            break;
        }
    }

    sConfig.set(CFG_QUINT32_TAB_TYPE, pluginsBox->currentIndex().row());
    sConfig.set(CFG_QUINT32_CONNECTION_TYPE, connectionType);

    close();
    tab->onTabShow();
}

WorkTab *TabDialog::ConnectSP(WorkTabInfo *info)
{
    QString portName("");
    BaudRateType rate;

    QComboBox *combo = findChild<QComboBox *>("PortBox");
    portName = combo->currentText();
    combo = findChild<QComboBox *>("RateBox");
    rate =  BaudRateType(combo->itemData(combo->currentIndex()).toInt());

    sConfig.set(CFG_STRING_SERIAL_PORT, portName);
    sConfig.set(CFG_QUINT32_SERIAL_BAUD, (quint32)rate);

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

        sConfig.set(CFG_QUINT32_TAB_TYPE, pluginsBox->currentIndex().row());
        sConfig.set(CFG_QUINT32_CONNECTION_TYPE, CONNECTION_SERIAL_PORT);

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
        tunnel->setShupito(shupito);
        tunnel->setIDString(portName);
        if(!tunnel->Open())
        {
            delete tunnel;
            return NULL;
        }
        sConMgr.AddCon(CONNECTION_SHUPITO, tunnel);
    }
    else if(!tunnel->isOpen())
    {
        tunnel->setShupito(shupito);
        if(!tunnel->Open())
            return NULL;
    }

    WorkTab *tab = info->GetNewTab();
    sWorkTabMgr.AddWorkTab(tab, info->GetName() + " - " + tunnel->GetIDString());
    tab->setConnection(tunnel);
    return tab;
}

WorkTab *TabDialog::ConnectTcp(WorkTabInfo *info)
{
    QString address = findChild<QLineEdit*>("tcpAddress")->text();
    quint16 port = findChild<QSpinBox*>("tcpPort")->value();

    sConfig.set(CFG_QUINT32_TCP_PORT, port);
    sConfig.set(CFG_STRING_TCP_ADDR, address);

    TcpSocket *socket =
            (TcpSocket*)sConMgr.FindConnection(CONNECTION_TCP_SOCKET, address + ":" + QString::number(port));
    if(!socket || !socket->isOpen())
    {
        QPushButton *create = findChild<QPushButton *>("CreateButton");
        create->setText(tr("Connecting..."));
        create->setEnabled(false);

        tmpTabInfo = info;

        if(!socket)
        {
            socket = new TcpSocket();
            socket->setAddress(address, port);
        }

        connect(socket, SIGNAL(connectResult(Connection*,bool)),  SLOT(tcpConResult(Connection*,bool)));
        socket->OpenConcurrent();
        return NULL;
    }
    else
    {
        WorkTab *tab = info->GetNewTab();
        sWorkTabMgr.AddWorkTab(tab, info->GetName() + " - " + socket->GetIDString());
        tab->setConnection(socket);
        return tab;
    }
}

void TabDialog::tcpConResult(Connection *con, bool result)
{
    QPushButton *create = findChild<QPushButton *>("CreateButton");
    disconnect(con, SIGNAL(connectResult(Connection*,bool)), this, 0);

    if(result)
    {
        WorkTab *tab = tmpTabInfo->GetNewTab();
        sWorkTabMgr.AddWorkTab(tab, tmpTabInfo->GetName() + " - " + con->GetIDString());
        tab->setConnection(con);
        sConMgr.AddCon(CONNECTION_TCP_SOCKET, con);

        sConfig.set(CFG_QUINT32_TAB_TYPE, pluginsBox->currentIndex().row());
        sConfig.set(CFG_QUINT32_CONNECTION_TYPE, CONNECTION_TCP_SOCKET);

        close();
        tab->onTabShow();
    }
    else
    {
        create->setText(tr("Create tab"));
        create->setEnabled(true);

        QMessageBox *box = new QMessageBox(this);
        box->setWindowTitle(tr("Error!"));
        box->setText(tr("Error tcp socket!"));
        box->setIcon(QMessageBox::Critical);
        box->exec();
        delete box;
        delete con;
    }
    tmpTabInfo = NULL;
}
