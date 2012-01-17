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

#include <QDropEvent>

#include "graphwidget.h"
#include "graph.h"
#include "graphdialogs.h"

GraphWidget::GraphWidget(QWidget *parent) : DataWidget(parent)
{
    m_widgetType = WIDGET_GRAPH;

    setTitle(tr("Graph"));
    setIcon(":/dataWidgetIcons/graph.png");

    m_graph = new Graph(this);

    layout->addWidget(m_graph);

    resize(400, 200);
}

GraphWidget::~GraphWidget()
{

}

void GraphWidget::setUp()
{
    DataWidget::setUp();
}

void GraphWidget::processData(analyzer_data *data)
{

}

void GraphWidget::saveWidgetInfo(AnalyzerDataFile *file)
{
    DataWidget::saveWidgetInfo(file);
}

void GraphWidget::loadWidgetInfo(AnalyzerDataFile *file)
{
    DataWidget::loadWidgetInfo(file);
}

void GraphWidget::dropEvent(QDropEvent *event)
{
    event->acceptProposedAction();

    GraphCurveAddDialog *dialog = new GraphCurveAddDialog(this);
    dialog->exec();
    delete dialog;
}

GraphWidgetAddBtn::GraphWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Graph"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/graph.png"));

    m_widgetType = WIDGET_GRAPH;
}
