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
#include <QMouseEvent>
#include <QPainter>

#include "analyzerdataarea.h"
#include "DataWidgets/numberwidget.h"
#include "DataWidgets/barwidget.h"
#include "DataWidgets/colorwidget.h"
#include "DataWidgets/GraphWidget/graphwidget.h"
#include "DataWidgets/ScriptWidget/scriptwidget.h"
#include "lorrisanalyzer.h"
#include "analyzerdatafile.h"
#include "analyzerdatastorage.h"

AnalyzerDataArea::AnalyzerDataArea(LorrisAnalyzer *analyzer, AnalyzerDataStorage *storage) :
    QFrame(analyzer)
{
    setFrameStyle(QFrame::Panel | QFrame::Plain);
    setAcceptDrops(true);
    m_widgetIdCounter = 0;
    m_storage = storage;
    m_analyzer = analyzer;

    m_skipNextMove = false;
}

AnalyzerDataArea::~AnalyzerDataArea()
{
    clear();
}

void AnalyzerDataArea::clear()
{
    for(w_map::iterator itr = m_widgets.begin(); itr != m_widgets.end(); ++itr)
        delete itr.value();
    m_widgets.clear();
    m_marks.clear();
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
    w->setUp(m_storage);

    w->move(pos);
    if(show)
        w->show();

    quint32 id = getNewId();
    w->setId(id);
    m_widgets.insert(id, w);

    connect(m_analyzer, SIGNAL(newData(analyzer_data*,quint32)), w, SLOT(newData(analyzer_data*,quint32)));
    connect(w,          SIGNAL(removeWidget(quint32)),              SLOT(removeWidget(quint32)));
    connect(w,          SIGNAL(updateMarker(DataWidget*)),          SLOT(updateMarker(DataWidget*)));
    connect(w,          SIGNAL(updateData()),                       SIGNAL(updateData()));
    connect(w,          SIGNAL(mouseStatus(bool,data_widget_info)), SIGNAL(mouseStatus(bool,data_widget_info)));
    connect(w,  SIGNAL(SendData(QByteArray)), m_analyzer->getCon(), SLOT(SendData(QByteArray)));

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
        case WIDGET_SCRIPT:  return new ScriptWidget(parent);
    }
    return NULL;
}

void AnalyzerDataArea::removeWidget(quint32 id)
{
    w_map::iterator itr = m_widgets.find(id);
    if(itr == m_widgets.end())
        return;
    delete itr.value();
    m_widgets.erase(itr);

    m_marks.remove(id);
    update();
}

void AnalyzerDataArea::SaveWidgets(AnalyzerDataFile *file)
{
    // write widget count
    quint32 count = m_widgets.size();
    file->write((char*)&count, sizeof(quint32));

    // We want widgets saved in same order as they were created. It does not have to be super-fast,
    // so I am using std::map to sort them.
    std::map<quint32, DataWidget*> widgets;
    for(w_map::iterator itr = m_widgets.begin(); itr != m_widgets.end(); ++itr)
        widgets[itr.key()] = *itr;

    for(std::map<quint32, DataWidget*>::iterator itr = widgets.begin(); itr != widgets.end(); ++itr)
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
        else
            updateMarker(w);
    }
    update();
}

void AnalyzerDataArea::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_mouse_orig = event->globalPos();
        setCursor(Qt::SizeAllCursor);
    }
}

void AnalyzerDataArea::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        setCursor(Qt::ArrowCursor);
}

void AnalyzerDataArea::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    event->accept();

    if(m_marks.empty())
        return;

    QPainter painter(this);

    painter.setPen(Qt::red);
    painter.setBrush(QBrush(Qt::red, Qt::SolidPattern));

    for(mark_map::iterator itr = m_marks.begin(); itr != m_marks.end(); ++itr)
        painter.drawRect(*itr);
}

void AnalyzerDataArea::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() != Qt::LeftButton)
        return;

    QPoint n = event->globalPos() - m_mouse_orig;

    moveWidgets(n);

    m_mouse_orig = event->globalPos();
}

void AnalyzerDataArea::moveWidgets(QPoint diff)
{
    for(w_map::iterator itr = m_widgets.begin(); itr != m_widgets.end(); ++itr)
    {
        QPoint pos = (*itr)->pos() + diff;
        (*itr)->move(pos);

        updateMarker(*itr);
    }
    update();
}

void AnalyzerDataArea::getMarkPos(int &x, int &y, QSize &size)
{
    size = (y < 0 || y > height()) ? QSize(20, 5) : QSize(5, 20);

    if     (x < 0)       x = 0;
    else if(x > width()) x = width() - 5;

    if     (y < 0)        y = 0;
    else if(y > height()) y = height() - 5;
}

void AnalyzerDataArea::updateMarker(DataWidget *w)
{
    bool do_update = false;

    if(!rect().intersects(w->geometry()))
    {
        QPoint markPos(w->pos().x(), w->pos().y());
        markPos.rx() += w->width()/2;
        markPos.ry() += w->height()/2;

        QSize size;
        getMarkPos(markPos.rx(), markPos.ry(), size);

        m_marks[w->getId()] = QRect(markPos, size);

        do_update = true;

        w->setUpdating(false);
    }
    else
    {
        do_update = m_marks.remove(w->getId());
        if(do_update)
        {
            w->setUpdating(true);

            quint32 idx = 0;
            analyzer_data *data = m_analyzer->getLastData(idx);
            if(data)
                w->newData(data, idx);
        }
    }

    if(do_update)
        update();
}

void AnalyzerDataArea::moveEvent(QMoveEvent *event)
{
    if(m_skipNextMove)
        m_skipNextMove = false;
    else
        moveWidgets(event->oldPos() - pos());
}

void AnalyzerDataArea::resizeEvent(QResizeEvent *)
{
    for(w_map::iterator itr = m_widgets.begin(); itr != m_widgets.end(); ++itr)
        updateMarker(*itr);
}
