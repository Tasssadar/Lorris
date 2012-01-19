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

    connect(ui->connectButton, SIGNAL(clicked()), SLOT(connectButton()));
    connect(ui->tunnelSpeedBox, SIGNAL(editTextChanged(QString)), SLOT(tunnelSpeedChanged(QString)));
    connect(ui->tunnelCheck, SIGNAL(clicked(bool)), SLOT(tunnelToggled(bool)));

    vccLabel = findChild<QLabel*>("vccLabel");
    vddBox = findChild<QComboBox*>("vddBox");

    m_shupito = new Shupito(this);
    m_desc = new ShupitoDesc();

    connect(m_shupito, SIGNAL(descRead()), this, SLOT(descRead()));
    connect(m_shupito, SIGNAL(responseReceived(char)), this, SLOT(responseReceived(char)));
    connect(m_shupito, SIGNAL(vccValueChanged(quint8,double)), this, SLOT(vccValueChanged(quint8,double)));
    connect(m_shupito, SIGNAL(vddDesc(vdd_setup)), this, SLOT(vddSetup(vdd_setup)));
    connect(m_shupito, SIGNAL(tunnelStatus(bool)), this, SLOT(tunnelStateChanged(bool)));

    m_shupito->setTunnelSpeed(ui->tunnelSpeedBox->itemText(0).toInt(), false);

    m_response = RESPONSE_NONE;

    responseTimer = NULL;
    m_vcc = 0;
    lastVccIndex = 0;

    m_vdd_config = NULL;
    m_tunnel_config = NULL;
}

LorrisShupito::~LorrisShupito()
{
    stopAll();
    delete m_shupito;
    delete m_desc;
    delete ui;
}

void LorrisShupito::connectButton()
{
    if(m_state & STATE_DISCONNECTED)
    {
        ui->connectButton->setText(tr("Connecting..."));
        ui->connectButton->setEnabled(false);
        connect(m_con, SIGNAL(connectResult(Connection*,bool)), this, SLOT(connectionResult(Connection*,bool)));
        m_con->OpenConcurrent();
    }
    else
    {
        stopAll();
        m_con->Close();
        m_state |= STATE_DISCONNECTED;

        ui->connectButton->setText(tr("Connect"));
    }
}

void LorrisShupito::connectionResult(Connection */*con*/,bool result)
{
    disconnect(m_con, SIGNAL(connectResult(Connection*,bool)), this, 0);

    ui->connectButton->setEnabled(true);
    if(!result)
    {
        ui->connectButton->setText(tr("Connect"));

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
    if(connected)
    {
        m_state &= ~(STATE_DISCONNECTED);
        ui->connectButton->setText(tr("Disconnect"));
        stopAll();
        m_shupito->init(m_con, m_desc);
    }
    else
    {
        m_state |= STATE_DISCONNECTED;
        ui->connectButton->setText(tr("Connect"));  
    }
    ui->tunnelCheck->setEnabled(connected);
    ui->tunnelSpeedBox->setEnabled(connected);
    ui->vddBox->setEnabled(connected);
}

void LorrisShupito::readData(const QByteArray &data)
{
    QByteArray dta = data;
    m_shupito->readData(data);
}

void LorrisShupito::stopAll()
{
    if(m_tunnel_config)
    {
        m_shupito->setTunnelState(false);

        if(!m_tunnel_config->always_active())
        {
            sendAndWait(m_tunnel_config->getStateChangeCmd(false).getData(false));
            m_response = RESPONSE_NONE;
        }
    }

    if(m_vdd_config && !m_vdd_config->always_active())
    {
        sendAndWait(m_vdd_config->getStateChangeCmd(false).getData(false));
        m_response = RESPONSE_NONE;
    }
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
   // m_shupito->init(m_con, m_desc);

}

void LorrisShupito::descRead()
{
    ui->logText->appendPlainText("Device GUID: " % m_desc->getGuid());

    ShupitoDesc::intf_map map = m_desc->getInterfaceMap();
    for(ShupitoDesc::intf_map::iterator itr = map.begin(); itr != map.end(); ++itr)
        ui->logText->appendPlainText("Got interface GUID: " % itr->first);

    m_vdd_config = m_desc->getConfig("1d4738a0-fc34-4f71-aa73-57881b278cb1");
    m_shupito->setVddConfig(m_vdd_config);
    if(m_vdd_config)
    {
        if(!m_vdd_config->always_active())
        {
            sendAndWait(m_vdd_config->getStateChangeCmd(true).getData(false));
            if(m_response == RESPONSE_GOOD)
                ui->logText->appendPlainText("VDD started!");
            else
                ui->logText->appendPlainText("Could not start VDD!");
            m_response = RESPONSE_NONE;
        }
        ShupitoPacket packet(m_vdd_config->cmd, 2, 0, 0);
        m_con->SendData(packet.getData(false));
    }

    m_tunnel_config = m_desc->getConfig("356e9bf7-8718-4965-94a4-0be370c8797c");
    m_shupito->setTunnelConfig(m_tunnel_config);
    if(m_tunnel_config)
    {
        if(!m_tunnel_config->always_active())
        {
            sendAndWait(m_tunnel_config->getStateChangeCmd(true).getData(false));
            if(m_response == RESPONSE_GOOD)
                ui->logText->appendPlainText("Tunnel started!");
            else
                ui->logText->appendPlainText("Could not start tunnel!");
            m_response = RESPONSE_NONE;
        }

        m_shupito->setTunnelState(true);
    }else
        ui->tunnelCheck->setChecked(false);
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

void LorrisShupito::tunnelSpeedChanged(const QString &text)
{
    bool ok = false;
    quint32 speed = 0;

    if(text.length() != 0 && text.length() < 8)
        speed = text.toInt(&ok);

    if(ok)
    {
        ui->tunnelSpeedBox->setStyleSheet("");
        m_shupito->setTunnelSpeed(speed);
    }
    else
        ui->tunnelSpeedBox->setStyleSheet("background-color: #FF7777");
}

void LorrisShupito::tunnelToggled(bool enable)
{
    if(!m_tunnel_config)
    {
        if(enable)
        {
            QMessageBox box(this);
            box.setIcon(QMessageBox::Information);
            box.setWindowTitle(tr("Unsupported"));
            box.setText(tr("It looks like your Shupito does not support RS232 tunnel!"));
            box.exec();
        }
        return;
    }
    m_shupito->setTunnelState(enable);
}

void LorrisShupito::tunnelStateChanged(bool opened)
{
    if(!m_tunnel_config)
        return;



    QString text = tr("RS232 tunnel ");
    if(opened)
        text += tr("enabled");
    else
        text += tr("disabled");

    ui->logText->appendPlainText(text);
    ui->tunnelCheck->setChecked(opened);
}
