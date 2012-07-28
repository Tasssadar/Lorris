/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef GRAPH_H
#define GRAPH_H

#include <QColor>
#include <qwt_plot.h>
#include <qwt_plot_panner.h>

class QwtPlotGrid;
class QwtPlotCanvas;

class Graph : public QwtPlot
{
    Q_OBJECT

Q_SIGNALS:
    void updateSampleSize();

public:
    Graph(QWidget *parent = 0);

    void showLegend(bool show);

    double XupperBound();
    double XlowerBound();
    double YupperBound();
    double YlowerBound();

    void setBgColor(const QColor& c);
    QColor getBgColor();

protected:
    void mousePressEvent(QMouseEvent * event);
    void wheelEvent(QWheelEvent *event);

public slots:
    void showCurve(QwtPlotItem *item, bool on);

private:
    QwtPlotGrid *m_grid;
};

class Panner : public QwtPlotPanner
{
    Q_OBJECT
public:
    Panner(QwtPlotCanvas *canvas);

private slots:
    void moveAxes(int dx, int dy);
    void finished(int dx, int dy);

private:
    int m_lastX;
    int m_lastY;
};

#endif // GRAPH_H
