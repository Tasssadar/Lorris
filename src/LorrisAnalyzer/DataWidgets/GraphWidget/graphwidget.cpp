/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QDropEvent>
#include <qwt_plot_curve.h>
#include <QMenu>
#include <QSignalMapper>
#include <QInputDialog>
#include <QTimer>
#include <QColorDialog>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>

#include "graphwidget.h"
#include "graph.h"
#include "graphdialogs.h"
#include "graphdata.h"
#include "graphcurve.h"
#include "../../storage.h"
#include "graphexport.h"
#include "../../datafilter.h"

REGISTER_DATAWIDGET(WIDGET_GRAPH, Graph, NULL)

static const int sampleValues[SAMPLE_ACT_COUNT] = { -1, -2, -3, 10, 50, 100, 200, 500, 1000 };

GraphWidget::GraphWidget(QWidget *parent) : DataWidget(parent)
{
    m_widgetType = WIDGET_GRAPH;

    setTitle(tr("Graph"));
    setIcon(":/dataWidgetIcons/graph.png");

    m_graph = new Graph(this);
    m_add_dialog = NULL;

    layout->addWidget(m_graph);

    resize(400, 200);
}

GraphWidget::~GraphWidget()
{
    delete m_graph;
    delete m_add_dialog;

    for(quint8 i = 0; i < m_curves.size(); ++i)
        delete m_curves[i];
}

void GraphWidget::setUp(Storage *storage)
{
    DataWidget::setUp(storage);

    m_storage = storage;
    m_doReplot = false;

    m_editCurve = contextMenu->addAction(tr("Edit curve properties"));
    m_editCurve->setEnabled(false);

    m_deleteCurve = contextMenu->addMenu(tr("Remove Curve"));
    m_deleteCurve->setEnabled(false);
    m_deleteMap = NULL;

    QAction *removeAllCurves = m_deleteCurve->addAction(tr("Remove all curves"));
    m_deleteCurve->addSeparator();

    QMenu *sampleSize = contextMenu->addMenu(tr("Sample size"));

    QSignalMapper *sampleMap = new QSignalMapper(this);
    static const QString sampleNames[SAMPLE_ACT_COUNT] =
    {
        tr("Show all data"), tr("Set custom..."), tr("According to X axis"), "10", "50", "100", "200", "500", "1000"
    };

    for(quint8 i = 0; i < SAMPLE_ACT_COUNT; ++i)
    {
        if(i == 3)
            sampleSize->addSeparator();

        m_sample_act[i] = sampleSize->addAction(sampleNames[i]);
        m_sample_act[i]->setCheckable(true);
        sampleMap->setMapping(m_sample_act[i], sampleValues[i]);
        connect(m_sample_act[i], SIGNAL(triggered()), sampleMap, SLOT(map()));
    }

    m_sample_size_idx = 2;
    m_sample_size = -3;
    m_sample_act[2]->setChecked(true);

    updateSampleSize();

    QAction *exportAct = contextMenu->addAction(tr("Export data..."));
    QAction *bgAct = contextMenu->addAction(tr("Change background..."));

    m_showLegend = contextMenu->addAction(tr("Show legend"));
    m_showLegend->setCheckable(true);
    m_showLegend->setChecked(true);

    m_autoScroll = contextMenu->addAction(tr("Automaticaly scroll graph"));
    m_autoScroll->setCheckable(true);
    toggleAutoScroll(true);

    QTimer *replotTimer = new QTimer(this);
    replotTimer->start(100);

    connect(m_editCurve,  SIGNAL(triggered()),        SLOT(editCurve()));
    connect(exportAct,    SIGNAL(triggered()),        SLOT(exportData()));
    connect(bgAct,        SIGNAL(triggered()),        SLOT(changeBackground()));
    connect(sampleMap,    SIGNAL(mapped(int)),        SLOT(sampleSizeChanged(int)));
    connect(m_showLegend, SIGNAL(triggered(bool)),    SLOT(showLegend(bool)));
    connect(m_autoScroll, SIGNAL(triggered(bool)),    SLOT(toggleAutoScroll(bool)));
    connect(m_graph,      SIGNAL(updateSampleSize()), SLOT(updateSampleSize()));
    connect(replotTimer,  SIGNAL(timeout()),          SLOT(tryReplot()));
    connect(removeAllCurves, SIGNAL(triggered()),     SLOT(removeAllCurves()));
}

void GraphWidget::updateRemoveMapping()
{
    delete m_deleteMap;
    m_deleteMap = new QSignalMapper(this);
    connect(m_deleteMap, SIGNAL(mapped(QString)), SLOT(removeCurve(QString)), Qt::QueuedConnection);

    for(QHash<QString, QAction*>::iterator itr = m_deleteAct.begin(); itr != m_deleteAct.end(); ++itr)
    {
        m_deleteMap->setMapping(*itr, itr.key());
        connect(*itr, SIGNAL(triggered()), m_deleteMap, SLOT(map()));
    }
}

void GraphWidget::newData(analyzer_data */*data*/, quint32 index)
{
    if(!isUpdating() || m_curves.empty())
        return;

    for(quint8 i = 0; i < m_curves.size(); ++i)
        m_curves[i]->curve->dataPosChanged(index);

    updateVisibleArea();
}

void GraphWidget::processData(analyzer_data */*data*/)
{

}

void GraphWidget::saveWidgetInfo(DataFileParser *file)
{
    DataWidget::saveWidgetInfo(file);

    file->writeBlockIdentifier("graphWSample");
    {
        file->write((char*)&m_sample_size, sizeof(m_sample_size));
    }

    file->writeBlockIdentifier("graphWLegend");
    {
        bool showLegend = m_showLegend->isChecked();
        file->write((char*)&showLegend, 1);
    }

    // axis range
    file->writeBlockIdentifier("graphWAxisRange");
    {
        double vals[] = { m_graph->XlowerBound(), m_graph->XupperBound(), m_graph->YlowerBound(), m_graph->YupperBound() };
        file->write((char*)&vals, sizeof(vals));
    }

    // autoscroll
    file->writeBlockIdentifier("graphWAutoScroll");
    {
        file->write((char*)&m_enableAutoScroll, sizeof(bool));
    }

    // Graph data
    m_graph->saveData(file);

    // curves
    file->writeBlockIdentifier("graphWCurveCount");
    {
        quint32 size = m_curves.size();
        file->write((char*)&size, sizeof(quint32));
    }

    for(quint8 i = 0; i < m_curves.size(); ++i)
    {
        GraphCurveInfo *info = m_curves[i];

        file->writeBlockIdentifier("graphWCurve");

        // curve name
        file->writeBlockIdentifier("graphWCurveName");
        QByteArray title = info->curve->title().text().toAscii();
        quint32 size = title.length();
        file->write((char*)&size, sizeof(quint32));
        file->write(title.data());

        // data info
        file->writeBlockIdentifier("graphWCurveDataInfoV2");
        saveDataInfo(file, info->info);

        // data type
        file->writeBlockIdentifier("graphWCurveDataType");
        quint8 type = info->curve->getDataType();
        file->write((char*)&type, sizeof(quint8));

        // color
        file->writeBlockIdentifier("graphWCurveColor");
        file->writeString(info->curve->pen().color().name());

        // formula
        file->writeBlockIdentifier("graphWCurveFormula");
        file->writeString(info->curve->getFormula());

        // visibility
        file->writeBlockIdentifier("graphWCurveVisible");
        file->writeVal(info->curve->isVisible());
    }
}

void GraphWidget::loadWidgetInfo(DataFileParser *file)
{
    DataWidget::loadWidgetInfo(file);

    if(file->seekToNextBlock("graphWSample", BLOCK_WIDGET))
    {
        file->read((char*)&m_sample_size, sizeof(m_sample_size));

        m_sample_size_idx = -2;

        for(quint8 i = 0; i < sizeof(sampleValues)/sizeof(int); ++i)
        {
            if(sampleValues[i] == m_sample_size)
                m_sample_size_idx = i;
            m_sample_act[i]->setChecked(sampleValues[i] == m_sample_size);
        }
        if(m_sample_size == -2)
            m_sample_act[1]->setChecked(true);
    }

    if(file->seekToNextBlock("graphWLegend", BLOCK_WIDGET))
    {
        bool show;
        file->read((char*)&show, 1);
        showLegend(show);
    }

    // axis range
    if(file->seekToNextBlock("graphWAxisRange", BLOCK_WIDGET))
    {
        double vals[4];
        file->read((char*)&vals, sizeof(vals));

        m_graph->setAxisScale(QwtPlot::xBottom, vals[0], vals[1]);
        m_graph->setAxisScale(QwtPlot::yLeft, vals[2], vals[3]);
    }

    // autoscroll
    if(file->seekToNextBlock("graphWAutoScroll", BLOCK_WIDGET))
    {
        bool enable;
        file->read((char*)&enable, sizeof(enable));
        toggleAutoScroll(enable);
    }

    // Graph data
    m_graph->loadData(file);

    if(!file->seekToNextBlock("graphWCurveCount", BLOCK_WIDGET))
        return;

    quint32 curveCount;
    file->read((char*)&curveCount, sizeof(quint32));

    for(quint32 i = 0; i < curveCount; ++i)
    {
        if(!file->seekToNextBlock("graphWCurve", BLOCK_WIDGET))
            continue;

        QString name;
        quint8 dataType;
        data_widget_info info;
        QString color;

        // title
        if(!file->seekToNextBlock("graphWCurveName", "graphWCurve"))
            continue;
        {
            quint32 size = 0;
            file->read((char*)&size, sizeof(quint32));
            name = file->read(size);
        }

        // data info
        if(file->seekToNextBlock("graphWCurveDataInfoV2", "graphWCurve"))
            loadDataInfo(file, info);
        else if(file->seekToNextBlock("graphWCurveDataInfo", "graphWCurve"))
            loadOldDataInfo(file, info);
        else
            continue;

        // data type
        if(!file->seekToNextBlock("graphWCurveDataType", "graphWCurve"))
            continue;
        file->read((char*)&dataType, sizeof(quint8));

        // color
        if(!file->seekToNextBlock("graphWCurveColor", "graphWCurve"))
            continue;
        color = file->readString();

        GraphDataSimple *dta = new GraphData(m_storage, info, m_sample_size, dataType);
        GraphCurve *curve = new GraphCurve(name, dta);

        curve->setPen(QPen(QColor(color)));
        curve->attach(m_graph);
        m_graph->showCurve(curve, true);
        m_curves.push_back(new GraphCurveInfo(curve, info));

        QAction *deleteCurve = m_deleteCurve->addAction(name);
        m_deleteCurve->setEnabled(true);
        m_deleteAct[name] = deleteCurve;

        m_editCurve->setEnabled(true);

        if(file->seekToNextBlock("graphWCurveFormula", "graphWCurve"))
            curve->setFormula(file->readString());

        if(file->seekToNextBlock("graphWCurveVisible", "graphWCurve"))
            m_graph->showCurve(curve, file->readVal<bool>());
    }
    updateRemoveMapping();
}

void GraphWidget::dropEvent(QDropEvent *event)
{
    event->acceptProposedAction();

    quint32 pos;
    DataFilter *f;

    QByteArray data = event->mimeData()->data("analyzer/dragLabel");
    QDataStream str(data);

    str >> pos;
    str.readRawData((char*)&f, sizeof(f));

    m_dropData = std::make_pair(pos, f);

    if(m_add_dialog)
        delete m_add_dialog;
    m_add_dialog = new GraphCurveAddDialog(this, &m_curves, false);
    connect(m_add_dialog, SIGNAL(accepted()), this, SLOT(acceptCurveChanges()));
    m_add_dialog->open();
}

void GraphWidget::applyCurveChanges()
{
    if(!m_add_dialog->forceEdit())
        setInfo(m_dropData.second, m_dropData.first);

    if(!m_add_dialog->edit())
    {
        GraphData *data = new GraphData(m_storage, m_info, m_sample_size, m_add_dialog->getDataType());
        GraphCurve *curve = new GraphCurve(m_add_dialog->getName(), data);
        curve->setPen(QPen(m_add_dialog->getColor()));
        curve->attach(m_graph);
        data->setFormula(m_add_dialog->getFormula());
        m_graph->showCurve(curve, true);
        m_curves.push_back(new GraphCurveInfo(curve, m_info));

        QAction *deleteCurve = m_deleteCurve->addAction(m_add_dialog->getName());
        m_deleteCurve->setEnabled(true);
        m_deleteAct[m_add_dialog->getName()] = deleteCurve;

        m_editCurve->setEnabled(true);

        m_info.filter->connectWidget(this, false);
    }
    else
    {
        GraphCurveInfo *info = NULL;
        for(quint8 i = 0; !info && i < m_curves.size(); ++i)
        {
            if(m_curves[i]->curve->title().text() == m_add_dialog->getEditName())
                info = m_curves[i];
        }
        if(!info)
            return;

        QString curName = info->curve->title().text();

        QAction *deleteCurve = m_deleteAct[curName];
        m_deleteCurve->removeAction(deleteCurve);
        m_deleteAct.remove(curName);
        delete deleteCurve;

        deleteCurve = m_deleteCurve->addAction(m_add_dialog->getName());
        m_deleteAct[m_add_dialog->getName()] = deleteCurve;

        info->curve->setTitle(m_add_dialog->getName());
        info->curve->setPen(QPen(m_add_dialog->getColor()));
        if(!m_add_dialog->forceEdit())
        {
            info->curve->setDataInfo(m_info);
            info->info = m_info;
            m_info.filter->connectWidget(this, false);
        }
        info->curve->setDataType(m_add_dialog->getDataType());
        info->curve->setFormula(m_add_dialog->getFormula());
    }

    updateRemoveMapping();
    updateVisibleArea();

    emit updateForMe();
}

void GraphWidget::acceptCurveChanges()
{
    applyCurveChanges();

    delete m_add_dialog;
    m_add_dialog = NULL;

    m_state |= STATE_ASSIGNED;
}

void GraphWidget::updateVisibleArea()
{
    if(m_curves.empty())
        return;
    m_doReplot = true;
}

void GraphWidget::tryReplot()
{
    if(m_doReplot)
    {
        if(m_enableAutoScroll && !m_curves.empty())
        {
            qint32 size = 0;

            for(quint8 i = 0; i < m_curves.size(); ++i)
            {
                qint32 c_size = m_curves[i]->curve->getSize();

                if(c_size > size)
                    size = c_size;
            }

            qint32 x_max = m_graph->XupperBound() - m_graph->XlowerBound();
            m_graph->setAxisScale(QwtPlot::xBottom, size - x_max, size);
        }

        m_graph->replot();
        m_doReplot = false;
    }
}

void GraphWidget::sampleSizeChanged(int val)
{
    if(val != -2 && sampleValues[m_sample_size_idx] == val)
        return;

    int sample = val;
    bool ok = true;
    if(val == -2)
    {
        sample = QInputDialog::getInt(this, tr("Set sample size"), tr("Sample size:"), 0, 0, 2147483647, 1, &ok);
        if(ok)
            m_sample_size_idx = -2;
    }

    for(quint8 i = 0; i < sizeof(sampleValues)/sizeof(int); ++i)
    {
        if(sampleValues[i] == val)
            m_sample_size_idx = i;
        m_sample_act[i]->setChecked(sampleValues[i] == val);
    }

    if(!ok || m_sample_size == sample)
        return;

    if(sample != -3)
    {
        for(quint8 i = 0; i < m_curves.size(); ++i)
            m_curves[i]->curve->setSampleSize(sample);
    }
    else
        updateSampleSize();

    m_sample_size = sample;

    m_showLegend->setEnabled(sample == -1);

    updateVisibleArea();
}

void GraphWidget::editCurve()
{
    if(m_add_dialog)
        delete m_add_dialog;

    m_add_dialog = new GraphCurveAddDialog(this, &m_curves, true);
    m_add_dialog->open();

    connect(m_add_dialog, SIGNAL(accepted()), SLOT(acceptCurveChanges()));
    connect(m_add_dialog, SIGNAL(apply()),    SLOT(applyCurveChanges()));
}

void GraphWidget::removeCurve(QString name)
{
    std::vector<GraphCurveInfo*>::iterator itr = m_curves.begin();
    for(; itr != m_curves.end(); ++itr)
    {
        QString cur_name = (*itr)->curve->title().text();
        if(cur_name == name)
            break;
    }

    if(itr == m_curves.end())
        return;

    GraphCurveInfo *info = *itr;
    m_curves.erase(itr);

    delete info->curve;
    delete info;

    m_deleteCurve->removeAction(m_deleteAct[name]);
    delete m_deleteAct[name];
    m_deleteAct.remove(name);

    m_doReplot = true;

    m_editCurve->setEnabled(!m_curves.empty());
    m_deleteCurve->setEnabled(!m_curves.empty());

    updateRemoveMapping();
}

void GraphWidget::removeAllCurves()
{
    while(!m_curves.empty())
    {
        GraphCurveInfo *info = m_curves.back();
        m_curves.pop_back();

        QString name = info->curve->title().text();
        delete info->curve;
        delete info;

        m_deleteCurve->removeAction(m_deleteAct[name]);
        delete m_deleteAct[name];
        m_deleteAct.remove(name);
    }

    m_editCurve->setEnabled(false);
    m_deleteCurve->setEnabled(false);

    m_doReplot = true;
}

void GraphWidget::showLegend(bool show)
{
    m_showLegend->setChecked(show);
    m_graph->showLegend(show);
}

void GraphWidget::toggleAutoScroll(bool scroll)
{
    m_autoScroll->setChecked(scroll);
    m_enableAutoScroll = scroll;
}

GraphCurve *GraphWidget::addCurve(QString name, QString color)
{
    for(quint32 i = 0; i < m_curves.size(); ++i)
        if(m_curves[i]->curve->title().text() == name)
            return m_curves[i]->curve;

    GraphDataSimple *dta = new GraphDataSimple();
    GraphCurve *curve = new GraphCurve(name, dta);

    curve->setPen(QPen(QColor(color)));
    curve->attach(m_graph);
    m_graph->showCurve(curve, true);
    m_curves.push_back(new GraphCurveInfo(curve, m_info));

    QAction *deleteCurve = m_deleteCurve->addAction(name);
    m_deleteCurve->setEnabled(true);
    m_deleteAct[name] = deleteCurve;

    m_editCurve->setEnabled(true);

    updateRemoveMapping();
    return curve;
}

void GraphWidget::setAxisScale(bool x, double min, double max)
{
    int axis = x ? QwtPlot::xBottom : QwtPlot::yLeft;

    m_graph->setAxisScale(axis, min, max);
}

void GraphWidget::updateSampleSize()
{
    if(m_sample_size != -3)
        return;

    qint32 size = abs(m_graph->XupperBound() - m_graph->XlowerBound())*2;

    for(quint8 i = 0; i < m_curves.size(); ++i)
        m_curves[i]->curve->setSampleSize(size);
}

void GraphWidget::exportData()
{
    GraphExport ex(&m_curves, this);
    ex.exec();
}

void GraphWidget::changeBackground()
{
    QColor c = QColorDialog::getColor(m_graph->canvas()->palette().color(QPalette::Window));

    if(!c.isValid())
        return;

    m_graph->setBgColor(c);
}

GraphWidgetAddBtn::GraphWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Graph"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/graph.png"));

    m_widgetType = WIDGET_GRAPH;
}
