/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QLabel>
#include <QMouseEvent>
#include <QPoint>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QDrag>
#include <QMenu>
#include <QInputDialog>

#include "datawidget.h"
#include "../../WorkTab/WorkTab.h"
#include "../widgetarea.h"
#include "../../misc/datafileparser.h"

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
    setAutoFillBackground(true);

    QPalette p = palette();
    p.setColor(QPalette::Window, QColor("#F5F5F5"));
    setPalette(p);

    contextMenu = NULL;
    m_mouseIn = false;
    m_updating = true;
    m_dragAction = DRAG_NONE;

    m_widgetControlled = -1;

    setMinimumSize(20, 20);
}

DataWidget::~DataWidget()
{
    // Remove highlight from top data widget
    if(m_mouseIn)
        emit mouseStatus(false, m_info, m_widgetControlled);

    WorkTab::DeleteAllMembers(layout);
    delete layout;
}

void DataWidget::setId(quint32 id)
{
    m_id = id;
    m_closeLabel->setId(id);
}

void DataWidget::setUp(Storage */*storage*/)
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

void DataWidget::setTitleVisibility(bool visible)
{
    m_closeLabel->setVisible(visible);
    m_title_label->setVisible(visible);
    m_icon_widget->setVisible(visible);
}

void DataWidget::setTitle(const QString &title)
{
    emit titleChanged(title);

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

    switch(e->buttons())
    {
        case Qt::NoButton:
        {
            switch(getDragAction(e->pos()))
            {
                case DRAG_NONE:
                    setCursor(Qt::ArrowCursor);
                    break;
                case DRAG_MOVE:
                    setCursor(Qt::SizeAllCursor);
                    break;
                case DRAG_RES_LEFT:
                case DRAG_RES_RIGHT:
                    setCursor(Qt::SizeHorCursor);
                    break;
                case DRAG_RES_BOTTOM:
                    setCursor(Qt::SizeVerCursor);
                    break;
                case (DRAG_RES_LEFT | DRAG_RES_BOTTOM):
                    setCursor(Qt::SizeBDiagCursor);
                    break;
                case (DRAG_RES_RIGHT | DRAG_RES_BOTTOM):
                    setCursor(Qt::SizeFDiagCursor);
                    break;
            }
            break;
        }
        case Qt::LeftButton:
        {
            switch(m_dragAction)
            {
                case DRAG_NONE: break;
                case DRAG_MOVE: dragMove(e); break;
                default: dragResize(e); break;
            }
            break;
        }
        default:
            break;
    }
}

void DataWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    m_dragAction = DRAG_NONE;
    QWidget::mouseReleaseEvent(ev);
}

void DataWidget::enterEvent(QEvent *)
{
    if(m_assigned || m_widgetControlled != -1)
        emit mouseStatus(true, m_info, m_widgetControlled);
    m_mouseIn = true;
}

void DataWidget::leaveEvent(QEvent *)
{
    if(m_assigned || m_widgetControlled != -1)
        emit mouseStatus(false, m_info, m_widgetControlled);
    m_mouseIn = false;
}

void DataWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if(m_locked || !event->source() || event->mimeData()->text().at(0) == 'w')
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
    emit updateForMe();
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

    QPoint m_pos = e->pos();
    mapXYToGrid(m_pos);
    int gx = x + m_pos.x();

    if(m_dragAction & DRAG_RES_LEFT)
    {
        w += x - gx;
        x = gx;
    }
    else if(m_dragAction & DRAG_RES_RIGHT)
        w = m_pos.x();

    if(m_dragAction & DRAG_RES_BOTTOM)
        h = m_pos.y();

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
    QPoint p = pos() + ( e->globalPos() - mOrigin );
    mapXYToGrid(p);
    move(p);

    mOrigin = e->globalPos();
    mapXYToGrid(mOrigin);

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

void DataWidget::saveWidgetInfo(DataFileParser *file)
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

void DataWidget::loadWidgetInfo(DataFileParser *file)
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

void DataWidget::setWidgetControlled(qint32 widget)
{
    m_widgetControlled = widget;

    m_closeLabel->setScript(widget != -1);
}

void DataWidget::remove()
{
    emit removeWidget(m_id);
}

void DataWidget::onWidgetAdd(DataWidget */*w*/)
{

}

void DataWidget::onWidgetRemove(DataWidget */*w*/)
{

}

void DataWidget::onScriptEvent(const QString& /*eventId*/)
{

}

void DataWidget::mapXYToGrid(QPoint& point)
{
    mapToGrid(point.rx());
    mapToGrid(point.ry());
    point += ((WidgetArea*)parent())->getGridOffset();
}

void DataWidget::mapXYToGrid(int& x, int& y)
{
    mapToGrid(x);
    mapToGrid(y);

    const QPoint& offset = ((WidgetArea*)parent())->getGridOffset();
    x += offset.x();
    y += offset.y();
}

void DataWidget::mapToGrid(int &val)
{
    int grid = ((WidgetArea*)parent())->getGrid();
    int div = abs(val%grid);
    val += div >= grid/2 ? grid - div : -div;
}

void DataWidget::align()
{
    QPoint p(pos());
    p -= ((WidgetArea*)parent())->getGridOffset();
    mapXYToGrid(p);
    move(p);

    p = QPoint(width(), height());
    mapXYToGrid(p);
    p -= ((WidgetArea*)parent())->getGridOffset();
    resize(p.x(), p.y());
}

DataWidgetAddBtn::DataWidgetAddBtn(QWidget *parent) : QPushButton(parent)
{
    setFlat(true);
    setStyleSheet("text-align: left");
    m_pixmap = NULL;
}

DataWidgetAddBtn::~DataWidgetAddBtn()
{
    delete m_pixmap;
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

const QPixmap& DataWidgetAddBtn::getRender()
{
    if(!m_pixmap)
    {
        DataWidget *w = WidgetArea::newWidget(m_widgetType, this);
        if(w)
        {
            m_pixmap = new QPixmap(w->size());
            w->render(m_pixmap);
            delete w;
        }
        else
            m_pixmap = new QPixmap();
    }
    return *m_pixmap;
}

CloseLabel::CloseLabel(QWidget *parent) : QLabel(parent)
{
    m_state = CLOSE_NONE;

    setObjectName("closeLabel");
    setStyleSheet("border-bottom: 1px solid black");
    setAlignment(Qt::AlignVCenter);
    setLocked(false);

    setCursor(Qt::ArrowCursor);
}

void CloseLabel::mousePressEvent(QMouseEvent *event)
{
    if (m_state == CLOSE_NONE && event->button() == Qt::LeftButton)
        emit removeWidget(m_id);
    else
        QLabel::mousePressEvent(event);
}

void CloseLabel::setLocked(bool locked)
{
    if(locked)
        m_state |= CLOSE_LOCKED;
    else
        m_state &= ~(CLOSE_LOCKED);

    setText(getTextByState());
}

void CloseLabel::setScript(bool script)
{
    if(script)
        m_state |= CLOSE_SCRIPT;
    else
        m_state &= ~(CLOSE_SCRIPT);

    setText(getTextByState());
}

QString CloseLabel::getTextByState()
{
    if     (m_state & CLOSE_LOCKED)  return tr(" [L] ");
    else if(m_state & CLOSE_SCRIPT)  return tr(" [S] ");
    else                             return " X ";
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
