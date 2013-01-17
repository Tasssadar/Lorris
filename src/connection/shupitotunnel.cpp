/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QLabel>
#include <QComboBox>

#include "shupitotunnel.h"
#include "../LorrisShupito/shupito.h"

#include "../WorkTab/WorkTab.h"
#include "../WorkTab/WorkTabInfo.h"

ShupitoTunnel::ShupitoTunnel()
    : PortConnection(CONNECTION_SHUPITO_TUNNEL)
{
    m_shupito = NULL;
    dataSigConnected = false;
}

ShupitoTunnel::~ShupitoTunnel()
{
    Close();
}

void ShupitoTunnel::doOpen()
{
    if(m_shupito && !this->isOpen())
    {
        this->SetOpen(true);

        if(!dataSigConnected)
        {
            connect(m_shupito, SIGNAL(tunnelData(QByteArray)), this, SIGNAL(dataRead(QByteArray)));
            dataSigConnected = true;
        }
    }
}

void ShupitoTunnel::doClose()
{
    disconnect(m_shupito, SIGNAL(tunnelData(QByteArray)), this, NULL);
    dataSigConnected = false;
    this->SetState(st_disconnected);
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
    else if(this->isOpen())
        Close();

    m_shupito = s;
}

void ShupitoTunnel::SendData(const QByteArray &data)
{
    if(!this->isOpen() || !m_shupito)
        return;

    m_shupito->sendTunnelData(data);
}
