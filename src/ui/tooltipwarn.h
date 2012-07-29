/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef TOOLTIPWARN_H
#define TOOLTIPWARN_H

#include <QFrame>

class QPushButton;

class ToolTipWarn : public QFrame
{
    Q_OBJECT
public:
    ToolTipWarn(const QString& text, QWidget *posTo = NULL, QWidget *parent = NULL, int delay = 3000, QString icon = QString());

    void setButton(QPushButton *btn);
};

#endif // TOOLTIPWARN_H
