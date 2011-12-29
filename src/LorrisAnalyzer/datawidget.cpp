#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDrag>
#include <QMouseEvent>

#include "datawidget.h"
#include "widgets/analyzerwidget.h"
#include "WorkTab/WorkTab.h"

#include "common.h"
#include <stdio.h>

DataWidget::DataWidget(QWidget *parent) : QWidget(parent)
{
    layout = new QVBoxLayout(this);

    first_line = new QHBoxLayout();
    second_line = new QHBoxLayout();

    QSpacerItem *spacer = new QSpacerItem(0, 20, QSizePolicy::Expanding, QSizePolicy::Maximum);
    second_line->addSpacerItem(spacer);

    layout->addLayout(first_line, 0);
    layout->addLayout(second_line, 0);

    setObjectName("DataWidget");
    m_size = 0;
}

DataWidget::~DataWidget()
{
    WorkTab::DeleteAllMembers(layout);
    delete layout;
    layout = NULL;
}

void DataWidget::setSize(quint16 size)
{
    if(size > m_size)
    {
        for(;m_size != size; )
        {
            DataLabel *label = new DataLabel("    ", this);
            label->setAutoFillBackground(true);
            label->setObjectName(QString::number(++m_size));
            label->setStyleSheet( "background-color: #000000; color: #FFFFFF" );
            label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            first_line->addWidget(label);
        }
    }
    else
    {
        for(;m_size != size; --m_size)
        {
            DataLabel *label = this->findChild<DataLabel*>(QString(m_size));
            first_line->removeWidget(label);
            delete label;
        }
    }
}

void DataWidget::newData(QByteArray data)
{
    for(quint16 i = 1; i <= m_size; ++i)
    {
        DataLabel *label = this->findChild<DataLabel*>(QString::number(i));
        label->setText(Utils::hexToString(data[i-1], true));
    }
}

void DataWidget::connectLabel(AnalyzerWidget *widget, int id)
{
    DataLabel *label = findChild<DataLabel*>(QString::number(id));
    if(!label)
        return;
    label->disconnect(widget);

    connect(label, SIGNAL(textChanged(QString,int)), widget, SLOT(textChanged(QString,int)));
}

DataLabel::DataLabel(const QString & text, QWidget *parent) : QLabel(text, parent)
{
}

DataLabel::~DataLabel()
{
}

void DataLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        mimeData->setText(objectName());
        drag->setMimeData(mimeData);

        QPixmap pixmap(size());
        this->render(&pixmap);

        drag->setPixmap(pixmap);

        drag->exec();
        event->accept();
    }
}

void DataLabel::setText ( const QString & text)
{
    emit textChanged(text, objectName().toUInt());
    QLabel::setText(text);
}
