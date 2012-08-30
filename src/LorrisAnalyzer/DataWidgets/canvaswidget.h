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
    void drawRect(int x, int y, int w, int h);
    void drawEllipse(int x, int y, int w, int h);
    void drawCircle(int x, int y, int r);
    void setLineSize(int width);
    void setLineColor(const QString& color);
    void setFillColor(const QString& color);
    void clear();

    int getCanvasWidth() const;
    int getCanvasHeight() const;

private:
    Canvas *m_canvas;
    QPoint m_lastLine;
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
    void addLine(const QLine& l);
    void addRect(const QRect& r);
    void addCircle(const QPoint& center, int r1, int r2);
    void clear();

    QColor& lineColor() { return m_lineColor; }
    QColor& fillColor() { return m_fillColor; }
    int& lineWidth() { return m_lineWidth; }

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    struct line
    {
        QLine l;
        QColor color;
        int width;
    };

    struct rect
    {
        QRect r;
        QColor color;
        QColor fillColor;
        int width;
    };

    struct circle
    {
        QPoint center;
        int r1;
        int r2;
        QColor color;
        QColor fillColor;
        int width;
    };

    std::vector<line> m_lines;
    std::vector<rect> m_rects;
    std::vector<circle> m_circles;
    QPoint m_mouse;
    QPoint m_offset;
    QColor m_lineColor;
    QColor m_fillColor;
    int m_lineWidth;
};

#endif // CANVASWIDGET_H
