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

Q_SIGNALS:
    void tabText(const QString& text);

public:
    ChildTab(QWidget *parent = NULL);

    quint32 getParentId() const { return m_parentId; }
    void setParentId(quint32 id) { m_parentId = id; }

    void setTabText(const QString &text)
    {
        emit tabText(text);
    }

protected:
    quint32 m_parentId;

private:
};

#endif // CHILDTAB_H
