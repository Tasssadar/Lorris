/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <qwt_plot.h>

#include "graphcurve.h"
#include "../../storage.h"

GraphCurve::GraphCurve(const QString &name, GraphData *data) :
    QObject(NULL),QwtPlotCurve(name)
{
    m_data = data;
    init();
}

GraphCurve::GraphCurve(const GraphCurve &curve) : QObject(NULL), QwtPlotCurve(curve.title())
{
    m_data = curve.m_data;
    init();
}

GraphCurve::~GraphCurve()
{
    detach();
}

void GraphCurve::init()
{
    //setRenderHint(QwtPlotItem::RenderAntialiased);
    setLegendAttribute(QwtPlotCurve::LegendShowLine, true);
    setData(m_data);
}

void GraphCurve::setSampleSize(quint32 size, quint32 offset)
{
    m_data->setSampleSize(size, offset);
}

void GraphCurve::setSampleOffset(quint32 offset)
{
    m_data->setSampleOffset(offset);
}

void GraphCurve::dataPosChanged(quint32 pos)
{
    m_data->dataPosChanged(pos);
}

qint32 GraphCurve::getMax()
{
    return m_data->getMax();
}

qint32 GraphCurve::getMin()
{
    return m_data->getMin();
}

quint32 GraphCurve::getSize()
{
    return m_data->size();
}

quint32 GraphCurve::getMaxX()
{
    return m_data->getMaxX();
}

void GraphCurve::setDataType(quint8 type)
{
    m_data->setDataType(type);
}

void GraphCurve::addPoint(qreal index, qreal val)
{
    m_data->addPoint(index, val);
    plot()->replot();
}

void GraphCurve::clear()
{
    m_data->clear();
    plot()->replot();
}
