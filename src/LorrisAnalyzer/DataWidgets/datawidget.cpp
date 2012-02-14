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

#include <QLabel>
#include <QMouseEvent>
#include <QPoint>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QDrag>
#include <QMenu>
#include <QInputDialog>

#include "datawidget.h"
#include "WorkTab/WorkTab.h"
#include "../analyzerdataarea.h"
#include "../analyzerdatafile.h"

DataWidget::DataWidget(QWidget *parent) :
    QFrame(parent)
{
    layout = new QVBoxLayout(this);
    QHBoxLayout *title_bar = new QHBoxLayout();

    m_icon_widget = new QLabel(this);
    m_icon_widget->setStyleSheet("padding: 2px");

    m_title_label = new QLabel(this);
    m_title_label->setObjectName("titleLabel");
    m_title_label->setStyleSheet("border-right: 1px solid black; border-bottom: 1px solid black");
    m_title_label->setAlignment(Qt::AlignVCenter);

    m_closeLabel = new CloseLabel(this);

    QFrame *sepV = new QFrame(this);
    sepV->setFrameStyle(QFrame::HLine | QFrame::Plain);
    sepV->setLineWidth(1);

    title_bar->addWidget(m_icon_widget);
    title_bar->addWidget(m_title_label, 1);
    title_bar->addWidget(m_closeLabel, 0);

    layout->setMargin(0);
    title_bar->setMargin(0);

    layout->addLayout(title_bar);
    layout->addWidget(sepV);

    setFrameStyle(QFrame::Box | QFrame::Plain);
    setLineWidth(1);
    setMidLineWidth(2);

    contextMenu = NULL;
    m_mouseIn = false;
    m_updating = true;
}

DataWidget::~DataWidget()
{
    // Remove highlight from top data widget
    if(m_mouseIn)
        emit mouseStatus(false, m_info);

    WorkTab::DeleteAllMembers(layout);
    delete layout;
}

void DataWidget::setId(quint32 id)
{
    m_id = id;
    m_closeLabel->setId(id);
}

void DataWidget::setUp(AnalyzerDataStorage */*storage*/)
{
    setAcceptDrops(true);
    m_assigned = false;
    m_locked = false;
    contextMenu = new QMenu(this);
    m_mouseIn = false;

    m_lockAction = new QAction(tr("Lock"), this);
    m_lockAction->setCheckable(true);
    connect(m_lockAction, SIGNAL(triggered()), this, SLOT(lockTriggered()));
    contextMenu->addAction(m_lockAction);

    QAction *setTitleAct = new QAction(tr("Set title"), this);
    connect(setTitleAct, SIGNAL(triggered()), this, SLOT(setTitleTriggered()));
    contextMenu->addAction(setTitleAct);

    contextMenu->addSeparator();
    setContextMenuPolicy(Qt::DefaultContextMenu);

    setMouseTracking(true);

    connect(m_closeLabel, SIGNAL(removeWidget(quint32)), this, SIGNAL(removeWidget(quint32)));
}

void DataWidget::setTitle(const QString &title)
{
    m_title_label->setText(title);
}

QString DataWidget::getTitle()
{
    return m_title_label->text();
}

void DataWidget::setIcon(QString path)
{
    QIcon icon(path);
    m_icon_widget->setPixmap(icon.pixmap(16));
}

void DataWidget::contextMenuEvent ( QContextMenuEvent * event )
{
    contextMenu->exec(event->globalPos());
}

void DataWidget::mousePressEvent( QMouseEvent* e )
{
    m_dragAction = getDragAction(e->pos());
    mOrigin = e->globalPos();
}

void DataWidget::mouseMoveEvent( QMouseEvent* e )
{
    if(m_locked)
        return;

    if(e->buttons() == Qt::LeftButton) //dragging
    {
        if(m_dragAction == DRAG_MOVE)
            dragMove(e);
        else
            dragResize(e);
    }
}

void DataWidget::enterEvent(QEvent *)
{
    if(m_assigned)
        emit mouseStatus(true, m_info);
    m_mouseIn = true;
}

void DataWidget::leaveEvent(QEvent *)
{
    if(m_assigned)
        emit mouseStatus(false, m_info);
    m_mouseIn = false;
}

void DataWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if(m_locked || event->mimeData()->text().at(0) == 'w')
    {
        QFrame::dragEnterEvent(event);
        return;
    }
    event->acceptProposedAction();
}

void DataWidget::dropEvent(QDropEvent *event)
{
    if(m_locked)
        return;
    event->acceptProposedAction();

    QStringList data = event->mimeData()->text().split(" ");
    qint32 pos = data[0].toInt();
    qint16 device = data[1].toInt();
    qint16 cmd = data[2].toInt();
    setInfo(device, cmd, pos);
    m_assigned = true;
    emit updateData();
}

quint8 DataWidget::getDragAction(const QPoint &clickPos)
{
    quint8 res = 0;
    int x = clickPos.x();
    int y = clickPos.y();

    if(x < RESIZE_BORDER)
        res |= DRAG_RES_LEFT;
    else if(x > width() - RESIZE_BORDER)
        res |= DRAG_RES_RIGHT;

    if(y > height() - RESIZE_BORDER)
        res |= DRAG_RES_BOTTOM;

    if(res == 0)
        res = DRAG_MOVE;
    return res;
}

void DataWidget::dragResize(QMouseEvent* e)
{
    int w = width();
    int h = height();
    int x = pos().x();
    int y = pos().y();
    int gx = x + e->pos().x();

    if(m_dragAction & DRAG_RES_LEFT)
    {
        w += x - gx;
        x = gx;
    }
    else if(m_dragAction & DRAG_RES_RIGHT)
        w = e->pos().x();

    if(m_dragAction & DRAG_RES_BOTTOM)
        h = e->pos().y();

    if(w < minimumWidth())
    {
        w = width();
        x = pos().x();
    }

    if(h < minimumHeight())
    {
        h = height();
        y = pos().y();
    }

    resize(w, h);
    move(x, y);
}

void DataWidget::dragMove(QMouseEvent *e)
{
    move(pos() + ( e->globalPos() - mOrigin ));
    mOrigin = e->globalPos();

    emit updateMarker(this);
}

void DataWidget::newData(analyzer_data *data, quint32 /*index*/)
{
    if(!m_updating || !m_assigned || m_info.pos >= (quint32)data->getData().length())
        return;

    quint8 id;
    quint8 cmd;

    if(m_info.command != -1 && (!data->getCmd(cmd) || cmd != m_info.command))
        return;

    if(m_info.device != -1 && (!data->getDeviceId(id) || id != m_info.device))
        return;

    processData(data);
}

void DataWidget::processData(analyzer_data */*data*/)
{

}

void DataWidget::lockTriggered()
{
    m_locked = !m_locked;
    m_lockAction->setChecked(m_locked);
    m_closeLabel->setLocked(m_locked);
}

void DataWidget::setTitleTriggered()
{
    QString title = QInputDialog::getText(this, tr("Set widget title"), tr("Enter title:"));
    if(title.length() == 0)
        return;
    setTitle(title);
}

void DataWidget::saveWidgetInfo(AnalyzerDataFile *file)
{
    char *p = NULL;

    // widget type
    file->writeBlockIdentifier("widgetType");
    p = (char*)&m_widgetType;
    file->write(p, sizeof(m_widgetType));

    // widget pos and size
    file->writeBlockIdentifier("widgetPosSize");
    int val[] = { pos().x(), pos().y(), width(), height() };
    file->write((char*)&val, sizeof(val));

    // data info
    file->writeBlockIdentifier("widgetDataInfo");
    p = (char*)&m_info.pos;
    file->write(p, sizeof(m_info));

    // locked
    file->writeBlockIdentifier("widgetLocked");
    p = (char*)&m_locked;
    file->write(p, sizeof(m_locked));

    // title
    file->writeBlockIdentifier("widgetTitle");
    QByteArray title = getTitle().toAscii();
    quint32 size = title.length();
    file->write((char*)&size, sizeof(quint32));
    file->write(title.data());
}

void DataWidget::loadWidgetInfo(AnalyzerDataFile *file)
{
    // data info
    char *p = NULL;
    if(file->seekToNextBlock("widgetDataInfo", BLOCK_WIDGET))
    {
        p = (char*)&m_info.pos;
        file->read(p, sizeof(m_info));

        m_assigned = true;
    }

    // Locked
    if(file->seekToNextBlock("widgetLocked", BLOCK_WIDGET))
    {
        p = (char*)&m_locked;
        file->read(p, sizeof(m_locked));
        m_lockAction->setChecked(m_locked);
        m_closeLabel->setLocked(m_locked);
    }

    // title
    if(file->seekToNextBlock("widgetTitle", BLOCK_WIDGET))
    {
        quint32 size = 0;
        file->read((char*)&size, sizeof(quint32));

        QString title(file->read(size));
        setTitle(title);
    }
}

DataWidgetAddBtn::DataWidgetAddBtn(QWidget *parent) : QPushButton(parent)
{
    setFlat(true);
    setStyleSheet("text-align: left");
}

DataWidgetAddBtn::~DataWidgetAddBtn()
{

}

void DataWidgetAddBtn::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        QMimeData *mimeData = new QMimeData;
        mimeData->setText("w" + QString::number(m_widgetType));

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setPixmap(getRender());
        drag->exec();
        event->accept();
    }
}

QPixmap DataWidgetAddBtn::getRender()
{
    DataWidget *w = AnalyzerDataArea::newWidget(m_widgetType, this);
    if(!w)
        return QPixmap();

    QPixmap map(w->size());
    w->render(&map);
    delete w;
    return map;
}

CloseLabel::CloseLabel(QWidget *parent) : QLabel(parent)
{
    setObjectName("closeLabel");
    setStyleSheet("border-bottom: 1px solid black");
    setAlignment(Qt::AlignVCenter);
    setLocked(false);
}

void CloseLabel::mousePressEvent(QMouseEvent *event)
{
    if (!m_locked && event->button() == Qt::LeftButton)
        emit removeWidget(m_id);
    else
        QLabel::mousePressEvent(event);
}

void CloseLabel::setLocked(bool locked)
{
    m_locked = locked;
    setText(locked ? tr(" [L] ") : " X ");
}

QVariant DataWidget::getNumFromPacket(analyzer_data *data, quint32 pos, quint8 type)
{
    QVariant res;

    try
    {
        switch(type)
        {
            case NUM_INT8:  res.setValue((int)data->getInt8(pos));  break;
            case NUM_INT16: res.setValue((int)data->getInt16(pos)); break;
            case NUM_INT32: res.setValue(data->getInt32(pos)); break;
            case NUM_INT64: res.setValue(data->getInt64(pos)); break;

            case NUM_UINT8:  res.setValue(data->getUInt8(pos)); break;
            case NUM_UINT16: res.setValue(data->getUInt16(pos)); break;
            case NUM_UINT32: res.setValue(data->getUInt32(pos)); break;
            case NUM_UINT64: res.setValue(data->getUInt64(pos)); break;

            case NUM_FLOAT:  res.setValue(data->getFloat(pos));  break;
            case NUM_DOUBLE: res.setValue(data->getDouble(pos)); break;
        }
    }
    catch(const char*) { }

    return res;
}
