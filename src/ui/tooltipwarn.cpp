/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIcon>
#include <QTimer>
#include <QLabel>
#include <QPalette>
#include <QPushButton>
#include <QToolButton>
#include <QMovie>
#include <QDesktopWidget>
#include <QApplication>

#include "tooltipwarn.h"

ToolTipWarn::ToolTipWarn(const QString &text, QWidget *posTo, QWidget *parent, int delay, QString icon) :
    QFrame(parent, Qt::ToolTip)
{
    QHBoxLayout *l = new QHBoxLayout(this);
    QVBoxLayout *labelLayout = new QVBoxLayout;

    QPalette p(palette());
    p.setColor(QPalette::Window, p.color(QPalette::ToolTipBase));
    setPalette(p);

    QLabel *iconLabel = new QLabel(this);
    if(icon.isEmpty())
        iconLabel->setPixmap(QIcon(":/icons/warning").pixmap(32, 32));
    else
        iconLabel->setPixmap(QIcon(icon).pixmap(32, 32));

    m_lorrLabel = new QLabel("Lorris", this);
    QFont font = m_lorrLabel->font();
    font.setBold(true);
    m_lorrLabel->setFont(font);

    QLabel *textLabel = new QLabel(text, this);

    QToolButton *closeBtn = new QToolButton(this);
    closeBtn->setIcon(QIcon(":/actions/red-cross"));
    closeBtn->setCursor(Qt::ArrowCursor);
    closeBtn->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    connect(closeBtn, SIGNAL(clicked()), SLOT(deleteLater()));

    labelLayout->addWidget(m_lorrLabel);
    labelLayout->addWidget(textLabel);

    l->addWidget(iconLabel);
    l->addLayout(labelLayout);
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

    m_lorrLabel->hide();
}

void ToolTipWarn::setButton(QPushButton *btn)
{
    btn->setParent(this);
    ((QHBoxLayout*)layout())->insertWidget(layout()->count()-1, btn);

     QApplication::processEvents();  // to reorder layouts
}

void ToolTipWarn::showSpinner()
{
    QLabel *l = (QLabel*)(((QBoxLayout*)layout())->itemAt(0)->widget());

    QMovie *movie = new QMovie(":/actions/spinner", QByteArray(), this);
    l->setMovie(movie);
    movie->start();
}

void ToolTipWarn::toRightBottom()
{
    QDesktopWidget *desktop = qApp->desktop();
    if(!desktop)
        return;

    m_lorrLabel->show();

    QApplication::processEvents(); // to reorder layouts

    QRect rect = desktop->availableGeometry();
    move(rect.width() - width() - 15, rect.height() - height() - 15);
}
