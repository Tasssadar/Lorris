#include <QtGui/QScrollBar>

#include "tracewidget.h"
#include "ui_tracewidget.h"

TraceWidget::TraceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TraceWidget)
{
    ui->setupUi(this);
    ui->textEdit->document()->setMaximumBlockCount(100);

    connect(ui->sendButton, SIGNAL(clicked()), this, SLOT(procSendButtonClick()));
    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(procClearButtonClick()));
}

TraceWidget::~TraceWidget()
{
    delete ui;
}

void TraceWidget::setTitle(const QString &name)
{
    this->setWindowTitle(QString(tr("Tracing I / O device: %1")).arg(name));
}

void TraceWidget::printTrace(const QByteArray &data, bool directionRx)
{
    ui->textEdit->setTextColor((directionRx) ? Qt::darkBlue : Qt::darkGreen);
    ui->textEdit->insertPlainText(QString(data));

    QScrollBar *bar = ui->textEdit->verticalScrollBar();
    bar->setValue(bar->maximum());
}

void TraceWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void TraceWidget::procSendButtonClick()
{
    QByteArray data;
    data.append(ui->lineEdit->text());
    if (data.size() > 0) {
        this->printTrace(data, false);
        emit this->sendSerialData(data);
    }
}

void TraceWidget::procClearButtonClick()
{
    ui->textEdit->clear();
}
