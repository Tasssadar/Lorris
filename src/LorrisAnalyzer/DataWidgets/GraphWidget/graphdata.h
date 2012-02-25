#ifndef GRAPHDATA_H
#define GRAPHDATA_H

#include <qwt_series_data.h>
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

class GraphData : public QwtSeriesData<QPointF>
{
public:
    GraphData(AnalyzerDataStorage *storage, data_widget_info& info, qint32 sample_size, quint8 data_type);

    QPointF sample(size_t i) const;
    size_t size() const;

    QRectF boundingRect() const;
    void setSampleSize(qint32 size);
    void dataPosChanged(quint32 pos);

    qint32 getMax() { return m_max; }
    qint32 getMin() { return m_min; }
    void setDataType(quint8 type);
    quint8 getDataType() { return m_data_type; }
    void setInfo(data_widget_info& info);

private:
    void eraseSpareData(qint32 absPos, quint32 pos);
    quint32 getStorageBegin(qint32 absPos);
    void setMinMax(double val);
    void resetMinMax();

    AnalyzerDataStorage *m_storage;
    data_widget_info m_info;

    std::vector<graph_data_st> m_data;
    qint32 m_sample_size;

    qint32 m_min;
    qint32 m_max;
    quint32 m_data_pos;
    quint8 m_data_type;
};

#endif // GRAPHDATA_H
