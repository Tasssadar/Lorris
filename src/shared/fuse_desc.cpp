/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "fuse_desc.h"

fuse_desc::fuse_desc(const QString& name, const QStringList &chipSigns, const QString &desc)
{
    m_name = name;
    m_chipIds = chipSigns;
    m_desc = desc;
}
