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

#include "analyzerdataarea.h"
#include "DataWidgets/numberwidget.h"
#include "DataWidgets/barwidget.h"
#include "DataWidgets/colorwidget.h"
#include "DataWidgets/graphwidget.h"
#include "lorrisanalyzer.h"
#include "analyzerdatafile.h"

AnalyzerDataArea::AnalyzerDataArea(QWidget *parent) :
    QFrame(parent)
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setAcceptDrops(true);
    m_widgetIdCounter = 0;
}

AnalyzerDataArea::~AnalyzerDataArea()
{
    clear();
}

void AnalyzerDataArea::clear()
{
    for(w_map::iterator itr = m_widgets.begin(); itr != m_widgets.end(); ++itr)
        delete itr->second;
    m_widgets.clear();
}

void AnalyzerDataArea::dropEvent(QDropEvent *event)
{
    QString data = event->mimeData()->text().remove(0, 1);
    quint8 type = data.toInt();

    addWidget(event->pos(), type);

    event->acceptProposedAction();
}

DataWidget *AnalyzerDataArea::addWidget(QPoint pos, quint8 type, bool show)
{
    DataWidget *w = newWidget(type, this);
    if(!w)
        return NULL;
    w->setUp();


    fixWidgetPos(pos, w);
    w->move(pos);
    if(show)
        w->show();

    quint32 id = getNewId();
    w->setId(id);
    m_widgets.insert(std::make_pair<quint32,DataWidget*>(id, w));

    connect(((LorrisAnalyzer*)parent()), SIGNAL(newData(analyzer_data*)), w, SLOT(newData(analyzer_data*)));
    connect(w, SIGNAL(updateData()), this, SIGNAL(updateData()));
    connect(w, SIGNAL(mouseStatus(bool,data_widget_info)), this, SIGNAL(mouseStatus(bool,data_widget_info)));
    connect(w, SIGNAL(removeWidget(quint32)), this, SLOT(removeWidget(quint32)));
    return w;
}

void AnalyzerDataArea::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->text().at(0) == 'w')
        event->acceptProposedAction();
    else
        QFrame::dragEnterEvent(event);
}

DataWidget *AnalyzerDataArea::newWidget(quint8 type, QWidget *parent)
{
    switch(type)
    {
        case WIDGET_NUMBERS: return new NumberWidget(parent);
        case WIDGET_BAR:     return new BarWidget(parent);
        case WIDGET_COLOR:   return new ColorWidget(parent);
        case WIDGET_GRAPH:   return new GraphWidget(parent);
    }
    return NULL;
}

void AnalyzerDataArea::removeWidget(quint32 id)
{
    w_map::iterator itr = m_widgets.find(id);
    if(itr == m_widgets.end())
        return;
    delete itr->second;
    m_widgets.erase(itr);
}

void AnalyzerDataArea::fixWidgetPos(QPoint &pos, QWidget *w)
{
    int x = pos.x() + w->width();
    int y = pos.y() + w->height();

    if(x > width())
        pos.setX(width() - w->width());

    if(y > height())
        pos.setY(height() - w->height());
}

void AnalyzerDataArea::SaveWidgets(AnalyzerDataFile *file)
{
    // write widget count
    quint32 count = m_widgets.size();
    file->write((char*)&count, sizeof(quint32));

    for(w_map::iterator itr = m_widgets.begin(); itr != m_widgets.end(); ++itr)
    {
        file->writeBlockIdentifier(BLOCK_WIDGET);
        itr->second->saveWidgetInfo(file);
    }
}

void AnalyzerDataArea::LoadWidgets(AnalyzerDataFile *file, bool skip)
{
    clear();

    quint32 count = 0;
    file->read((char*)&count, sizeof(quint32));

    for(quint32 i = 0; i < count; ++i)
    {
        if(!file->seekToNextBlock(BLOCK_WIDGET, 0))
            break;
        // type
        if(!file->seekToNextBlock("widgetType", BLOCK_WIDGET))
            break;
        quint8 type = 0;
        file->read((char*)&type, sizeof(quint8));

        // pos and size
        if(!file->seekToNextBlock("widgetPosSize", BLOCK_WIDGET))
            break;
        int val[4];
        file->read((char*)&val, sizeof(val));

        DataWidget *w = addWidget(QPoint(val[0], val[1]), type, !skip);
        if(!w)
            continue;
        w->resize(val[2], val[3]);
        w->loadWidgetInfo(file);
        if(skip)
            removeWidget(w->getId());
    }
}
