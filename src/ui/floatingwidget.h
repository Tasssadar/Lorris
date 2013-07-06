/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef FLOATINGWIDGET_H
#define FLOATINGWIDGET_H

#include <QWidget>
#include <QListWidget>

class FloatingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FloatingWidget(QWidget *parent = 0);
    ~FloatingWidget();

    void ensureOnScreen();

protected:
    void resizeEvent(QResizeEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);

    virtual void focusLost();

private slots:
    void focusChanged(QWidget *, QWidget *to);

private:
    bool isAncestorOf(const QWidget *child) const;
};

class FlatListWidget : public QListWidget
{
    Q_OBJECT
public:
    FlatListWidget(bool selectByHover, QWidget *parent = 0);

protected:
    bool event(QEvent *e);

private:
    bool m_selectByHover;
};

#endif // FLOATINGWIDGET_H
