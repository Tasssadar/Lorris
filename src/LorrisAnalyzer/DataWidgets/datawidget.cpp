#include "datawidget.h"

DataWidget::DataWidget(QWidget *parent) :
    QMdiSubWindow(parent)
{
    setWindowFlags(Qt::WindowFlags(windowFlags() & ~(Qt::WindowMaximizeButtonHint) & ~(Qt::WindowMinimizeButtonHint)));
    adjustSize();
    setFixedSize(width(), height());
}

DataWidget::~DataWidget()
{

}
