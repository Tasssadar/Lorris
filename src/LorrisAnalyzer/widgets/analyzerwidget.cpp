#include <QDragEnterEvent>

#include "analyzerwidget.h"

AnalyzerWidget::AnalyzerWidget(QWidget *parent) : QMdiSubWindow(parent)
{
    setAcceptDrops(true);
    setAttribute(Qt::WA_DeleteOnClose);
}

AnalyzerWidget::~AnalyzerWidget()
{

}

void AnalyzerWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/plain"))
        event->acceptProposedAction();
}
