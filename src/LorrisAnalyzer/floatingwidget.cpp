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
#include "../misc/utils.h"

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
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(pixmap.rect(), Qt::white);
    painter.setBrush(Qt::black);

    QRect r = pixmap.rect();
    r.adjust(2, 2, -2, -2);
    painter.drawRoundRect(r, 8, 8);
    painter.end();

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

FlatListWidget::FlatListWidget(bool selectByHover, QWidget *parent) :
    QListWidget(parent)
{
    m_selectByHover = selectByHover;

    setFrameStyle(QFrame::Plain | QFrame::NoFrame);

    setStyleSheet("QListWidget { background-color: #000000; color: #ffffff; }"
                  "QListWidget::item { padding-top: 3px; padding-bottom:3px;}"
                  "QListWidget::item:selected { background-color: #FF4444; color: white; }");

    if(selectByHover)
    {
        setStyleSheet(styleSheet() +
                      "QListWidget::item:hover { background-color: transparent; }");
    }
    else
    {
        setStyleSheet(styleSheet() +
                      "QListWidget::item:hover { background-color: #FF4444; color: white; }");
    }

    setUniformItemSizes(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFont(Utils::getMonospaceFont());
}

bool FlatListWidget::event(QEvent *e)
{
    if(!m_selectByHover || e->type() != QEvent::HoverMove)
        return QListWidget::event(e);

    QHoverEvent *ev = (QHoverEvent*)e;
    QListWidgetItem *it = itemAt(ev->pos());
    if(it)
        setCurrentItem(it);
    return true;
}
