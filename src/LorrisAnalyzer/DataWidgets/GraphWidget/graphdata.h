/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef GRAPHDATA_H
#define GRAPHDATA_H

#include <qwt_series_data.h>
#include <deque>

#include "../datawidget.h"
#include "../../../misc/formulaevaluation.h"

class Storage;
struct data_widget_info;

class GraphData : public QwtSeriesData<QPointF>
{
public:
    typedef std::deque<QPointF> DataMap;
    typedef std::deque<QPointF>::iterator DataMapItr;

    GraphData(Storage *storage, data_widget_info& info, qint32 sample_size, quint8 data_type);
    ~GraphData();

    void setScriptBased(bool set = true) { m_script_based = set; }

    QPointF sample(size_t i) const;
    size_t size() const;
    QRectF boundingRect() const;

    void addPoint(qreal index, qreal data);
    qint32 getMax() { return m_max; }
    qint32 getMin() { return m_min; }
    quint32 getMaxX();
    void clear();
    void reloadData(bool force);

    void setSampleSize(quint32 size, quint32 offset);
    void setSampleOffset(quint32 offset);
    void dataPosChanged(quint32 index);

    void setDataType(quint8 type);
    quint8 getDataType() { return m_data_type; }
    void setInfo(data_widget_info&);

    QString getFormula() { return m_eval.getFormula(); }
    void setFormula(const QString& f) { m_eval.setFormula(f); }

private:
    void removeDataAfter(quint32 index);
    void removeDataBefore(quint32 index);
    inline DataMapItr insertData(DataMapItr hint, quint32 idx, double val);
    inline void setMinMax(double val);

    QPointF getPointAtIdx(quint32 idx);

    FormulaEvaluation m_eval;
    bool m_script_based;

    Storage *m_storage;
    data_widget_info m_info;

    quint32 m_sample_size;
    quint32 m_sample_offset;
    quint8 m_data_type;

    DataMap m_data;
    quint32 m_data_start;
    quint32 m_data_end;
    quint32 m_last_index;

    double m_min, m_max;
};

#endif // GRAPHDATA_H
