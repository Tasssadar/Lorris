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
