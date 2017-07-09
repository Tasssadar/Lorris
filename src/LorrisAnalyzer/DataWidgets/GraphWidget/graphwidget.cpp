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
#include <QTimer>
#include <QColorDialog>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <QMimeData>

#include "graphwidget.h"
#include "graph.h"
#include "graphdialogs.h"
#include "graphdata.h"
#include "graphcurve.h"
#include "../../storage.h"
#include "graphexport.h"
#include "../../datafilter.h"
#include "../../../ui/floatinginputdialog.h"
#include "../../labellayout.h"

REGISTER_DATAWIDGET(WIDGET_GRAPH, Graph, NULL)
W_TR(QT_TRANSLATE_NOOP("DataWidget", "Graph"))

static const int sampleValues[SAMPLE_ACT_COUNT] = { -1, -2, -3, 10, 50, 100, 200, 500, 1000 };

GraphWidget::GraphWidget(QWidget *parent) : DataWidget(parent)
{
    m_graph = new Graph(this);
    m_add_dialog = NULL;
    m_drop_layout = NULL;

    layout->addWidget(m_graph);

    resize(400, 200);
}

GraphWidget::~GraphWidget()
{
    delete m_drop_layout;
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
    m_indexChange = UINT32_MAX;

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

    QMenu *enableAxes = contextMenu->addMenu(tr("Visible axes"));
    QSignalMapper *axisMap = new QSignalMapper(this);
    static const QString axisNames[QwtPlot::xTop] = {
        tr("Y - left"), tr("Y - right"), ("X - bottom")
    };

    for(size_t i = 0; i < sizeof_array(axisNames); ++i)
    {
        m_axis_act[i] = enableAxes->addAction(axisNames[i]);
        m_axis_act[i]->setCheckable(true);
        axisMap->setMapping(m_axis_act[i], i);
        connect(m_axis_act[i], SIGNAL(triggered()), axisMap, SLOT(map()));
    }

    m_axis_act[QwtPlot::yLeft]->setChecked(true);
    m_axis_act[QwtPlot::xBottom]->setChecked(true);

    QAction *exportAct = contextMenu->addAction(tr("Export data..."));
    QAction *bgAct = contextMenu->addAction(tr("Change background..."));

    m_showLegend = contextMenu->addAction(tr("Show legend"));
    m_showLegend->setCheckable(true);
    m_showLegend->setChecked(true);

    m_autoScroll = contextMenu->addAction(tr("Automaticaly scroll graph"));
    m_autoScroll->setCheckable(true);
    toggleAutoScroll(true);

    QAction *rateAct = contextMenu->addAction(tr("Set refresh rate..."));

    m_refreshRateMs = 100;
    m_replotTimer = new QTimer(this);
    m_replotTimer->start(m_refreshRateMs);

    connect(m_editCurve,  SIGNAL(triggered()),        SLOT(editCurve()));
    connect(exportAct,    SIGNAL(triggered()),        SLOT(exportData()));
    connect(bgAct,        SIGNAL(triggered()),        SLOT(changeBackground()));
    connect(sampleMap,    SIGNAL(mapped(int)),        SLOT(sampleSizeChanged(int)));
    connect(axisMap,      SIGNAL(mapped(int)),        SLOT(toggleAxisVisibility(int)));
    connect(m_showLegend, SIGNAL(triggered(bool)),    SLOT(showLegend(bool)));
    connect(m_autoScroll, SIGNAL(triggered(bool)),    SLOT(toggleAutoScroll(bool)));
    connect(m_graph,      SIGNAL(updateSampleSize()), SLOT(updateSampleSize()));
    connect(m_replotTimer,SIGNAL(timeout()),          SLOT(tryReplot()));
    connect(removeAllCurves, SIGNAL(triggered()),     SLOT(removeAllCurves()));
    connect(rateAct,      SIGNAL(triggered()),        SLOT(setRefreshRateAct()));
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
    if(isUpdating() && !m_curves.empty())
        m_indexChange = index;
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

    // autoscroll
    file->writeBlockIdentifier("graphWAutoScroll");
    {
        file->write((char*)&m_enableAutoScroll, sizeof(bool));
    }

    // visible axes
    file->writeBlockIdentifier("graphWAxisVisibility");
    {
        const quint32 cnt = sizeof_array(m_axis_act);
        *file << cnt;
        for(quint32 i = 0; i < cnt; ++i)
            *file << m_graph->axisEnabled(i);
    }

    file->writeBlockIdentifier("graphWRefreshRate");
    {
        *file << m_refreshRateMs;
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
        file->writeString(info->curve->title().text());

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

        // axis
        file->writeBlockIdentifier("graphWCurveAxis");
        file->writeVal(info->curve->yAxis());

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

    // axis range - deprecated, moved to Graph class
    if(file->seekToNextBlock("graphWAxisRange", BLOCK_WIDGET))
    {
        double vals[4];
        file->read((char*)&vals, sizeof(vals));

        m_graph->setAxisScale(QwtPlot::xBottom, vals[0], vals[1]);
        m_graph->setAxisScale(QwtPlot::yLeft, vals[2], vals[3]);
        m_graph->syncYZeros();
    }

    // autoscroll
    if(file->seekToNextBlock("graphWAutoScroll", BLOCK_WIDGET))
    {
        bool enable;
        file->read((char*)&enable, sizeof(enable));
        toggleAutoScroll(enable);
    }

    // axis visibility
    if(file->seekToNextBlock("graphWAxisVisibility", BLOCK_WIDGET))
    {
        const quint32 cnt = file->readVal<quint32>();
        for(quint32 i = 0; i < cnt; ++i)
        {
            const bool enabled = file->readVal<bool>();
            m_graph->enableAxis(i, enabled);
            m_axis_act[i]->setChecked(enabled);
        }
    }

    // refresh rate
    if(file->seekToNextBlock("graphWRefreshRate", BLOCK_WIDGET))
        setRefreshRate(file->readVal<int>());

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
        int axis = QwtPlot::yLeft;

        // title
        if(!file->seekToNextBlock("graphWCurveName", "graphWCurve"))
            continue;

        name = file->readString();

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

        // axis
        if(file->seekToNextBlock("graphWCurveAxis", "graphWCurve"))
            axis = file->readVal<int>();

        GraphData *dta = new GraphData(m_storage, info, m_sample_size, dataType);
        GraphCurve *curve = new GraphCurve(name, dta);

        curve->setPen(QPen(QColor(color)));
        curve->setYAxis(axis);
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
    updateSampleSize();
}

void GraphWidget::dragEnterEvent(QDragEnterEvent *event)
{
    DataWidget::dragEnterEvent(event);
    if(!event->isAccepted())
        return;

    m_graph->setVisible(false);

    m_drop_cur_label = -1;
    m_drop_layout = new QVBoxLayout;
    m_drop_layout->addStretch(1);

    QLabel *headline = new QLabel(tr("Assign to curve:"), this);
    QFont f = headline->font();
    f.setPointSize(f.pointSize()*1.5);
    headline->setFont(f);
    m_drop_layout->addWidget(headline, 0, Qt::AlignHCenter);

    QHBoxLayout *l = new QHBoxLayout;
    m_drop_layout->addLayout(l);
    l->addStretch(1);

    QLabel *lab = new QLabel("New curve...", this);
    lab->setAutoFillBackground(true);
    lab->setStyleSheet("border: 2px solid black; padding: 15px 8px; background: white;");
    l->addWidget(lab);
    l->addSpacing(20);
    m_drop_labels.push_back(lab);

    for(size_t i = 0; i < m_curves.size(); ++i) {
        GraphCurveInfo *cd = m_curves[i];

        lab = new QLabel(cd->curve->title().text(), this);
        lab->setAutoFillBackground(true);
        QColor clr = cd->curve->pen().color();
        lab->setStyleSheet(QString("border: 4px solid rgb(%1, %2, %3); padding: 15px 8px; background: white;").arg(clr.red()).arg(clr.green()).arg(clr.blue()));

        l->addWidget(lab);
        l->addSpacing(10);
        m_drop_labels.push_back(lab);
    }
    l->addStretch(1);

    m_drop_layout->addStretch(1);

    layout->addLayout(m_drop_layout, 1);
}

void GraphWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    m_graph->setVisible(true);
    if(m_drop_layout) {
        m_drop_labels.clear();
        Utils::deleteLayoutMembers(m_drop_layout);
        delete m_drop_layout;
        m_drop_layout = NULL;
    }
}

void GraphWidget::dragMoveEvent(QDragMoveEvent *event)
{
    int newTarget = 0;
    for(size_t i = 0; i < m_drop_labels.size(); ++i) {
        QLabel *l = m_drop_labels[i];
        if(l->geometry().contains(event->pos())) {
            newTarget = i;
            break;
        }
    }

    if(newTarget != m_drop_cur_label) {
        if(m_drop_cur_label != -1) {
            QString s = m_drop_labels[m_drop_cur_label]->styleSheet();
            s.replace("background: yellow;", "background: white;");
            m_drop_labels[m_drop_cur_label]->setStyleSheet(s);
        }

        QString s = m_drop_labels[newTarget]->styleSheet();
        s.replace("background: white;", "background: yellow;");
        m_drop_labels[newTarget]->setStyleSheet(s);

        m_drop_cur_label = newTarget;
    }
}

void GraphWidget::dropEvent(QDropEvent *event)
{
    event->acceptProposedAction();

    m_graph->setVisible(true);
    if(m_drop_layout) {
        m_drop_labels.clear();
        Utils::deleteLayoutMembers(m_drop_layout);
        delete m_drop_layout;
        m_drop_layout = NULL;
    }

    quint32 pos;
    DataFilter *f;

    QByteArray data = event->mimeData()->data("analyzer/dragLabel");
    QDataStream str(data);

    str >> pos;
    str.readRawData((char*)&f, sizeof(f));

    if(m_drop_cur_label > 0) {
        emit mouseStatus(false, m_info, m_widgetControlled);

        setInfo(f, pos);

        GraphCurveInfo *targetCurve = m_curves[m_drop_cur_label-1];
        targetCurve->curve->setDataInfo(m_info);
        targetCurve->info = m_info;
        m_info.filter->connectWidget(this, false);

        updateRemoveMapping();
        updateVisibleArea();
        emit updateForMe();
    } else {
        m_dropData = std::make_pair(pos, f);

        if(m_add_dialog)
            delete m_add_dialog;
        m_add_dialog = new GraphCurveAddDialog(this, &m_curves, false);
        connect(m_add_dialog, SIGNAL(accepted()), this, SLOT(acceptCurveChanges()));
        m_add_dialog->open();
    }
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
        curve->setYAxis(m_add_dialog->getAxis());
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
        info->curve->setYAxis(m_add_dialog->getAxis());
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
    if(m_indexChange != UINT32_MAX) {
        const size_t size = m_curves.size();
        if(size != 0) {
            for(size_t i = 0; i < size; ++i) {
                if(m_enableAutoScroll && m_sample_size == -3)
                     m_curves[i]->curve->setSampleOffset(m_indexChange);
                m_curves[i]->curve->dataPosChanged(m_indexChange);
            }
            m_doReplot = true;
        }

        m_indexChange = UINT32_MAX;
    }

    if(m_doReplot)
    {
        if(m_enableAutoScroll && !m_curves.empty())
        {
            qint32 size = 0;

            for(quint8 i = 0; i < m_curves.size(); ++i)
            {
                qint32 c_size = m_curves[i]->curve->getMaxX();

                if(c_size > size)
                    size = c_size;
            }

            qint32 x_max = abs(m_graph->XupperBound() - m_graph->XlowerBound());
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
        sample = FloatingInputDialog::getInt(tr("Sample size:"), 0, 0, INT_MAX, 1, &ok);
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

    switch(sample)
    {
        case -3: // Automatic
            updateSampleSize();
            break;
        case -2: // User-specified
            break;
        case -1: // all data
            for(quint8 i = 0; i < m_curves.size(); ++i)
                m_curves[i]->curve->setSampleSize(UINT32_MAX);
            break;
        default:
            for(quint8 i = 0; i < m_curves.size(); ++i)
                m_curves[i]->curve->setSampleSize(sample);
            break;
    }

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

    GraphData *dta = new GraphData(m_storage, m_info, UINT_MAX, NUM_UINT8);
    dta->setScriptBased();

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
    m_graph->syncYZeros();
}

void GraphWidget::updateSampleSize()
{
    if(m_sample_size != -3)
        return;

    qint32 size = abs(m_graph->XupperBound() - m_graph->XlowerBound());

    for(quint8 i = 0; i < m_curves.size(); ++i)
        m_curves[i]->curve->setSampleSize(size, (std::max)(m_graph->XupperBound(), 0.0));
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

void GraphWidget::toggleAxisVisibility(int axis)
{
    m_axis_act[axis]->setChecked(!m_graph->axisEnabled(axis));
    m_graph->enableAxis(axis, !m_graph->axisEnabled(axis));
}

void GraphWidget::setRefreshRateAct() {
    bool ok;
    int newRate = FloatingInputDialog::getInt(tr("Refresh rate in ms:"), m_refreshRateMs, 0, INT_MAX, 1, &ok);
    if(ok) {
        setRefreshRate(newRate);
    }
}

void GraphWidget::setRefreshRate(int rateMs) {
    if(rateMs != m_refreshRateMs) {
        m_refreshRateMs = rateMs;
        m_replotTimer->start(rateMs);
    }
}
