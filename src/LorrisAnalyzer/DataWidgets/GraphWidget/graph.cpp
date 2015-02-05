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
#include <qwt_plot_layout.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_event_pattern.h>
#include <qwt_legend_label.h>
#include <float.h>

#include <QMouseEvent>
#include <QWheelEvent>
#include <QColorDialog>

#include "../datawidget.h"
#include "graph.h"
#include "graphcurve.h"

Graph::Graph(QWidget *parent) : QwtPlot(parent)
{
    // zoom in/out with the wheel
    Magnifier *magnifier = new Magnifier( canvas() );
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

    m_grid = new QwtPlotGrid();
    m_grid->setMinorPen(QPen(Qt::gray, 0.0, Qt::DotLine));
    m_grid->setMajorPen(QPen(Qt::black, 0.0, Qt::DotLine));
    m_grid->enableX(true);
    m_grid->enableXMin(true);
    m_grid->enableY(true);
    m_grid->enableYMin(true);
    m_grid->attach(this);

    initLegend();

    connect(axisWidget(QwtPlot::xBottom), SIGNAL(scaleDivChanged()), SIGNAL(updateSampleSize()));

    setAxisScale(QwtPlot::xBottom, -20, 20);
    setAxisScale(QwtPlot::yRight, -20, 20);
    setAxisScale(QwtPlot::yLeft, -20, 20);
    axisWidget(QwtPlot::xBottom)->setToolTip(tr("Double-click to add marker"));
    axisWidget(QwtPlot::yLeft)->setToolTip(tr("Double-click to add marker"));
    axisWidget(QwtPlot::yRight)->setToolTip(tr("Double-click to add marker"));

    // to show graph appearance while dragging from add button to widget area
    replot();
}

void Graph::showCurve(GraphCurve *curve, bool show)
{
    showCurve(itemToInfo(curve), show, -1);
}

void Graph::showCurve(const QVariant &itemInfo, bool on, int index )
{
    QwtPlotItem *it = infoToItem(itemInfo);
    if(it)
        it->setVisible(on);

    if(legend())
    {
        QWidget *w = ((QwtLegend*)legend())->legendWidget(itemInfo);
        if(w && w->inherits("QwtLegendLabel"))
            ((QwtLegendLabel*)w)->setChecked(on);
    }

    replot();
}

void Graph::showLegend(bool show)
{
    if(show == (legend() != NULL))
        return;

    if(show)
        initLegend();
    else
        delete legend();

    updateLayout();
}

void Graph::initLegend()
{
    QwtLegend *legend = new QwtLegend;
    legend->setDefaultItemMode(QwtLegendData::Checkable);
    insertLegend(legend, QwtPlot::BottomLegend);

    connect(legend, SIGNAL(checked(QVariant,bool,int)), SLOT(showCurve(QVariant,bool,int)));

    QwtPlotItemList l = this->itemList(QwtPlotItem::Rtti_PlotCurve);
    for(int i = 0; i < l.size(); ++i)
    {
        QwtPlotItem *curve = l[i];
        QVariant info = itemToInfo(curve);
        QWidget *w = legend->legendWidget(info);
        if(w && w->inherits("QwtLegendLabel"))
            ((QwtLegendLabel*)w)->setChecked(curve->isVisible());
    }
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

    GraphMarkerDialog d(axis, this);
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
        return;

    double max = axisScaleDiv(axis).upperBound();
    double min = axisScaleDiv(axis).lowerBound();

    double diff = fabs(max - min);
    if(diff == 0)
        diff = 1;

    float exp = (event->modifiers() & Qt::ShiftModifier) ? 0.01 : 0.001;
    double newDiff = fabs(diff + (diff*(exp *event->delta())));

    double pct = getAxisPosPct(axis, event->pos());
    double newMax = (max - diff*(1-pct)) + newDiff*(1-pct);
    double newMin = (min + diff*pct) - newDiff*pct;

    if(newMin > newMax || newMin == newMax)
        return;

    setAxisScale(axis, newMin, newMax);
    if(axis == QwtPlot::yLeft || axis == QwtPlot::yRight)
        syncYZeros(axis, newMin, newMax);
    replot();
}

int Graph::getAxisOnPos(const QPoint &pos)
{
    int yLeftPos = axisWidget(QwtPlot::yLeft)->pos().x() + axisWidget(QwtPlot::yLeft)->width();
    int yRightPos = axisWidget(QwtPlot::yRight)->pos().x();
    int xPos = axisWidget(QwtPlot::xBottom)->pos().y();

    if(pos.x() < yLeftPos)      return QwtPlot::yLeft;
    else if(yRightPos && pos.x() > yRightPos) return QwtPlot::yRight;
    else if(xPos && pos.y() > xPos) return QwtPlot::xBottom;
    else                    return -1;
}

double Graph::getAxisPosPct(int axis, const QPoint &pos)
{
    QwtScaleWidget *w = axisWidget(axis);
    switch(axis)
    {
        case QwtPlot::yLeft:
        case QwtPlot::yRight:
            return 1.0 - (double(pos.y() - w->pos().y()) / w->height());
        case QwtPlot::xTop:
        case QwtPlot::xBottom:
            return (double(pos.x() - w->pos().x()) / w->width());
    }
    return 0.5;
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
    m->setYAxis(axis);
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
    return axisScaleDiv(QwtPlot::xBottom).upperBound();
}

double Graph::XlowerBound()
{
    return axisScaleDiv(QwtPlot::xBottom).lowerBound();
}

double Graph::YupperBound()
{
    return axisScaleDiv(QwtPlot::yLeft).upperBound();
}

double Graph::YlowerBound()
{
    return axisScaleDiv(QwtPlot::yLeft).lowerBound();
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
    m_grid->setMajorPen(QPen(text, 0.0, Qt::DotLine));

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

    file->writeBlockIdentifier("graphWMarkersYRight");
    saveMarkers(file, QwtPlot::yRight);

    // axes
    file->writeBlockIdentifier("graphWAxisRangeV2");
    *file << (quint32)QwtPlot::axisCnt;
    for(size_t i = 0; i < QwtPlot::axisCnt; ++i)
    {
        const QwtScaleDiv div = axisScaleDiv(i);
        *file << div.lowerBound();
        *file << div.upperBound();
    }
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

    if(file->seekToNextBlock("graphWMarkersYRight", BLOCK_WIDGET))
        loadMarkers(file, QwtPlot::yRight);

    // axes
    if(file->seekToNextBlock("graphWAxisRangeV2", BLOCK_WIDGET))
    {
        const quint32 cnt = file->readVal<quint32>();
        for(quint32 i = 0; i < cnt; ++i)
        {
            double lower = file->readVal<double>();
            double upper = file->readVal<double>();
            setAxisScale(i, lower, upper);
        }
    }

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

void Graph::syncYZeros()
{
    const QwtScaleDiv div = axisScaleDiv(QwtPlot::yLeft);
    syncYZeros(QwtPlot::yLeft, div.lowerBound(), div.upperBound());
}

void Graph::syncYZeros(int masterAxis, double masterAxisLower, double masterAxisUpper)
{
    int slaveAxis = masterAxis == QwtPlot::yLeft ? QwtPlot::yRight : QwtPlot::yLeft;
    const QwtScaleDiv divSlave = axisScaleDiv(slaveAxis);

    double slaveAxisRange = divSlave.range();
    double zeroOff = masterAxisLower/(masterAxisUpper - masterAxisLower);
    double lower = slaveAxisRange*zeroOff;
    double upper = slaveAxisRange*zeroOff + slaveAxisRange;
    setAxisScale(slaveAxis, lower, upper);
}

Panner::Panner(QWidget *canvas) : QwtPlotPanner(canvas)
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

void Magnifier::widgetWheelEvent(QWheelEvent *e)
{
    QWheelEvent modEv(e->pos(), -e->delta(), e->buttons(), e->modifiers(), e->orientation());
    QwtPlotMagnifier::widgetWheelEvent(&modEv);
}

GraphMarkerDialog::GraphMarkerDialog(int axis, QWidget *parent) : QDialog(parent), ui(new Ui::GraphMarkerDialog)
{
    ui->setupUi(this);
    ui->valBox->setRange(-DBL_MAX, DBL_MAX);
    ui->axisBox->setCurrentIndex(axis);

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

int GraphMarkerDialog::getAxis() const
{
    return ui->axisBox->currentIndex();
}
