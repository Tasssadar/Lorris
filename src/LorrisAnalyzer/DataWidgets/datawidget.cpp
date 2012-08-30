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
#include <QPropertyAnimation>

#include "datawidget.h"
#include "../../WorkTab/WorkTab.h"
#include "../widgetarea.h"
#include "../../misc/datafileparser.h"

DataWidget::DataWidget(QWidget *parent) :
    QFrame(parent), m_gestures(this)
{
    layout = new QVBoxLayout(this);
    QHBoxLayout *title_bar = new QHBoxLayout();

    m_icon_widget = new QLabel(this);
    m_icon_widget->setStyleSheet("padding: 2px");
    m_icon_widget->setMouseTracking(true);

    m_title_label = new QLabel(this);
    m_title_label->setObjectName("titleLabel");
    m_title_label->setStyleSheet("border-right: 1px solid black; border-bottom: 1px solid black");
    m_title_label->setAlignment(Qt::AlignVCenter);
    m_title_label->setMouseTracking(true);

    m_closeLabel = new CloseLabel(this);

    QFrame *sepV = new QFrame(this);
    sepV->setFrameStyle(QFrame::HLine | QFrame::Plain);
    sepV->setLineWidth(1);

    title_bar->addWidget(m_icon_widget);
    title_bar->addWidget(m_title_label, 1);
    title_bar->addWidget(m_closeLabel, 0);

    title_bar->setMargin(0);
    layout->setContentsMargins(5, 0, 5, 5);

    layout->addLayout(title_bar);
    layout->addWidget(sepV);

    setFrameStyle(QFrame::Box | QFrame::Plain);
    setLineWidth(1);
    setMidLineWidth(2);
    setAutoFillBackground(true);
    setFocusPolicy(Qt::StrongFocus);

    QPalette p = palette();
    p.setColor(QPalette::Window, QColor("#FFFFFF"));
    setPalette(p);

    contextMenu = NULL;
    m_dragAction = DRAG_NONE;
    m_copy_widget = NULL;
    m_state = STATE_UPDATING;

    m_widgetControlled = -1;

    setMinimumSize(20, 20);
}

DataWidget::~DataWidget()
{
    // Remove highlight from top data widget
    if(m_state & STATE_MOUSE_IN)
        emit mouseStatus(false, m_info, m_widgetControlled);

    Utils::deleteLayoutMembers(layout);
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
    contextMenu = new QMenu(this);

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

    connect(m_closeLabel, SIGNAL(removeWidget(quint32)),  SIGNAL(removeWidget(quint32)));
    connect(&m_gestures,  SIGNAL(gestureCompleted(int)),  SLOT(gestureCompleted(int)));
}

void DataWidget::setTitleVisibility(bool visible)
{
    m_closeLabel->setVisible(visible);
    m_title_label->setVisible(visible);
    m_icon_widget->setVisible(visible);
}

void DataWidget::setTitle(QString title)
{
    if(parent()->inherits("WidgetArea"))
        widgetArea()->correctWidgetName(title, this);

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

void DataWidget::childEvent(QChildEvent *event)
{
    QFrame::childEvent(event);

    if(event->child() == m_icon_widget || event->child() == m_title_label)
        return;

    if(event->type() == QEvent::ChildAdded)
        event->child()->installEventFilter(this);
    else if(event->type() == QEvent::ChildRemoved)
        event->child()->removeEventFilter(this);
}

bool DataWidget::eventFilter(QObject *, QEvent *ev)
{
    switch(ev->type())
    {
        case QEvent::Enter:
            setCursor(Qt::ArrowCursor);
            break;
        case QEvent::FocusIn:
            raise();
            break;
        default:
            break;
    }
    return false;
}

void DataWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton && m_title_label->rect().contains(e->pos()))
        titleDoubleClick();
    else
        QFrame::mouseDoubleClickEvent(e);
}

void DataWidget::mousePressEvent( QMouseEvent* e )
{
    m_dragAction = getDragAction(e);
    mOrigin = e->globalPos();
    m_clickPos = e->pos();
}

void DataWidget::mouseMoveEvent( QMouseEvent* e )
{
    switch(e->buttons())
    {
        case Qt::NoButton:
        {
            quint8 act = getDragAction(e);
            if(isLocked() && act != DRAG_NONE && act != (DRAG_MOVE | DRAG_COPY))
                return;

            setCursor(DataWidget::getCursor(act));
            break;
        }
        case Qt::LeftButton:
        {
            switch(m_dragAction)
            {
                case DRAG_NONE: break;
                case DRAG_MOVE:
                    if(!isLocked() && !isMoveBlocked())
                    {
                        dragMove(e, this);
                        m_gestures.moveEvent(e->globalPos());
                    }
                    break;
                case (DRAG_MOVE | DRAG_COPY):
                    if(m_copy_widget) dragMove(e, m_copy_widget);
                    else              copyWidget(e);
                    break;
                default:
                    if(!isLocked())
                        dragResize(e);
                    break;
            }
            break;
        }
        default:
            break;
    }
}

void DataWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    emit clearPlacementLines();
    m_copy_widget = NULL;
    m_dragAction = DRAG_NONE;
    m_gestures.clear();

    m_state &= ~(STATE_BLOCK_MOVE);

    QWidget::mouseReleaseEvent(ev);
}

void DataWidget::enterEvent(QEvent *)
{
    if(isAssigned() || m_widgetControlled != -1)
        emit mouseStatus(true, m_info, m_widgetControlled);
    m_state |= STATE_MOUSE_IN;
}

void DataWidget::leaveEvent(QEvent *)
{
    if(isAssigned() || m_widgetControlled != -1)
        emit mouseStatus(false, m_info, m_widgetControlled);
    m_state &= ~(STATE_MOUSE_IN);
}

void DataWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if(isLocked() || !event->source() || !event->mimeData()->hasText() || event->mimeData()->text().at(0) == 'w')
    {
        QFrame::dragEnterEvent(event);
        return;
    }
    event->acceptProposedAction();
}

void DataWidget::dropEvent(QDropEvent *event)
{
    if(isLocked())
        return;

    event->acceptProposedAction();

    QStringList data = event->mimeData()->text().split(" ");
    qint32 pos = data[0].toInt();
    qint16 device = data[1].toInt();
    qint16 cmd = data[2].toInt();
    setInfo(device, cmd, pos);

    m_state |= STATE_ASSIGNED;

    emit updateForMe();
}

quint8 DataWidget::getDragAction(QMouseEvent *ev)
{
    quint8 res = 0;
    int x = ev->pos().x();
    int y = ev->pos().y();

    if(x < RESIZE_BORDER)
        res |= DRAG_RES_LEFT;
    else if(x > width() - RESIZE_BORDER)
        res |= DRAG_RES_RIGHT;

    if(y > height() - RESIZE_BORDER)
        res |= DRAG_RES_BOTTOM;
    else if(y < RESIZE_BORDER)
        res |= DRAG_RES_TOP;

    if(res == 0)
    {
        res = DRAG_MOVE;
        if(ev->modifiers() & Qt::ControlModifier)
            res |= DRAG_COPY;
    }
    return res;
}

void DataWidget::dragResize(QMouseEvent* e)
{
    int w = width();
    int h = height();
    int x = pos().x();
    int y = pos().y();

    QPoint p = e->pos();
    mapXYToGrid(p);
    p -= widgetArea()->getGridOffset();

    if(m_dragAction & DRAG_RES_LEFT)
    {
        int gx = x + e->pos().x();
        mapToGrid(gx);

        w += x - gx;
        x = gx;
    }
    else if(m_dragAction & DRAG_RES_RIGHT)
        w = p.x();

    if(m_dragAction & DRAG_RES_TOP)
    {
        int gy = y + e->pos().y();
        mapToGrid(gy);

        h += y - gy;
        y = gy;
    }
    else if(m_dragAction & DRAG_RES_BOTTOM)
        h = p.y();

    if(w < minimumWidth())
        x = pos().x();

    if(h < minimumHeight())
        y = pos().y();

    w = (std::max)(w, minimumWidth());
    h = (std::max)(h, minimumHeight());

    // stick to placement lines
    widgetArea()->updatePlacement(x, y, w, h, this);

    int lx, ly;
    QPoint mouse = mapToParent(e->pos() - m_clickPos);
    int wx = mouse.x() + width();
    int wy = mouse.y() + height();
    const QVector<QLine>& lines = widgetArea()->getPlacementLines();
    for(QVector<QLine>::const_iterator itr = lines.begin(); itr != lines.end(); ++itr)
    {
        lx = (*itr).x1();
        ly = (*itr).y1();

        if((m_dragAction & DRAG_RES_RIGHT) && abs(lx - wx) < PLACEMENT_STICK)
                w = lx - x;
        else if((m_dragAction & DRAG_RES_LEFT) && abs(lx - mouse.x()) < PLACEMENT_STICK)
        {
            w += x - lx;
            x = lx;
        }

        if((m_dragAction & DRAG_RES_BOTTOM) && abs(ly - wy) < PLACEMENT_STICK)
            h = ly - y;
        else if((m_dragAction & DRAG_RES_TOP) && abs(ly - mouse.y()) < PLACEMENT_STICK)
        {
            h += y -ly;
            y = ly;
        }
    }

    m_clickPos += (QPoint(w, h) - QPoint(width(), height())) - (pos() - QPoint(x, y));

    resize(w, h);
    move(x, y);
}

void DataWidget::dragMove(QMouseEvent *e, DataWidget *widget)
{
    if(widget == this && (m_state & STATE_SCALED_UP))
    {
        m_state &= ~(STATE_SCALED_UP);

        QPoint pos = mapToParent(e->pos());
        int pctW = (e->pos().x()*100)/width();
        int pctH = (e->pos().y()*100)/height();

        m_orig_geometry.moveLeft(pos.x() - (m_orig_geometry.width()*pctW)/100);
        m_orig_geometry.moveTop(pos.y() - (m_orig_geometry.height()*pctH)/100);

        startAnimation(m_orig_geometry);
        return;
    }

    QPoint p = widget->pos() + ( e->globalPos() - mOrigin );
    mapXYToGrid(p);
    mOrigin = e->globalPos();
    mapXYToGrid(mOrigin);

    // stick to placement lines
    widgetArea()->updatePlacement(p.x(), p.y(),
                                  widget->width(), widget->height(), widget);


    QPoint mouse = mapToParent(e->pos() - m_clickPos);
    int lx, ly;
    int wx = mouse.x() + widget->width();
    int wy = mouse.y() + widget->height();
    const QVector<QLine>& lines = widgetArea()->getPlacementLines();
    for(QVector<QLine>::const_iterator itr = lines.begin(); itr != lines.end(); ++itr)
    {
        lx = (*itr).x1();
        ly = (*itr).y1();

        if(abs(lx - mouse.x()) < PLACEMENT_STICK)
            p.rx() = lx;
        else if(abs(lx - wx) < PLACEMENT_STICK)
            p.rx() = lx - widget->width();

        if(abs(ly - mouse.y()) < PLACEMENT_STICK)
            p.ry() = ly;
        else if(abs(ly - wy) < PLACEMENT_STICK)
            p.ry() = ly - widget->height();
    }

    widget->move(p);

    emit updateMarker(widget);
}

void DataWidget::newData(analyzer_data *data, quint32 /*index*/)
{
    if(!isUpdating() || !isAssigned() || m_info.pos >= (quint32)data->getData().length())
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
    m_state ^= STATE_LOCKED;
    m_lockAction->setChecked(isLocked());
    m_closeLabel->setLocked(isLocked());
}

void DataWidget::setLocked(bool locked)
{
    if(isLocked() == locked)
        return;
    lockTriggered();
}

void DataWidget::setTitleTriggered()
{
    QString title = QInputDialog::getText(this, tr("Set widget title"), tr("Enter title:"),
                                          QLineEdit::Normal, getTitle());
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
    file->writeVal(isLocked());

    // title
    file->writeBlockIdentifier("widgetTitleUtf8");
    file->writeString(getTitle());

    // scaled up
    file->writeBlockIdentifier("widgetScaledUp");
    file->writeVal(bool(m_state & STATE_SCALED_UP));
    file->writeVal(m_orig_geometry.x());
    file->writeVal(m_orig_geometry.y());
    file->writeVal(m_orig_geometry.width());
    file->writeVal(m_orig_geometry.height());
}

void DataWidget::loadWidgetInfo(DataFileParser *file)
{
    // data info
    char *p = NULL;
    if(file->seekToNextBlock("widgetDataInfo", BLOCK_WIDGET))
    {
        p = (char*)&m_info.pos;
        file->read(p, sizeof(m_info));

        m_state |= STATE_ASSIGNED;
    }

    // Locked
    if(file->seekToNextBlock("widgetLocked", BLOCK_WIDGET))
    {
        bool locked = file->readVal<bool>();

        if(locked)
            m_state |= STATE_LOCKED;
        else
            m_state &= ~(STATE_LOCKED);

        m_lockAction->setChecked(locked);
        m_closeLabel->setLocked(locked);
    }

    // title
    if(file->seekToNextBlock("widgetTitle", BLOCK_WIDGET))
    {
        quint32 size = 0;
        file->read((char*)&size, sizeof(quint32));

        QString title(file->read(size));
        setTitle(title);
    }
    else if(file->seekToNextBlock("widgetTitleUtf8", BLOCK_WIDGET))
    {
        setTitle(file->readString());
    }

    // scaled up
    if(file->seekToNextBlock("widgetScaledUp", BLOCK_WIDGET))
    {
        if(file->readVal<bool>())
            m_state |= STATE_SCALED_UP;

        int x = file->readVal<int>();
        int y = file->readVal<int>();
        int w = file->readVal<int>();
        int h = file->readVal<int>();
        m_orig_geometry = QRect(x, y, w, h);
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
    point += widgetArea()->getGridOffset();
}

void DataWidget::mapXYToGrid(int& x, int& y)
{
    mapToGrid(x);
    mapToGrid(y);

    const QPoint& offset = widgetArea()->getGridOffset();
    x += offset.x();
    y += offset.y();
}

void DataWidget::mapToGrid(int &val)
{
    int grid = widgetArea()->getGrid();
    int div = abs(val%grid);
    val += div >= grid/2 ? grid - div : -div;
}

void DataWidget::align()
{
    QPoint p(pos());
    p -= widgetArea()->getGridOffset();
    mapXYToGrid(p);
    move(p);

    p = QPoint(width(), height());
    mapXYToGrid(p);
    p -= widgetArea()->getGridOffset();
    resize(p.x(), p.y());
}

void DataWidget::copyWidget(QMouseEvent *ev)
{
    Q_ASSERT(!m_copy_widget);

    m_copy_widget = sWidgetFactory.copy(this);
    m_copy_widget->move(mapToParent(ev->pos()));
}

void DataWidget::titleDoubleClick()
{
    if(isLocked())
        return;
    setTitleTriggered();
}

void DataWidget::focusInEvent(QFocusEvent *event)
{
    QFrame::focusInEvent(event);
    this->raise();
}

Qt::CursorShape DataWidget::getCursor(quint8 act)
{
    switch(act)
    {
        case DRAG_NONE:
        default:
            return Qt::ArrowCursor;

        case DRAG_MOVE:
            return Qt::SizeAllCursor;

        case DRAG_RES_LEFT:
        case DRAG_RES_RIGHT:
            return Qt::SizeHorCursor;

        case DRAG_RES_TOP:
        case DRAG_RES_BOTTOM:
            return Qt::SizeVerCursor;

        case (DRAG_RES_LEFT | DRAG_RES_BOTTOM):
        case (DRAG_RES_RIGHT | DRAG_RES_TOP):
            return Qt::SizeBDiagCursor;

        case (DRAG_RES_RIGHT | DRAG_RES_BOTTOM):
        case (DRAG_RES_LEFT | DRAG_RES_TOP):
            return Qt::SizeFDiagCursor;

        case (DRAG_MOVE | DRAG_COPY):
            return Qt::UpArrowCursor;
    }
}

void DataWidget::gestureCompleted(int gesture)
{
    if(gesture != GESTURE_SHAKE_LEFT && gesture != GESTURE_SHAKE_RIGHT)
        return;

    m_state |= (STATE_BLOCK_MOVE | STATE_SCALED_UP);
    m_orig_geometry = geometry();

    startAnimation(widgetArea()->rect());
}

void DataWidget::startAnimation(const QRect &target)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry", this);
    animation->setDuration(100);
    animation->setStartValue(geometry());
    animation->setEndValue(target);
    connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));

    animation->start();
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
        DataWidget *w = sWidgetFactory.getWidget(m_widgetType, this);
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
