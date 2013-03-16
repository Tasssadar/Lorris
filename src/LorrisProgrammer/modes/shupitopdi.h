/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SHUPITOPDI_H
#define SHUPITOPDI_H

#include "shupitomode.h"

class ShupitoPDI : public ShupitoModeCommon
{
    Q_OBJECT
public:
    ShupitoPDI(Shupito *shupito);

protected:
    ShupitoDesc::config const *getModeCfg() override;
    void editIdArgs(QString& id, quint8& id_length) override;
};

#endif // SHUPITOPDI_H


