/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef CHILDTAB_H
#define CHILDTAB_H

#include "tab.h"

class ChildTab : public Tab
{
    Q_OBJECT
public:
    ChildTab(quint32 parentId, QWidget *parent = 0);

    quint32 getParentId() const { return m_parentId; }

protected:
    quint32 m_parentId;

private:
};

#endif // CHILDTAB_H
