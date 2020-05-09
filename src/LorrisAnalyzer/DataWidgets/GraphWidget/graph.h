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
#include <qwt_plot_magnifier.h>

#include "ui_graphmarkerdialog.h"

class QwtPlotGrid;
class QwtPlotCanvas;
class QwtPlotMarker;
class DataFileParser;

class GraphCurve;

class Graph : public QwtPlot
{
    Q_OBJECT

Q_SIGNALS:
    void updateSampleSize();

public:
    Graph(QWidget *parent = 0);
    ~Graph();

    void showLegend(bool show);

    double XupperBound();
    double XlowerBound();
    double YupperBound();
    double YlowerBound();

    void setBgColor(const QColor& c);
    QColor getBgColor();

    void saveData(DataFileParser *file);
    void loadData(DataFileParser *file);

    void saveMarkers(DataFileParser *file, int axis);
    void loadMarkers(DataFileParser *file, int axis);

protected:
    void mousePressEvent(QMouseEvent * event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

public slots:
    void showCurve(const QVariant &itemInfo, bool on, int index);
    void showCurve(GraphCurve *curve, bool show);
    void syncYZeros();

private:
    int getAxisOnPos(const QPoint& pos);
    double getAxisPosPct(int axis, const QPoint &pos);
    void createMarkerRmMenu(const QPoint& pos, int axis);
    void addMarker(double val, const QColor& color, int axis);
    void initLegend();
    void syncYZeros(int masterAxis, double masterAxisLower, double masterAxisUpper);

    std::vector<QwtPlotMarker*>& getMarkers(int axis)
    {
        switch(axis)
        {
            case QwtPlot::yLeft:
                return m_yLeftMarkers;
            case QwtPlot::yRight:
                return m_yRightMarkers;
            case QwtPlot::xBottom:
            default:
                return m_xMarkers;
        }
    }

    QwtPlotGrid *m_grid;
    std::vector<QwtPlotMarker*> m_xMarkers;
    std::vector<QwtPlotMarker*> m_yLeftMarkers;
    std::vector<QwtPlotMarker*> m_yRightMarkers;
};

class Panner : public QwtPlotPanner
{
    Q_OBJECT
public:
    Panner(QWidget *canvas);

private slots:
    void moveAxes(int dx, int dy);
    void finished(int dx, int dy);

private:
    int m_lastX;
    int m_lastY;
};

class Magnifier : public QwtPlotMagnifier
{
    Q_OBJECT
public:
    explicit Magnifier(QWidget *canvas) : QwtPlotMagnifier(canvas)
    {
    }

protected:
    virtual void widgetWheelEvent(QWheelEvent *event);
};

class GraphMarkerDialog : public QDialog, private Ui::GraphMarkerDialog
{
    Q_OBJECT
public:
    GraphMarkerDialog(int axis, QWidget *parent = 0);
    ~GraphMarkerDialog();

    double getValue() const;
    QColor getColorVal() const { return m_color; }
    int getAxis() const;

private slots:
    void on_colorBtn_clicked();

private:
    void setButtonColor(const QColor& clr);

    Ui::GraphMarkerDialog *ui;
    QColor m_color;
};

#endif // GRAPH_H
