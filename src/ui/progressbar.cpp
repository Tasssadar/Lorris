/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/
#include <QHideEvent>
#include <QApplication>

#include "progressbar.h"
#include "../misc/utils.h"

ProgressBar::ProgressBar(QWidget *parent) :
    QProgressBar(parent)
{
}

void ProgressBar::setValue(int value)
{
    if(value == this->value())
        return;

    if(value == -1)
        m_win7.setProgressState(EcWin7::NoProgress);
    else
    {
        m_win7.setProgressState(EcWin7::Normal);
        m_win7.setProgressValue(value, maximum());
    }
    QProgressBar::setValue(value);
}

void ProgressBar::hide()
{
    QProgressBar::hide();
    m_win7.setProgressState(EcWin7::NoProgress);
}
