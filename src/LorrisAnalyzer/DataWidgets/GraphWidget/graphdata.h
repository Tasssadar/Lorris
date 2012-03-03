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

#ifndef GRAPHDATA_H
#define GRAPHDATA_H

#include <qwt_series_data.h>
#include <deque>

#include "../datawidget.h"

class AnalyzerDataStorage;
struct data_widget_info;

struct graph_data_st
{
    graph_data_st(const qreal& v, const int& i)
    {
        val = v;
        itr = i;
    }

    qreal val;
    quint32 itr;
};

class GraphDataSimple : public QwtSeriesData<QPointF>
{
public:
    typedef std::deque<graph_data_st*> storage;
    GraphDataSimple();

    QPointF sample(size_t i) const;
    size_t  size() const;
    QRectF boundingRect() const;

    void addPoint(quint32 index, qreal data);
    qint32 getMax() { return m_max; }
    qint32 getMin() { return m_min; }
    void clear();

    virtual void setSampleSize(qint32) { }
    virtual void dataPosChanged(quint32) { }

    virtual void setDataType(quint8) { }
    virtual quint8 getDataType() { return 0; }
    virtual void setInfo(data_widget_info&) { }

protected:
    void setMinMax(double val);
    void resetMinMax();

    storage m_data;
    QHash<quint32, graph_data_st*> m_indexes;

    qint32 m_min;
    qint32 m_max;
};

class GraphData : public GraphDataSimple
{
public:
    GraphData(AnalyzerDataStorage *storage, data_widget_info& info, qint32 sample_size, quint8 data_type);

    void setSampleSize(qint32 size);
    void dataPosChanged(quint32 pos);

    void setDataType(quint8 type);
    quint8 getDataType() { return m_data_type; }
    void setInfo(data_widget_info& info);
private:
    void eraseSpareData(qint32 absPos, quint32 pos);
    quint32 getStorageBegin(qint32 absPos);

    AnalyzerDataStorage *m_storage;
    data_widget_info m_info;

    qint32 m_sample_size;

    quint32 m_data_pos;
    quint8 m_data_type;
};

#endif // GRAPHDATA_H
