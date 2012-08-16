/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <QProgressBar>

#include "../../dep/ecwin7/ecwin7.h"

class ProgressBar : public QProgressBar
{
    Q_OBJECT
public:
    explicit ProgressBar(QWidget *parent = 0);

    void setWindowId(WId id) {
        m_win7.init(id);
    }

public slots:
    void setValue(int value);
    void hide();

private:
    EcWin7 m_win7;
};

#endif // PROGRESSBAR_H
