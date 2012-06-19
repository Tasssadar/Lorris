/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef TOOLTIPWARN_H
#define TOOLTIPWARN_H

#include <QWidget>

class ToolTipWarn : public QWidget
{
    Q_OBJECT
public:
    explicit ToolTipWarn(const QString& text, QWidget *posTo = NULL, QWidget *parent = NULL, int delay = 3000);
};

#endif // TOOLTIPWARN_H
