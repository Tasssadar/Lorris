/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "dep/qcustomplot/qcustomplot.h"
#include "plotwidget.h"
#include "plotcurve.h"
#include "../../datafilter.h"

REGISTER_DATAWIDGET(WIDGET_PLOT, Plot, NULL)
W_TR(QT_TRANSLATE_NOOP("DataWidget", "Plot"))

PlotWidget::PlotWidget(QWidget *parent) :
    DataWidget(parent)
{
    m_plot = new QCustomPlot(this);
    m_plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_plot->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    m_plot->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    m_plot->setInteraction(QCustomPlot::iSelectPlottables);
    m_plot->setAntialiasedElements(QCP::aeNone);

    layout->addWidget(m_plot, 1);

    resize(400, 200);

    m_state |= STATE_ASSIGNED;

    PlotCurve *c = new PlotCurve(m_plot->addGraph(), this);
    data_widget_info i;
    i.pos = 8;
    i.filter = new EmptyFilter(123213, "nic", this);
    c->setInfo(i);
    c->setSampleSize(10000);
    c->setDataType(NUM_INT16);

    m_curves.push_back(c);

    QTimer *t = new QTimer(this);
    t->start(100);
    connect(t, SIGNAL(timeout()), m_plot, SLOT(replot()));
}

void PlotWidget::setUp(Storage *storage)
{
    DataWidget::setUp(storage);

    m_storage = storage;
}

void PlotWidget::saveWidgetInfo(DataFileParser *file)
{
    DataWidget::saveWidgetInfo(file);
}

void PlotWidget::loadWidgetInfo(DataFileParser *file)
{
    DataWidget::loadWidgetInfo(file);
}

void PlotWidget::newData(analyzer_data */*data*/, quint32 index)
{
    if(!isUpdating() || m_curves.empty())
        return;

    for(size_t i = 0; i < m_curves.size(); ++i)
        m_curves[i]->newData(index, m_storage);
}

void PlotWidget::processData(analyzer_data */*data*/)
{
    // Data processed in ::newData()
}
