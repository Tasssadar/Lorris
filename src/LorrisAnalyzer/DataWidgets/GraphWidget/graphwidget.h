/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QHash>
#include <qwt_plot.h>
#include "../datawidget.h"

class QSignalMapper;
class Graph;
class GraphCurveAddDialog;
class GraphCurve;
class QTimer;

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
    void removeCurve(QString name);
    void removeAllCurves();
    void updateVisibleArea();

protected:
    void processData(analyzer_data *data);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void applyCurveChanges();
    void acceptCurveChanges();
    void newData(analyzer_data *data, quint32 index);
    void sampleSizeChanged(int val);
    void editCurve();
    void showLegend(bool show);
    void toggleAutoScroll(bool scroll);
    void updateSampleSize();
    void tryReplot();
    void exportData();
    void changeBackground();
    void toggleAxisVisibility(int axis);
    void setRefreshRateAct();

private:
    void updateRemoveMapping();
    void setRefreshRate(int rateMs);

    Graph *m_graph;
    GraphCurveAddDialog *m_add_dialog;
    std::pair<quint32, DataFilter*> m_dropData;
    Storage *m_storage;

    QVBoxLayout *m_drop_layout;
    std::vector<QLabel*> m_drop_labels;
    int m_drop_cur_label;

    QAction *m_sample_act[SAMPLE_ACT_COUNT];
    QAction *m_axis_act[QwtPlot::xTop];
    QAction *m_editCurve;
    QAction *m_showLegend;
    QAction *m_autoScroll;

    QMenu *m_deleteCurve;
    QHash<QString, QAction*> m_deleteAct;
    QSignalMapper *m_deleteMap;

    int m_sample_size_idx;
    qint32 m_sample_size;
    bool m_enableAutoScroll;
    quint32 m_indexChange;
    int m_refreshRateMs;
    QTimer *m_replotTimer;

    std::vector<GraphCurveInfo*> m_curves;
    bool m_doReplot;
};

#endif // GRAPHWIDGET_H
