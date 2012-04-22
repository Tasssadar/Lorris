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

#include "../WorkTab/WorkTab.h"
#include "../WorkTab/WorkTabInfo.h"

ShupitoTunnel::ShupitoTunnel()
{
    m_shupito = NULL;
    dataSigConnected = false;

    m_type = CONNECTION_SHUPITO_TUNNEL;
}

ShupitoTunnel::~ShupitoTunnel()
{
}

void ShupitoTunnel::removeFromMgr()
{
    sConMgr.RemoveCon(CONNECTION_SHUPITO, this);
}

bool ShupitoTunnel::Open()
{
    if(!m_shupito)
        return false;

    if(this->isOpen())
        return true;

    this->SetOpen(true);

    if(!dataSigConnected)
    {
        connect(m_shupito, SIGNAL(tunnelData(QByteArray)), this, SIGNAL(dataRead(QByteArray)));
        dataSigConnected = true;
    }
    return true;
}

void ShupitoTunnel::Close()
{
    if(!this->isOpen())
        return;

    disconnect(m_shupito, SIGNAL(tunnelData(QByteArray)), this, NULL);

    dataSigConnected = false;
    this->SetOpen(false);
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
    else if(!this->isOpen())
        Close();

    m_shupito = s;
}

void ShupitoTunnel::SendData(const QByteArray &data)
{
    if(!this->isOpen() || !m_shupito)
        return;

    m_shupito->sendTunnelData(data);
}
