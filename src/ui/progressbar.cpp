/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/
#include <QHideEvent>

#include "progressbar.h"
#include "../utils.h"

ProgressBar::ProgressBar(QWidget *parent) :
    QProgressBar(parent)
{
}

void ProgressBar::setValue(int value)
{
    if(value == this->value())
        return;

    if(value == -1)
        Utils::setProgress(-1);
    else
        Utils::setProgress(value*100/maximum());
    QProgressBar::setValue(value);
}

void ProgressBar::hide()
{
    QProgressBar::hide();
    Utils::setProgress(-1);
}
