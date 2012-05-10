/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SHUPITOPDI_H
#define SHUPITOPDI_H

#include "shupitomode.h"

class ShupitoPDI : public ShupitoMode
{
    Q_OBJECT
public:
    ShupitoPDI(Shupito *shupito);

protected:
    ShupitoDesc::config *getModeCfg();
    void editIdArgs(QString& id, quint8& id_lenght);
};

#endif // SHUPITOPDI_H


