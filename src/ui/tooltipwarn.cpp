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
#include <QPushButton>
#include <QToolButton>
#include <QMovie>

#include "tooltipwarn.h"

ToolTipWarn::ToolTipWarn(const QString &text, QWidget *posTo, QWidget *parent, int delay, QString icon) :
    QFrame(parent, Qt::ToolTip)
{
    QHBoxLayout *l = new QHBoxLayout(this);

    QPalette p(palette());
    p.setColor(QPalette::Window, p.color(QPalette::ToolTipBase));
    setPalette(p);

    QLabel *iconLabel = new QLabel(this);
    if(icon.isEmpty())
        iconLabel->setPixmap(QIcon(":/icons/warning").pixmap(32, 32));
    else
        iconLabel->setPixmap(QIcon(icon).pixmap(32, 32));

    QLabel *textLabel = new QLabel(text, this);

    QToolButton *closeBtn = new QToolButton(this);
    closeBtn->setIcon(QIcon(":/actions/red-cross"));
    closeBtn->setCursor(Qt::ArrowCursor);
    closeBtn->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    connect(closeBtn, SIGNAL(clicked()), SLOT(deleteLater()));

    l->addWidget(iconLabel);
    l->addWidget(textLabel);
    l->addWidget(closeBtn);

    if(delay != -1)
    {
        QTimer *timer = new QTimer(this);
        timer->start(delay);
        connect(timer, SIGNAL(timeout()), SLOT(deleteLater()));
    }

    if(posTo && parent)
        move(parent->mapToGlobal(posTo->pos()) + QPoint(0, posTo->height()));

    setFrameStyle(QFrame::Box | QFrame::Plain);
    show();
}

void ToolTipWarn::setButton(QPushButton *btn)
{
    btn->setParent(this);
    ((QHBoxLayout*)layout())->insertWidget(layout()->count()-1, btn);
}

void ToolTipWarn::showSpinner()
{
    QLabel *l = (QLabel*)(((QBoxLayout*)layout())->itemAt(0)->widget());

    QMovie *movie = new QMovie(":/actions/spinner", QByteArray(), this);
    l->setMovie(movie);
    movie->start();
}
