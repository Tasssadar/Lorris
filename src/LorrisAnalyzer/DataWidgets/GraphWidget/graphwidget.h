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

#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include "../datawidget.h"

class QSignalMapper;
class Graph;
class GraphCurveAddDialog;
class GraphCurve;

#define SAMPLE_ACT_COUNT 9

struct GraphCurveInfo
{
    GraphCurveInfo(GraphCurve* curve, data_widget_info& info)
    {
        this->curve = curve;
        this->info = info;
    }

    GraphCurve* curve;
    data_widget_info info;
};

class GraphWidget : public DataWidget
{
    Q_OBJECT
public:
    GraphWidget(QWidget *parent = 0);
    ~GraphWidget();

    void setUp(Storage *storage);
    void saveWidgetInfo(DataFileParser *file);
    void loadWidgetInfo(DataFileParser *file);

public slots:
    GraphCurve *addCurve(QString name, QString color);
    void setAxisScale(bool x, double min, double max);
    void updateVisibleArea();

protected:
     void processData(analyzer_data *data);
     void dropEvent(QDropEvent *event);

private slots:
     void addCurve();
     void newData(analyzer_data *data, quint32 index);
     void sampleSizeChanged(int val);
     void editCurve();
     void removeCurve(QString name);
     void showLegend(bool show);
     void toggleAutoScroll(bool scroll);
     void updateSampleSize();

private:
     void updateRemoveMapping();

     Graph *m_graph;
     GraphCurveAddDialog *m_add_dialog;
     QString m_drop_data;
     Storage *m_storage;

     QAction *m_sample_act[SAMPLE_ACT_COUNT];
     QAction *m_editCurve;
     QAction *m_showLegend;
     QAction *m_autoScroll;

     QMenu *m_deleteCurve;
     std::map<QString, QAction*> m_deleteAct;
     QSignalMapper *m_deleteMap;

     int m_sample_size_idx;
     qint32 m_sample_size;
     bool m_enableAutoScroll;

     std::vector<GraphCurveInfo*> m_curves;
};

class GraphWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    GraphWidgetAddBtn(QWidget *parent = 0);
};

#endif // GRAPHWIDGET_H
