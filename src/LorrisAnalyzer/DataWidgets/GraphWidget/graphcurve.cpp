#include "graphcurve.h"
#include "../../analyzerdatastorage.h"


GraphCurve::GraphCurve(const QString &name, AnalyzerDataStorage *data, data_widget_info& info, qint32 sample_size, quint8 data_type) :
    QwtPlotCurve(name)
{

    //setRenderHint(QwtPlotItem::RenderAntialiased);
    setLegendAttribute(QwtPlotCurve::LegendShowLine, true);
    setPaintAttribute(QwtPlotCurve::CacheSymbols, true);

    m_data = new GraphData(data, info, sample_size, data_type);
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
