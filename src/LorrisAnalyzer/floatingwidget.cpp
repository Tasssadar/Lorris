/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QPainter>
#include <QApplication>
#include <QBitmap>
#include <QMouseEvent>

#include "floatingwidget.h"

#define DISSAPEAR 50

FloatingWidget::FloatingWidget(QWidget *parent) :
    QWidget(parent)
{
    setWindowFlags(windowFlags() | Qt::Popup);
    setAutoFillBackground(true);
    setWindowOpacity(0.9);
    setMouseTracking(true);

    QPalette p = palette();
    p.setColor(QPalette::Background, Qt::black);
    setPalette(p);

    connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), SLOT(focusChanged(QWidget*, QWidget*)));
}

FloatingWidget::~FloatingWidget()
{
    //releaseKeyboard();
}

void FloatingWidget::focusChanged(QWidget *, QWidget *to)
{
    if(!to || (to != this && !isAncestorOf(to)))
        deleteLater();
}

bool FloatingWidget::isAncestorOf(const QWidget *child) const
{
    while (child)
    {
        if (child == this)
            return true;
        child = child->parentWidget();
    }
    return false;
}

void FloatingWidget::resizeEvent(QResizeEvent *ev)
{
    QPixmap pixmap(size());
    QPainter painter(&pixmap);

    painter.fillRect(pixmap.rect(), Qt::white);
    painter.setBrush(Qt::black);

    painter.drawRoundRect(pixmap.rect(), 8, 8);

    setMask(pixmap.createMaskFromColor(Qt::white));
}

void FloatingWidget::mouseMoveEvent(QMouseEvent *ev)
{
    int x = ev->x();
    int y = ev->y();
    if (x > width()+DISSAPEAR || x < -DISSAPEAR ||
        y > height()+DISSAPEAR || y < -DISSAPEAR)
    {
        deleteLater();
    }

    QWidget::mouseMoveEvent(ev);
}
