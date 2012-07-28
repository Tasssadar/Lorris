/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

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
#include <qwt_legend_item.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_event_pattern.h>
#include <float.h>

#include <QMouseEvent>
#include <QWheelEvent>
#include <QColorDialog>

#include "../datawidget.h"
#include "graph.h"

Graph::Graph(QWidget *parent) : QwtPlot(parent)
{
    // zoom in/out with the wheel
    QwtPlotMagnifier *magnifier = new QwtPlotMagnifier( canvas() );
    magnifier->setMouseButton(Qt::NoButton);

    // panning with the left mouse button
    Panner *panner =  new Panner( canvas() );
    panner->setMouseButton(Qt::LeftButton);

    // zooming with middle button
    QwtPlotZoomer *zoomer = new QwtPlotZoomer(canvas());
    {
        QVector<QwtEventPattern::MousePattern> pattern(6);
        pattern[0].button = Qt::MiddleButton;
        zoomer->setMousePattern(pattern);
    }

#if defined(Q_WS_X11)
    // Even if not recommended by TrollTech, Qt::WA_PaintOutsidePaintEvent
    // works on X11. This has a nice effect on the performance.

    canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, true);
#endif

    m_grid = new QwtPlotGrid();
    m_grid->setMinPen(QPen(Qt::gray, 0.0, Qt::DotLine));
    m_grid->setMajPen(QPen(Qt::black, 0.0, Qt::DotLine));
    m_grid->enableX(true);
    m_grid->enableXMin(true);
    m_grid->enableY(true);
    m_grid->enableYMin(false);
    m_grid->attach(this);

    QwtLegend *legend = new QwtLegend;
    legend->setItemMode(QwtLegend::CheckableItem);
    insertLegend(legend, QwtPlot::BottomLegend);

    connect(this, SIGNAL(legendChecked(QwtPlotItem*,bool)), SLOT(showCurve(QwtPlotItem*, bool)));
    connect(axisWidget(QwtPlot::xBottom), SIGNAL(scaleDivChanged()), SIGNAL(updateSampleSize()));

    setAxisScale(QwtPlot::xBottom, -20, 20);
    setAxisScale(QwtPlot::yLeft, -20, 20);
    axisWidget(QwtPlot::xBottom)->setToolTip(tr("Double-click to add marker"));
    axisWidget(QwtPlot::yLeft)->setToolTip(tr("Double-click to add marker"));

    // to show graph appearance while dragging from add button to widget area
    replot();
}

void Graph::showCurve(QwtPlotItem *item, bool on)
{
    item->setVisible(on);
    QWidget *w = legend()->find(item);
    if ( w && w->inherits("QwtLegendItem") )
        ((QwtLegendItem *)w)->setChecked(on);

    replot();
}

void Graph::showLegend(bool show)
{
    plotLayout()->setLegendPosition(show ? QwtPlot::BottomLegend : QwtPlot::ExternalLegend);
    legend()->setVisible(show);
    replot();
}

void Graph::mousePressEvent(QMouseEvent *event)
{
    int x = event->pos().x();
    int y = event->pos().y();

    if((x < RESIZE_BORDER) ||
       (x > width() - RESIZE_BORDER) ||
       (y > height() - RESIZE_BORDER))
    {
        return QWidget::mousePressEvent(event);
    }

    int axis = getAxisOnPos(event->pos());
    if(axis == -1)
    {
        QwtPlot::mousePressEvent(event);
        event->accept();
        return;
    }

    if(event->button() == Qt::RightButton)
    {
        createMarkerRmMenu(event->globalPos(), axis);
        event->accept();
        return;
    }
}

void Graph::mouseDoubleClickEvent(QMouseEvent *event)
{
    int axis = getAxisOnPos(event->pos());
    if(axis == -1)
        return QwtPlot::mouseDoubleClickEvent(event);

    event->accept();

    GraphMarkerDialog d(this);
    if(d.exec() == QDialog::Accepted)
    {
        addMarker(d.getValue(), d.getColorVal(), axis);
        replot();
    }
}

void Graph::wheelEvent(QWheelEvent *event)
{
    int axis = getAxisOnPos(event->pos());
    if(axis == -1)
        return QwtPlot::wheelEvent(event);

    double max = axisScaleDiv(axis)->upperBound();
    double min = axisScaleDiv(axis)->lowerBound();

    double diff = fabs(max - min);
    if(diff == 0)
        diff = 1;

    float exp = (event->modifiers() & Qt::ShiftModifier) ? 0.01 : 0.001;
    double newDiff = fabs(diff + (diff*(exp *event->delta())))/2;

    diff /= 2;
    double newMax = (max - diff) + newDiff;
    double newMin = (min + diff) - newDiff;

    if(newMin > newMax)
        return;

    setAxisScale(axis, newMin, newMax);
    replot();
}

int Graph::getAxisOnPos(const QPoint &pos)
{
    int yPos = axisWidget(QwtPlot::yLeft)->pos().x() + axisWidget(QwtPlot::yLeft)->width();
    int xPos = axisWidget(QwtPlot::xBottom)->pos().y();

    if(pos.x() < yPos)      return QwtPlot::yLeft;
    else if(pos.y() > xPos) return QwtPlot::xBottom;
    else                    return -1;
}

void Graph::createMarkerRmMenu(const QPoint &pos, int axis)
{
    QMenu menu;
    QAction *title = menu.addAction(tr("Remove markers"));
    title->setEnabled(false);
    menu.addSeparator();

    std::vector<QwtPlotMarker*>& markers = getMarkers(axis);
    if(markers.empty())
    {
        QAction *act = menu.addAction(tr("No markers"));
        act->setEnabled(false);
    }
    else
    {
        for(std::vector<QwtPlotMarker*>::iterator itr = markers.begin(); itr != markers.end(); ++itr)
        {
            QAction *act = menu.addAction(QString::number((*itr)->value().x()));
            act->setData(QVariant::fromValue((void*)(*itr)));
        }
    }

    QAction * res = menu.exec(pos);
    if(!res)
        return;

    std::vector<QwtPlotMarker*>::iterator itr = std::find(markers.begin(), markers.end(),
                                                          (QwtPlotMarker*)res->data().value<void*>());
    if(itr == markers.end())
        return;

    (*itr)->detach();
    delete *itr;
    markers.erase(itr);

    replot();
}

void Graph::addMarker(double val, const QColor &color, int axis)
{
    QwtPlotMarker *m = new QwtPlotMarker();
    m->setLineStyle(axis == QwtPlot::xBottom ? QwtPlotMarker::VLine : QwtPlotMarker::HLine);
    m->setLinePen(QPen(color));
    m->setValue(val, val);
    m->attach(this);
    m->setLabel(QString::number(val));

    if(axis == QwtPlot::xBottom)
        m->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    else
        m->setLabelAlignment(Qt::AlignTop | Qt::AlignHCenter);

    getMarkers(axis).push_back(m);
}

double Graph::XupperBound()
{
    return axisScaleDiv(QwtPlot::xBottom)->upperBound();
}

double Graph::XlowerBound()
{
    return axisScaleDiv(QwtPlot::xBottom)->lowerBound();
}

double Graph::YupperBound()
{
    return axisScaleDiv(QwtPlot::yLeft)->upperBound();
}

double Graph::YlowerBound()
{
    return axisScaleDiv(QwtPlot::yLeft)->lowerBound();
}

void Graph::setBgColor(const QColor &c)
{
    QColor text;

    int diff = (299*c.red() + 587*c.green() + 114*c.blue())/1000;
    if(diff < 130) text = Qt::white;
    else           text = Qt::black;

    QPalette p = canvas()->palette();
    p.setColor(QPalette::Window, c);
    canvas()->setPalette(p);
    m_grid->setMajPen(QPen(text, 0.0, Qt::DotLine));

    replot();
}

QColor Graph::getBgColor()
{
    return canvas()->palette().color(QPalette::Window);
}

void Graph::saveData(DataFileParser *file)
{
    // background color
    file->writeBlockIdentifier("graphWBgColor");
    file->writeString(getBgColor().name());

    // markers
    file->writeBlockIdentifier("graphWMarkersX");
    saveMarkers(file, QwtPlot::xBottom);

    file->writeBlockIdentifier("graphWMarkersY");
    saveMarkers(file, QwtPlot::yLeft);
}

void Graph::saveMarkers(DataFileParser *file, int axis)
{
    std::vector<QwtPlotMarker*>& markers = getMarkers(axis);
    file->writeVal((quint32)markers.size());

    for(quint32 i = 0; i < markers.size(); ++i)
    {
        file->writeVal(markers[i]->value().x());
        file->writeString(markers[i]->linePen().color().name());
    }
}

void Graph::loadData(DataFileParser *file)
{
    // background color
    if(file->seekToNextBlock("graphWBgColor", BLOCK_WIDGET))
        setBgColor(QColor(file->readString()));

    // markers
    if(file->seekToNextBlock("graphWMarkersX", BLOCK_WIDGET))
        loadMarkers(file, QwtPlot::xBottom);

    if(file->seekToNextBlock("graphWMarkersY", BLOCK_WIDGET))
        loadMarkers(file, QwtPlot::yLeft);

    replot();
}

void Graph::loadMarkers(DataFileParser *file, int axis)
{
    quint32 count = file->readVal<quint32>();
    for(quint32 i = 0; i < count; ++i)
    {
        double val = file->readVal<double>();
        QColor color(file->readString());
        addMarker(val, color, axis);
    }
}

Panner::Panner(QwtPlotCanvas *canvas) : QwtPlotPanner(canvas)
{
    m_lastX = 0;
    m_lastY = 0;

    disconnect(this, SIGNAL(panned(int,int)), this, SLOT(moveCanvas(int,int)));
    connect(this, SIGNAL(moved(int,int)), SLOT(moveAxes(int,int)));
    connect(this, SIGNAL(panned(int,int)), SLOT(finished(int,int)));
}

void Panner::finished(int dx, int dy)
{
    moveCanvas(dx - m_lastX, dy - m_lastY);
    m_lastX = m_lastY = 0;
}

void Panner::moveAxes(int dx, int dy)
{
    moveCanvas(dx - m_lastX, dy - m_lastY);
    m_lastX = dx;
    m_lastY = dy;
}

GraphMarkerDialog::GraphMarkerDialog(QWidget *parent) : QDialog(parent), ui(new Ui::GraphMarkerDialog)
{
    ui->setupUi(this);
    ui->valBox->setRange(-DBL_MAX, DBL_MAX);

    setButtonColor(Qt::black);
}

GraphMarkerDialog::~GraphMarkerDialog()
{
    delete ui;
}

void GraphMarkerDialog::setButtonColor(const QColor& clr)
{
    QPixmap map(50, 25);
    map.fill(clr);
    m_color = clr;
    ui->colorBtn->setIcon(QIcon(map));
}

void GraphMarkerDialog::on_colorBtn_clicked()
{
    QColor clr = QColorDialog::getColor(m_color, this);
    if(!clr.isValid())
        return;

    setButtonColor(clr);
}

double GraphMarkerDialog::getValue() const
{
    return ui->valBox->value();
}
