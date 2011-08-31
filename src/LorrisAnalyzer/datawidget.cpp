#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDrag>
#include <QMouseEvent>

#include "datawidget.h"
#include "WorkTab/WorkTab.h"

DataWidget::DataWidget(QWidget *parent) : QWidget(parent)
{
    layout = new QVBoxLayout(this);

    QHBoxLayout *first_line = new QHBoxLayout();
    QHBoxLayout *second_line = new QHBoxLayout();

    DataLabel *label = new DataLabel("0x04", this);
    label->setAutoFillBackground(true);
    label->setStyleSheet( "background-color: #000000; color: #FFFFFF" );

    DataLabel *label2 = new DataLabel("0xFF", this);
    label2->setStyleSheet( "background-color: yellow; color: black" );
    label2->setAutoFillBackground(true);

    DataLabel *label3 = new DataLabel("0x04", this);
    label3->setAutoFillBackground(true);
    label3->setStyleSheet( "background-color: blue; color: #FFFFFF" );

    DataLabel *label4 = new DataLabel("0xFF", this);
    label4->setStyleSheet( "background-color: purple; color: white" );
    label4->setAutoFillBackground(true);

    first_line->addWidget(label);
    first_line->addWidget(label2);
    second_line->addWidget(label3);
    second_line->addWidget(label4);
    layout->addLayout(first_line);
    layout->addLayout(second_line);
}

DataWidget::~DataWidget()
{
    WorkTab::DeleteAllMembers(layout);
    delete layout;
    layout = NULL;
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
        mimeData->setText(text());
        drag->setMimeData(mimeData);

        QPixmap pixmap(size());
        this->render(&pixmap);

        drag->setPixmap(pixmap);

        drag->exec();
        event->accept();
    }
}
