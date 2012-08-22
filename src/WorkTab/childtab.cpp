/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "childtab.h"

ChildTab::ChildTab(QWidget *parent) : Tab(TABTYPE_CHILD, parent)
{
    m_parentId = 0;
}
