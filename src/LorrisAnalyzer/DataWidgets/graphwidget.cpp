#include <qapplication.h>
#include <qlayout.h>
#include <qwt_plot.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <qwt_series_data.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <qwt_text.h>
#include <qwt_math.h>
#include <qwt_plot_grid.h>

#include "graphwidget.h"

GraphWidget::GraphWidget(QWidget *parent) : DataWidget(parent)
{
    m_widgetType = WIDGET_GRAPH;

    setTitle(tr("Graph"));
    setIcon(":/dataWidgetIcons/graph.png");

    m_graph = new Graph(this);

    layout->addWidget(m_graph);

    resize(400, 200);
}

GraphWidget::~GraphWidget()
{

}

void GraphWidget::setUp()
{
    DataWidget::setUp();
}

void GraphWidget::processData(analyzer_data *data)
{

}

void GraphWidget::saveWidgetInfo(AnalyzerDataFile *file)
{
    DataWidget::saveWidgetInfo(file);
}

void GraphWidget::loadWidgetInfo(AnalyzerDataFile *file)
{
    DataWidget::loadWidgetInfo(file);
}

void GraphWidget::dropEvent(QDropEvent *event)
{

}

GraphWidgetAddBtn::GraphWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Graph"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/graph.png"));

    m_widgetType = WIDGET_GRAPH;
}

Graph::Graph(QWidget *parent) : QwtPlot(parent)
{
    // zoom in/out with the wheel
    (void) new QwtPlotMagnifier( canvas() );

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setPen(QPen(Qt::gray, 0.0, Qt::DotLine));
    grid->enableX(true);
    grid->enableXMin(true);
    grid->enableY(true);
    grid->enableYMin(false);
    grid->attach(this);

    // to show graph appearance while dragging from add button to widget area
    replot();
}
