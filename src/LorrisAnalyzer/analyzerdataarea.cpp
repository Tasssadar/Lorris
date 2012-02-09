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
#include "lorrisanalyzer.h"
#include "analyzerdatafile.h"
#include "analyzerdatastorage.h"

AnalyzerDataArea::AnalyzerDataArea(QWidget *parent, AnalyzerDataStorage *storage) :
    QFrame(parent)
{
    setFrameStyle(QFrame::Panel | QFrame::Plain);
    setAcceptDrops(true);
    m_widgetIdCounter = 0;
    m_storage = storage;
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
    w->setUp(m_storage);


    fixWidgetPos(pos, w);
    w->move(pos);
    if(show)
        w->show();

    quint32 id = getNewId();
    w->setId(id);
    m_widgets.insert(std::make_pair<quint32,DataWidget*>(id, w));

    connect(((LorrisAnalyzer*)parent()), SIGNAL(newData(analyzer_data*,quint32)), w, SLOT(newData(analyzer_data*,quint32)));
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
    m_marks.erase(id);
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
        // create markers
        else if(w->pos().x() < 0       || w->pos().y() < 0 ||
                w->pos().x() > width() || w->pos().y() > height())
        {
            QPoint markPos(w->pos().x(), w->pos().y());
            QSize size;

            getMarkPos(markPos.rx(), markPos.ry(), size);

            m_marks[w->getId()] = QRect(markPos, size);
        }
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

    QPainter painter(this);

    painter.setPen(Qt::red);
    painter.setBrush(QBrush(Qt::red, Qt::SolidPattern));

    for(mark_map::iterator itr = m_marks.begin(); itr != m_marks.end(); ++itr)
        painter.drawRect(itr->second);

    event->accept();
}

void AnalyzerDataArea::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() != Qt::LeftButton)
        return;

    QPoint n = event->globalPos() - m_mouse_orig;

    for(w_map::iterator itr = m_widgets.begin(); itr != m_widgets.end(); ++itr)
    {
        QPoint pos = itr->second->pos() + n;
        itr->second->move(pos);

        if(pos.x() < 0       || pos.y() < 0 ||
           pos.x() > width() || pos.y() > height())
        {
            QPoint markPos(pos.x(), pos.y());
            QSize size;

            getMarkPos(markPos.rx(), markPos.ry(), size);

            m_marks[itr->first] = QRect(markPos, size);
        }
        else
            m_marks.erase(itr->first);
    }

    m_mouse_orig = event->globalPos();
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

