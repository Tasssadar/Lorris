/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "qtobjectpointer.h"

QtObjectHolder::QtObjectHolder(QObject *p) : QObject()
{
    m_ptr = NULL;
    set(p);
}

void QtObjectHolder::set(QObject *p)
{
    if(m_ptr)
        disconnect(m_ptr, SIGNAL(destroyed()), this, SLOT(objDestroyed()));

    m_ptr = p;

    if(m_ptr)
        connect(m_ptr, SIGNAL(destroyed()), SLOT(objDestroyed()));
}

void QtObjectHolder::objDestroyed()
{
    disconnect(m_ptr, SIGNAL(destroyed()), this, SLOT(objDestroyed()));
    m_ptr = NULL;
}
