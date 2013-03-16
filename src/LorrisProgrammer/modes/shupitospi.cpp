/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "../shupito.h"
#include "shupitospi.h"

ShupitoSPI::ShupitoSPI(Shupito *shupito)
    : ShupitoModeCommon(shupito)
{
}

ShupitoDesc::config const *ShupitoSPI::getModeCfg()
{
    return m_shupito->getDesc()->getConfig("46dbc865-b4d0-466b-9b70-2f3f5b264e65");
}


void ShupitoSPI::editIdArgs(QString &id, quint8 &/*id_lenght*/)
{
    id = "avr:";
}
