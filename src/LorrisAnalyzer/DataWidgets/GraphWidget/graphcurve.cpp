/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include <qwt_plot.h>

#include "graphcurve.h"
#include "../../analyzerdatastorage.h"

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
