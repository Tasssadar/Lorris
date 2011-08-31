#include <QLabel>
#include <QDropEvent>

#include "textwidget.h"

TextWidget::TextWidget(QWidget *parent) : AnalyzerWidget(parent)
{
    QLabel *label = new QLabel("aaaaa", this);
    label->setObjectName("labelTextView");
    label->setAcceptDrops(true);
    this->setWidget(label);
}

void TextWidget::dropEvent(QDropEvent *event)
{
    QLabel *label = findChild<QLabel *>("labelTextView");
    label->setText(event->mimeData()->text());
}
