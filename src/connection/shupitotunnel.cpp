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

#include <QLabel>
#include <QComboBox>

#include "shupitotunnel.h"
#include "connectionmgr.h"
#include "../LorrisShupito/shupito.h"

#include "WorkTab/WorkTabInfo.h"

ShupitoTunnel::ShupitoTunnel()
{
    m_shupito = NULL;
    dataSigConnected = false;

    m_type = CONNECTION_SERIAL_PORT;
}

ShupitoTunnel::~ShupitoTunnel()
{

}

bool ShupitoTunnel::Open()
{
    if(!m_shupito)
        return false;

    if(opened)
        return true;

    opened = true;
    emit connected(true);

    if(!dataSigConnected)
    {
        connect(m_shupito, SIGNAL(tunnelData(QByteArray)), this, SIGNAL(dataRead(QByteArray)));
        dataSigConnected = true;
    }
    return true;
}

void ShupitoTunnel::Close()
{
    if(opened)
        return;

    emit connected(false);

    disconnect(m_shupito, SIGNAL(tunnelData(QByteArray)), this, NULL);

    dataSigConnected = false;
    opened = false;
}

void ShupitoTunnel::OpenConcurrent()
{
    emit connectResult(this, Open());
}

void ShupitoTunnel::setShupito(Shupito* s)
{
    if(m_shupito)
        disconnect(m_shupito, SIGNAL(tunnelData(QByteArray)),     this, NULL);

    if(s)
    {
        connect(s, SIGNAL(tunnelData(QByteArray)),     SIGNAL(dataRead(QByteArray)));
        dataSigConnected = true;
    }
    else if(opened)
        Close();

    m_shupito = s;
}

void ShupitoTunnel::SendData(const QByteArray &data)
{
    if(!opened || !m_shupito)
        return;

    int sent = 0;
    const int data_len = data.length();

    ShupitoPacket packet;
    quint8 cmd = m_shupito->getTunnelCmd();
    char pipe = m_shupito->getTunnelId();

    while(sent != data_len)
    {
        int chunk = data_len - sent;
        if(chunk > 14)
            chunk = 14;

        packet.set(false, cmd, chunk+1);
        packet.getDataRef().append(pipe);
        packet.getDataRef().append(data.mid(sent, chunk));

        m_shupito->sendPacket(packet);

        sent += chunk;
    }
}

void ShupitoTunnelBuilder::addOptToTabDialog(QGridLayout *layout)
{
     QLabel *portLabel = new QLabel(tr("Port: "), m_parent);
     m_portBox = new QComboBox(m_parent);

     QStringList shupitoList;
     sConMgr.GetShupitoIds(shupitoList);
     m_portBox->addItems(shupitoList);

     layout->addWidget(portLabel, 1, 0);
     layout->addWidget(m_portBox, 1, 1);
}

void ShupitoTunnelBuilder::CreateConnection(WorkTabInfo *info)
{
    QString portName = m_portBox->currentText();

    Shupito *shupito = sConMgr.GetShupito(portName);
    if(!shupito)
        return emit connectionFailed(tr("That Shupito does not exist anymore"));

    ShupitoTunnel *tunnel = (ShupitoTunnel*)sConMgr.FindConnection(CONNECTION_SHUPITO, portName);
    if(!tunnel)
    {
        tunnel = new ShupitoTunnel();
        tunnel->setShupito(shupito);
        tunnel->setIDString(portName);
        if(!tunnel->Open())
        {
            delete tunnel;
            return emit connectionFailed(tr("Failed to open tunnel!"));
        }
    }
    else if(!tunnel->isOpen())
    {
        tunnel->setShupito(shupito);
        if(!tunnel->Open())
            return emit connectionFailed(tr("Failed to open tunnel!"));
    }

    emit connectionSucces(tunnel, info->GetName() + " - " + tunnel->GetIDString(), info, CONNECTION_SHUPITO);
}
