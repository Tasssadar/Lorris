#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <qwt_plot.h>

#include "datawidget.h"

class Graph;

class GraphWidget : public DataWidget
{
    Q_OBJECT
public:
    GraphWidget(QWidget *parent = 0);
    ~GraphWidget();

    void setUp();
    void saveWidgetInfo(AnalyzerDataFile *file);
    void loadWidgetInfo(AnalyzerDataFile *file);

protected:
     void processData(analyzer_data *data);
     void dropEvent(QDropEvent *event);

private:
     Graph *m_graph;
};

class GraphWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    GraphWidgetAddBtn(QWidget *parent = 0);
};

class Graph : public QwtPlot
{
    Q_OBJECT
public:
    Graph(QWidget *parent = 0);
};


#endif // GRAPHWIDGET_H
