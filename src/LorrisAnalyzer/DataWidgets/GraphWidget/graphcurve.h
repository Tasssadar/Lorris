#ifndef GRAPHCURVE_H
#define GRAPHCURVE_H

#include <qwt_plot_curve.h>
#include "graphdata.h"
#include "../datawidget.h"

class AnalyzerDataStorage;

class GraphCurve : public QwtPlotCurve
{
public:
    GraphCurve(const QString& name, AnalyzerDataStorage *data, data_widget_info& info,
               qint32 sample_size, quint8 data_type);
    ~GraphCurve();

    void setSampleSize(qint32 size);
    void dataPosChanged(quint32 pos);

    qint32 getMin();
    qint32 getMax();
    quint32 getSize();
    void setDataType(quint8 type);

    void setDataInfo(data_widget_info& info)
    {
        m_data->setInfo(info);
    }

    quint8 getDataType() { return m_data->getDataType(); }

private:
    GraphData *m_data;
    qint32 m_sample_size;
};

#endif // GRAPHCURVE_H
