/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef PLUSTABBAR_H
#define PLUSTABBAR_H

#include <QTabBar>

class PlusTabBar : public QTabBar
{
    Q_OBJECT

Q_SIGNALS:
    void plusPressed();

public:
    explicit PlusTabBar(QWidget *parent = 0);

    void paintEvent(QPaintEvent *event);
    QSize sizeHint() const
    {
#ifdef Q_OS_MAC 
        return QTabBar::sizeHint() + QSize(40, 0);
#else
        return QTabBar::sizeHint() + QSize(20, 0);
#endif 
    }

public slots:
    void setDisablePlus(bool disable);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual void tabLayoutChange();

private:
    void updateRect();

    QPixmap& getMap()
    {
        return m_hover ? m_hover_pixmap : m_pixmap;
    }

    QRect m_plusRect;
    QPixmap m_pixmap;
    QPixmap m_hover_pixmap;
    bool m_disabled;
    bool m_hover;
    bool m_pressed;
};

#endif // PLUSTABBAR_H
