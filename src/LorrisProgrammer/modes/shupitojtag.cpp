/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "shupitojtag.h"
#include "../shupito.h"

ShupitoJtag::ShupitoJtag(Shupito *shupito)
    : ShupitoMode(shupito)
{
}

ProgrammerCapabilities ShupitoJtag::capabilities() const
{
    ProgrammerCapabilities caps;
    caps.svf = true;
    return caps;
}

ShupitoDesc::config const * ShupitoJtag::getModeCfg()
{
    return m_shupito->getDesc()->getConfig("ee047e35-dec8-48ab-b194-e3762c8f6b66");
}

void ShupitoJtag::switchToFlashMode(quint32 speed_hz)
{
}

void ShupitoJtag::switchToRunMode()
{
}

chip_definition ShupitoJtag::readDeviceId()
{
    chip_definition cd("jtag:");
    cd.setName("jtag");
    return cd;
}

void ShupitoJtag::erase_device(chip_definition& chip)
{
}

void ShupitoJtag::flashPage(chip_definition::memorydef *memdef, std::vector<quint8>& memory, quint32 address)
{
}

void ShupitoJtag::readMemRange(quint8 memid, QByteArray& memory, quint32 address, quint32 size)
{
}

void ShupitoJtag::executeText(QByteArray const & data, quint8 memId, chip_definition & chip)
{
}
