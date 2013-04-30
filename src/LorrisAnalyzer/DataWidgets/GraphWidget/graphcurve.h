/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef GRAPHCURVE_H
#define GRAPHCURVE_H

#include <qwt_plot_curve.h>

#include "graphdata.h"
#include "../datawidget.h"

class Storage;

class GraphCurve : public QObject, public QwtPlotCurve
{
    Q_OBJECT
public:
    GraphCurve(const QString& name, GraphDataSimple *data);
    GraphCurve(const GraphCurve &curve);
    GraphCurve()
    {
        GraphCurve("", NULL);
    }

    ~GraphCurve();

    void init();

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

    QString getFormula() { return m_data->getFormula(); }
    void setFormula(const QString& f) { m_data->setFormula(f); }

public slots:
    void addPoint(quint32 index, qreal val);
    void clear();

private:
    GraphDataSimple *m_data;
    qint32 m_sample_size;
};

Q_DECLARE_METATYPE(GraphCurve*)

#endif // GRAPHCURVE_H
