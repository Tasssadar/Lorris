/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <QProgressBar>

class ProgressBar : public QProgressBar
{
    Q_OBJECT
public:
    explicit ProgressBar(QWidget *parent = 0);

    void setValue(int value);

protected:
    void hideEvent(QHideEvent *ev);
};

#endif // PROGRESSBAR_H
