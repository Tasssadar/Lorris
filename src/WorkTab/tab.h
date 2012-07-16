/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef TAB_H
#define TAB_H

#include <QWidget>

enum tabTypes
{
    TABTYPE_WORKTAB = 0,
    TABTYPE_CHILD,

    TABTYPE_MAX
};

class Tab : public QWidget
{
    Q_OBJECT
Q_SIGNALS:
    void activateMe();

public:
    explicit Tab(quint8 type, QWidget *parent = 0);
    virtual ~Tab();

    quint8 getType() const { return m_type; }
    bool isWorkTab() const { return m_type == TABTYPE_WORKTAB; }
    bool isChild() const { return m_type == TABTYPE_CHILD; }

    void activateTab()
    {
        emit activateMe();
    }

private:
    quint8 m_type;
};

#endif // TAB_H
