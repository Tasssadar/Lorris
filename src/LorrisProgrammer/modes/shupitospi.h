/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SHUPITOSPI_H
#define SHUPITOSPI_H

#include "shupitomode.h"

class ShupitoSPI : public ShupitoModeCommon
{
    Q_OBJECT
public:
    ShupitoSPI(Shupito *shupito);
    ProgrammerCapabilities capabilities() const override;

protected:
    ShupitoDesc::config const *getModeCfg() override;
    void editIdArgs(QString& id, quint8& id_length) override;
};

#endif // SHUPITOSPI_H
