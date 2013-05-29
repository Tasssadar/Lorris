/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include <vector>
#include "../datawidget.h"

class QCustomPlot;
class PlotCurve;

class PlotWidget : public DataWidget
{
    Q_OBJECT
public:
    explicit PlotWidget(QWidget *parent = 0);

    void setUp(Storage *storage);
    void saveWidgetInfo(DataFileParser *file);
    void loadWidgetInfo(DataFileParser *file);
    
protected:
    void processData(analyzer_data *data);

private slots:
    void newData(analyzer_data *data, quint32 index);

private:
    QCustomPlot *m_plot;
    std::vector<PlotCurve*> m_curves;
    Storage *m_storage;
};

#endif // PLOTWIDGET_H
