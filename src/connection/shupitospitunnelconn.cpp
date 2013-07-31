/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "shupitospitunnelconn.h"
#include "../LorrisProgrammer/shupito.h"

QString ShupitoSpiTunnelConn::getCompanionName()
{
    return "spitunnel";
}

ShupitoSpiTunnelConn::ShupitoSpiTunnelConn() :
    PortConnection(CONNECTION_SHUPITO_SPI_TUNNEL)
{
    m_shupito = NULL;
}

void ShupitoSpiTunnelConn::setShupito(Shupito *shupito)
{
    m_shupito = shupito;
    if(!shupito)
        Close();
}

void ShupitoSpiTunnelConn::SendData(const QByteArray &data)
{
    if(isOpen())
        emit writeSpiData(data);
}

void ShupitoSpiTunnelConn::doOpen()
{
    if(m_shupito)
        SetState(st_connecting);
}

void ShupitoSpiTunnelConn::doClose()
{
    if(m_shupito)
        SetState(st_disconnecting);
    else
        SetState(st_disconnected);
}

void ShupitoSpiTunnelConn::spiDataRead(const QByteArray &data)
{
    if(isOpen())
        emit dataRead(data);
}

void ShupitoSpiTunnelConn::spiStateSwitchComplete(bool success)
{
    switch(state())
    {
        case st_connecting:
            SetState(success ? st_connected : st_disconnected);
            break;
        case st_disconnecting:
            SetState(st_disconnected);
            break;
        default:
            Q_ASSERT(false);
            break;
    }
}
