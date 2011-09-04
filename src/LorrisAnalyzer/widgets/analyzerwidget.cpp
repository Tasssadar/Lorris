#include <QDragEnterEvent>
#include <QHBoxLayout>

#include "analyzerwidget.h"
#include "WorkTab/WorkTab.h"

AnalyzerWidget::AnalyzerWidget(QWidget *parent) : QMdiSubWindow(parent, Qt::CustomizeWindowHint)
{
    setAcceptDrops(true);
    setAttribute(Qt::WA_DeleteOnClose);
    layout = NULL;
}

AnalyzerWidget::~AnalyzerWidget()
{
    if(layout)
        WorkTab::DeleteAllMembers(layout);
}

void AnalyzerWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/plain"))
        event->acceptProposedAction();
}

void AnalyzerWidget::textChanged(QString text, int id)
{

}
