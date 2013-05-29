/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef PLOTCURVE_H
#define PLOTCURVE_H

#include <QObject>
#include "../datawidget.h"

class QCPGraph;

class PlotCurve : public QObject
{
    Q_OBJECT
public:
    PlotCurve(QCPGraph *graph, QObject *parent = 0);
    
    QCPGraph *g() const { return m_graph; }
    QCPGraph *graph() const { return m_graph; }

    void newData(quint32 index, Storage *storage);
    void setInfo(const data_widget_info& info) { m_info = info; }
    void setSampleSize(quint32 size) { m_sample_size = size; }
    void setDataType(quint8 type) { m_data_type = type; }

private:
    QCPGraph *m_graph;
    quint32 m_last_index;
    data_widget_info m_info;
    quint32 m_sample_size;
    quint8 m_data_type;
};

#endif // PLOTCURVE_H
