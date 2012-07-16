/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QMouseEvent>

#include "resizeline.h"

ResizeLine::ResizeLine(QWidget *parent) : QFrame(parent)
{
    m_cur_stretch = 50;
    m_resize_layout = NULL;
    m_pct_label = NULL;

    setOrientation(true);
}

ResizeLine::ResizeLine(bool vertical, QWidget *parent) : QFrame(parent)
{
    m_cur_stretch = 50;
    m_resize_layout = NULL;
    m_pct_label = NULL;

    setOrientation(vertical);
}

void ResizeLine::setOrientation(bool vertical)
{
    m_vertical = vertical;

    setFrameStyle((vertical ? QFrame::VLine : QFrame::HLine) | QFrame::Plain);

    if(vertical)
        setSizeIncrement(QSizePolicy::Fixed, QSizePolicy::Expanding);
    else
        setSizeIncrement(QSizePolicy::Expanding, QSizePolicy::Fixed);

    setCursor(vertical ? Qt::SizeHorCursor : Qt::SizeVerCursor);
}

void ResizeLine::updateStretch()
{
    QBoxLayout *l = getLayout();
    Q_ASSERT(l);
    if(l)
    {
        int index = l->indexOf(this);
        if(index < 1)
        {
            Q_ASSERT(false);
            return;
        }
        m_cur_stretch = l->stretch(index-1);
    }
}

void ResizeLine::mousePressEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton)
        return QFrame::mousePressEvent(event);

    event->accept();

    m_resize_layout = getLayout();
    Q_ASSERT(m_resize_layout);

    m_mouse_pos = event->globalPos();

    if(m_resize_layout)
    {
        int index = m_resize_layout->indexOf(this);
        if(index < 1)
        {
            Q_ASSERT(false);
            m_resize_layout = NULL;
            return;
        }

        m_resize_index = index;

        QLayoutItem *item = m_resize_layout->itemAt(index-1);
        if(item->widget()) m_resize_pos[0] = item->widget()->pos();
        else               m_resize_pos[0] = item->layout()->geometry().topLeft();

        item = m_resize_layout->itemAt(index+1);
        if(!item)
            return;

        if(item->widget())
        {
            m_resize_pos[1] = item->widget()->pos();
            m_resize_pos[1].rx() += item->widget()->width();
            m_resize_pos[1].ry() += item->widget()->height();
        }
        else
            m_resize_pos[1] = item->layout()->geometry().bottomRight();

        m_pct_label = new QLabel(this, Qt::ToolTip);
        m_pct_label->setMargin(3);

        QPalette p(m_pct_label->palette());
        p.setColor(QPalette::Window, p.color(QPalette::ToolTipBase));
        m_pct_label->setPalette(p);

        setPctLabel(event->globalPos(), m_resize_layout->stretch(m_resize_index-1),
                                        m_resize_layout->stretch(m_resize_index+1));
        m_pct_label->show();
    }
}

void ResizeLine::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton)
        return QFrame::mouseReleaseEvent(event);

    event->accept();

    delete m_pct_label;
    m_pct_label = NULL;
}

void ResizeLine::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_resize_layout || !(event->buttons() & Qt::LeftButton))
        return QFrame::mouseMoveEvent(event);

    event->accept();

    QPoint mouse = event->globalPos();

    float dist, move;
    if(m_vertical)
    {
        move = (mouse - m_mouse_pos).x();
        dist = (m_resize_pos[1] - m_resize_pos[0]).x();
    }
    else
    {
        move = (mouse - m_mouse_pos).y();
        dist = (m_resize_pos[1] - m_resize_pos[0]).y();
    }

    m_cur_stretch += move / (dist / 100.f);

    if(m_cur_stretch > 100)    m_cur_stretch = 100;
    else if(m_cur_stretch < 0) m_cur_stretch = 0;

    int stretch = m_cur_stretch;
    if(abs(50 - m_cur_stretch) < 3)
        stretch = 50;

    m_resize_layout->setStretch(m_resize_index-1, stretch);
    m_resize_layout->setStretch(m_resize_index+1, 100 - stretch);

    m_mouse_pos = event->globalPos();

    setPctLabel(m_mouse_pos, stretch, 100 - stretch);
}

void ResizeLine::setPctLabel(const QPoint& p, int l, int r)
{
    if(!m_pct_label)
        return;

    m_pct_label->move(p + QPoint(0, 15));
    m_pct_label->setText(tr("%1% / %2%").arg(l).arg(r));
}
