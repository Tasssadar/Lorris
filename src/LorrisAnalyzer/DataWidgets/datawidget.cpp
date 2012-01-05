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

DataWidget::DataWidget(QWidget *parent) :
    QFrame(parent)
{
    layout = new QVBoxLayout(this);
    QHBoxLayout *title_bar = new QHBoxLayout();

    QLabel *title = new QLabel(this);
    title->setObjectName("titleLabel");
    title->setStyleSheet("border-right: 1px solid black; border-bottom: 1px solid black");
    title->setAlignment(Qt::AlignVCenter);

    m_closeLabel = new CloseLabel(this);

    QFrame *sepV = new QFrame(this);
    sepV->setFrameStyle(QFrame::HLine | QFrame::Plain);
    sepV->setLineWidth(1);

    title_bar->addWidget(title, 1);
    title_bar->addWidget(m_closeLabel, 0);

    layout->setMargin(0);
    title_bar->setMargin(0);

    layout->addLayout(title_bar);
    layout->addWidget(sepV);

    setFrameStyle(QFrame::Box | QFrame::Plain);
    setLineWidth(1);
    setMidLineWidth(2);

    contextMenu = NULL;
}

DataWidget::~DataWidget()
{
    WorkTab::DeleteAllMembers(layout);
    delete layout;
}

void DataWidget::setUp()
{
    setAcceptDrops(true);
    m_assigned = false;
    m_locked = false;
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
}

void DataWidget::setTitle(QString title)
{
    QLabel *titleLabel = findChild<QLabel*>("titleLabel");
    titleLabel->setText(" " + title);
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

    if(!m_locked && (e->buttons() & Qt::LeftButton)) //dragging
    {
        if(m_dragAction == DRAG_MOVE)
            dragMove(e);
        else
            dragResize(e);
    }
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

    int parW = ((QWidget*)parent())->width();
    int parH = ((QWidget*)parent())->height();
    if(w < minimumWidth() || x < 0 || x + w > parW)
    {
        w = width();
        x = pos().x();
    }

    if(h < minimumHeight() || y < 0 || y + h > parH)
    {
        h = height();
        y = pos().y();
    }

    resize(w, h);
    move(x, y);
}

void DataWidget::dragMove(QMouseEvent *e)
{
    QPoint n = pos() + ( e->globalPos() - mOrigin );

    if(n.x() < 0 || iw(n.x()))
        n.setX(pos().x());

    if(n.y() < 0 || ih(n.y()))
        n.setY(pos().y());

    move(n);
    mOrigin = e->globalPos();
}

void DataWidget::newData(analyzer_data *data)
{
    if(!m_assigned || m_info.pos >= (quint32)data->getData().length())
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
    return QPixmap();
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
        ((AnalyzerDataArea*)parent()->parent())->removeWidget(((DataWidget*)parent())->getId());
    else
        QLabel::mousePressEvent(event);
}

void CloseLabel::setLocked(bool locked)
{
    m_locked = locked;
    setText(locked ? tr(" [L] ") : " X ");
}


