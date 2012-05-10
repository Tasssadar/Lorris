/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <qwt_plot.h>

#include "graphcurve.h"
#include "../../storage.h"

GraphCurve::GraphCurve(const QString &name, GraphDataSimple *data) :
    QObject(NULL),QwtPlotCurve(name)
{

    //setRenderHint(QwtPlotItem::RenderAntialiased);
    setLegendAttribute(QwtPlotCurve::LegendShowLine, true);
    setPaintAttribute(QwtPlotCurve::CacheSymbols, true);

    m_data = data;
    setData(m_data);
}

GraphCurve::~GraphCurve()
{
    detach();
}

void GraphCurve::setSampleSize(qint32 size)
{
    m_data->setSampleSize(size);
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

void GraphCurve::setDataType(quint8 type)
{
    m_data->setDataType(type);
}

void GraphCurve::addPoint(quint32 index, qreal val)
{
    m_data->addPoint(index, val);
    plot()->replot();
}

void GraphCurve::clear()
{
    m_data->clear();
    plot()->replot();
}
