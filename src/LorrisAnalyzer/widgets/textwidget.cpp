#include <QLabel>
#include <QDropEvent>
#include <QHBoxLayout>

#include "textwidget.h"
#include "LorrisAnalyzer/datawidget.h"

TextWidget::TextWidget(QWidget *parent) : AnalyzerWidget(parent)
{
    QLabel *label = new QLabel("0xFF", this);
    label->setObjectName("labelTextView");
    label->setAcceptDrops(true);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    font.setPointSize(12);
    label->setFont(font);

    setWidget(label);
}

TextWidget::~TextWidget()
{

}

void TextWidget::dropEvent(QDropEvent *event)
{
    emit connectLabel(this, event->mimeData()->text().toUInt());
}

void TextWidget::textChanged(QString text, int id)
{
    QLabel *label = findChild<QLabel*>("labelTextView");
    label->setText(text);
}
