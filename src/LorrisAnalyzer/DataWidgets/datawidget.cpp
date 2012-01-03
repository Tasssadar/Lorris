#include <QLabel>
#include <QMouseEvent>
#include <QPoint>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QDrag>

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

    CloseLabel *close = new CloseLabel(this);

    QFrame *sepV = new QFrame(this);
    sepV->setFrameStyle(QFrame::HLine | QFrame::Plain);
    sepV->setLineWidth(1);

    title_bar->addWidget(title, 1);
    title_bar->addWidget(close, 0);

    layout->setMargin(0);
    title_bar->setMargin(0);

    layout->addLayout(title_bar);
    layout->addWidget(sepV);

    setFrameStyle(QFrame::Box | QFrame::Plain);
    setLineWidth(2);

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
}

void DataWidget::setTitle(QString title)
{
    QLabel *titleLabel = findChild<QLabel*>("titleLabel");
    titleLabel->setText(title);
}

void DataWidget::mousePressEvent( QMouseEvent* e )
{
    mOrigin = e->globalPos();  //mOrigin is a QPoint member
}

void DataWidget::mouseMoveEvent( QMouseEvent* e )
{
    if( e->buttons() & Qt::LeftButton ) //dragging
    {
        QPoint n =  pos() + ( e->globalPos() - mOrigin );
        if(n.x() < 0 || n.y() < 0 || iw(n.x()) || ih(n.y()))
            return;
        move(n);
        mOrigin = e->globalPos();
    }
}

void DataWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->text().at(0) == 'w')
    {
        QFrame::dragEnterEvent(event);
        return;
    }
    event->acceptProposedAction();
}

void DataWidget::dropEvent(QDropEvent *event)
{
    event->acceptProposedAction();

    QStringList data = event->mimeData()->text().split(" ");
    qint32 pos = data[0].toInt();
    qint16 device = data[1].toInt();
    qint16 cmd = data[2].toInt();
    setInfo(device, cmd, pos);
    m_assigned = true;
    emit updateData();
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

CloseLabel::CloseLabel(QWidget *parent) : QLabel(" X ", parent)
{
    setObjectName("closeLabel");
    setStyleSheet("border-bottom: 1px solid black");
    setAlignment(Qt::AlignVCenter);
}

void CloseLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        ((AnalyzerDataArea*)parent()->parent())->removeWidget(((DataWidget*)parent())->getId());
    else
        QLabel::mousePressEvent(event);
}


