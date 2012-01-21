#include "shupito.h"
#include "shupitospi.h"

ShupitoSPI::ShupitoSPI(Shupito *shupito) : ShupitoMode(shupito)
{
}

ShupitoDesc::config *ShupitoSPI::getModeCfg()
{
    return m_shupito->getDesc()->getConfig("46dbc865-b4d0-466b-9b70-2f3f5b264e65");
}


void ShupitoSPI::editIdArgs(QString &id, quint8 &/*id_lenght*/)
{
    id = "avr:";
}
