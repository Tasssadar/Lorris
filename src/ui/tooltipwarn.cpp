/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QHBoxLayout>
#include <QIcon>
#include <QTimer>
#include <QLabel>
#include <QPalette>

#include "tooltipwarn.h"

ToolTipWarn::ToolTipWarn(const QString& text, QWidget *posTo, QWidget *parent,  int delay) :
    QWidget(parent, Qt::ToolTip)
{
    QHBoxLayout *l = new QHBoxLayout(this);

    QPalette p(palette());
    p.setColor(QPalette::Window, p.color(QPalette::ToolTipBase));
    setPalette(p);

    QLabel *icon = new QLabel(this);
    icon->setPixmap(QIcon(":/icons/warning").pixmap(32, 32));

    QLabel *textLabel = new QLabel(text, this);

    l->addWidget(icon);
    l->addWidget(textLabel);

    if(delay != -1)
    {
        QTimer *timer = new QTimer(this);
        timer->start(delay);
        connect(timer, SIGNAL(timeout()), SLOT(deleteLater()));
    }

    if(posTo && parent)
        move(parent->mapToGlobal(posTo->pos()) + QPoint(0, posTo->height()));

    show();
}
