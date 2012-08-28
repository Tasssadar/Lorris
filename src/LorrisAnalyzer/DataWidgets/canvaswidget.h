/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include "datawidget.h"

class Canvas;

class CanvasWidget : public DataWidget
{
    Q_OBJECT

    Q_PROPERTY(int cWidth READ getCanvasWidth)
    Q_PROPERTY(int cHeight READ getCanvasHeight)

public:
    CanvasWidget(QWidget *parent);
    ~CanvasWidget();

    void setUp(Storage *storage);
    void saveWidgetInfo(DataFileParser *file);
    void loadWidgetInfo(DataFileParser *file);

public slots:
    void setBackground(const QString& color);

    void drawLine(int x, int y);
    void drawLine(int x1, int y1, int x2, int y2);
    void setLineSize(int width);
    void setLineColor(const QString& color);
    void clear();

    int getCanvasWidth() const;
    int getCanvasHeight() const;

private:
    Canvas *m_canvas;
    QPoint m_lastLine;
    QColor m_lineColor;
    int m_lineWidth;
};

class CanvasWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    CanvasWidgetAddBtn(QWidget *parent = 0);
};

class Canvas : public QWidget
{
    Q_OBJECT
public:
    Canvas(QWidget *parent);

    void save(DataFileParser *file);
    void load(DataFileParser *file);

    void setBackground(QColor bg);
    void addLine(const QLine& l, const QColor& c, int width);
    void clear();

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    struct line
    {
        line()
        {
            width = 0;
        }

        line(const QLine& l, const QColor& c, int width)
        {
            this->l = l;
            this->color = c;
            this->width = width;
        }

        QLine l;
        QColor color;
        int width;
    };

    std::vector<line> m_lines;
    QPoint m_mouse;
    QPoint m_offset;
};

#endif // CANVASWIDGET_H
