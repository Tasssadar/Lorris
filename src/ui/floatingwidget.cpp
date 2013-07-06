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
#include <QDesktopWidget>

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

}

void FloatingWidget::focusLost()
{
    deleteLater();
}

void FloatingWidget::focusChanged(QWidget *, QWidget *to)
{
    if(!to || (to != this && !isAncestorOf(to)))
        focusLost();
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
    painter.drawRoundRect(r, 4, 4);
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
        focusLost();
    }

    QWidget::mouseMoveEvent(ev);
}

void FloatingWidget::ensureOnScreen()
{
    QDesktopWidget w;
    QRect screen = w.screenGeometry(mapToGlobal(pos()));

    QPoint p = pos();
    if(p.x() < 0)
        p.rx() = 0;
    else if(p.x() + width() > screen.width())
        p.rx() = screen.width() - width();

    if(p.y() < 0)
        p.ry() = 0;
    else if(p.y() + height() > screen.height())
        p.ry() = screen.height() - height();

    move(p);
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
                      "QListWidget::item:hover:selected { background-color: #FF4444; color: white; }"
                      "QListWidget::item:hover:!selected { background-color: transparent; }");
    }
    else
    {
        setStyleSheet(styleSheet() +
                      "QListWidget::item:hover { background-color: #FF4444; color: white; }");
    }

    setUniformItemSizes(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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
