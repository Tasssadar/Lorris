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

class QwtPlotGrid;

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

#endif // GRAPH_H
