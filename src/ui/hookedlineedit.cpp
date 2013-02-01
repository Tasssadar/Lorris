/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QKeyEvent>
#include "hookedlineedit.h"

HookedLineEdit::HookedLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
}

void HookedLineEdit::keyPressEvent(QKeyEvent *ev)
{
    emit keyPressed(ev->key());
    return QLineEdit::keyPressEvent(ev);
}

void HookedLineEdit::keyReleaseEvent(QKeyEvent *ev)
{
    emit keyReleased(ev->key());
    return QLineEdit::keyReleaseEvent(ev);
}
