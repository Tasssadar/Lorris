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
    ~GraphDataSimple();

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

    QString getFormula() { return m_eval.getFormula(); }
    void setFormula(const QString& f) { m_eval.setFormula(f); }

protected:
    void setMinMax(double val);
    void resetMinMax();

    storage m_data;
    QHash<quint32, graph_data_st*> m_indexes;

    qint32 m_min;
    qint32 m_max;
    FormulaEvaluation m_eval;
};

class GraphData : public GraphDataSimple
{
public:
    GraphData(Storage *storage, data_widget_info& info, qint32 sample_size, quint8 data_type);

    void setSampleSize(qint32 size);
    void dataPosChanged(quint32 pos);

    void setDataType(quint8 type);
    quint8 getDataType() { return m_data_type; }
    void setInfo(data_widget_info& info);
private:
    void eraseSpareData(qint32 absPos, quint32 pos);
    quint32 getStorageBegin(qint32 absPos);

    Storage *m_storage;
    data_widget_info m_info;

    qint32 m_sample_size;

    quint32 m_data_pos;
    quint8 m_data_type;
};

#endif // GRAPHDATA_H
