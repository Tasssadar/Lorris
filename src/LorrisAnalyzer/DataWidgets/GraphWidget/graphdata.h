#ifndef GRAPHDATA_H
#define GRAPHDATA_H

#include <qwt_series_data.h>
#include "../datawidget.h"

class AnalyzerDataStorage;
struct data_widget_info;

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
    AnalyzerDataStorage *m_storage;
    data_widget_info m_info;

    std::vector<double> m_data;
    qint32 m_sample_size;

    qint32 m_min;
    qint32 m_max;
    quint32 m_data_pos;
    quint8 m_data_type;
};

#endif // GRAPHDATA_H
