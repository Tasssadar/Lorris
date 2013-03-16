/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <algorithm>

#include "../shupito.h"
#include "shupitopdi.h"

ShupitoPDI::ShupitoPDI(Shupito *shupito)
    : ShupitoModeCommon(shupito)
{
}

ShupitoDesc::config const *ShupitoPDI::getModeCfg()
{
    return m_shupito->getDesc()->getConfig("71efb903-3030-4fd3-8896-1946aba37efc");
}

void ShupitoPDI::editIdArgs(QString &id, quint8 &id_lenght)
{
    id = "avr:";
    id_lenght = std::min(id_lenght, (quint8)3);
}

