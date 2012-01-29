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

#include "shupitotunnel.h"
#include "connectionmgr.h"
#include "../LorrisShupito/shupito.h"

ShupitoTunnel::ShupitoTunnel()
{
    m_type = CONNECTION_SHUPITO;
    m_shupito = NULL;
    tunnelActive = true;
    dataSigConnected = false;
}

ShupitoTunnel::~ShupitoTunnel()
{

}

bool ShupitoTunnel::Open()
{
    if(!tunnelActive || !m_shupito)
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
    emit connected(false);
    disconnect(m_shupito, SIGNAL(tunnelData(QByteArray)), this, NULL);
    dataSigConnected = false;
    opened = false;
}

void ShupitoTunnel::OpenConcurrent()
{
    emit connectResult(this, Open());
}

void ShupitoTunnel::setShupito(Shupito* s, QString id)
{
    if(m_shupito)
    {
        disconnect(m_shupito, SIGNAL(tunnelData(QByteArray)), this, NULL);
        disconnect(m_shupito, SIGNAL(tunnelStatus(bool)), this, NULL);
    }

    if(s)
    {
        connect(s, SIGNAL(tunnelData(QByteArray)), this, SIGNAL(dataRead(QByteArray)));
        connect(s, SIGNAL(tunnelStatus(bool)), this, SLOT(tunnelStatus(bool)));
        dataSigConnected = true;
    }
    else if(opened)
        Close();

    m_shupito = s;
    m_idString = id;
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

void ShupitoTunnel::tunnelStatus(bool opened)
{
    if(opened && !this->opened && !dataSigConnected)
    {
        connect(m_shupito, SIGNAL(tunnelData(QByteArray)), this, SIGNAL(dataRead(QByteArray)));
        dataSigConnected = true;
    }
    else if(!opened && this->opened)
    {
        dataSigConnected = false;
        disconnect(m_shupito, SIGNAL(tunnelData(QByteArray)), this, NULL);
    }
    this->opened = opened;
    tunnelActive = opened;
    emit connected(opened);
}

