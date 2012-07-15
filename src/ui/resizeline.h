#ifndef RESIZELINE_H
#define RESIZELINE_H

#include <QFrame>
#include <QBoxLayout>
#include <QLabel>

class ResizeLine : public QFrame
{
    Q_OBJECT
public:
    ResizeLine(bool vertical, QWidget *parent);
    ResizeLine(QWidget *parent);

    void updateStretch();
    void setOrientation(bool vertical);

    virtual void setResizeLayout(QBoxLayout *l)
    {
        m_resize_layout = l;
    }

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    virtual QBoxLayout *getLayout()
    {
        return m_resize_layout;
    }

private:
    void setPctLabel(const QPoint& p, int l, int r);

    bool m_vertical;
    float m_cur_stretch;
    QBoxLayout *m_resize_layout;
    QPoint m_resize_pos[2];
    QPoint m_mouse_pos;
    int m_resize_index;
    QLabel *m_pct_label;
};
#endif // RESIZELINE_H
