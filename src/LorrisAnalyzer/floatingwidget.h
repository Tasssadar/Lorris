#ifndef FLOATINGWIDGET_H
#define FLOATINGWIDGET_H

#include <QWidget>

class FloatingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FloatingWidget(QWidget *parent = 0);
    ~FloatingWidget();

protected:
    void resizeEvent(QResizeEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);

private slots:
    void focusChanged(QWidget *, QWidget *to);

private:
    bool isAncestorOf(const QWidget *child) const;
};

#endif // FLOATINGWIDGET_H
