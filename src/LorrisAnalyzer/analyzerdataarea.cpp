#include <QDropEvent>

#include "analyzerdataarea.h"
#include "DataWidgets/numberwidget.h"
#include "lorrisanalyzer.h"

AnalyzerDataArea::AnalyzerDataArea(QWidget *parent) :
    QFrame(parent)
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setAcceptDrops(true);
    m_widgetIdCounter = 0;
}

AnalyzerDataArea::~AnalyzerDataArea()
{
    for(w_map::iterator itr = m_widgets.begin(); itr != m_widgets.end(); ++itr)
        delete itr->second;
    m_widgets.clear();
}

void AnalyzerDataArea::dropEvent(QDropEvent *event)
{
    QString data = event->mimeData()->text().remove(0, 1);
    quint8 type = data.toInt();
    DataWidget *w = newWidget(type);
    if(!w)
        return;
    w->setUp();


    QPoint newPos(event->pos());
    fixWidgetPos(newPos, w);
    w->move(newPos);
    w->show();

    quint32 id = getNewId();
    w->setId(id);
    m_widgets.insert(std::make_pair<quint32,DataWidget*>(id, w));

    connect(((LorrisAnalyzer*)parent()), SIGNAL(newData(analyzer_data*)), w, SLOT(newData(analyzer_data*)));
    connect(w, SIGNAL(updateData()), this, SIGNAL(updateData()));
    event->acceptProposedAction();
}

void AnalyzerDataArea::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->text().at(0) == 'w')
        event->acceptProposedAction();
    else
        QFrame::dragEnterEvent(event);
}

DataWidget *AnalyzerDataArea::newWidget(quint8 type)
{
    switch(type)
    {
        case WIDGET_NUMBERS: return new NumberWidget(this);
    }
    return NULL;
}

void AnalyzerDataArea::removeWidget(quint32 id)
{
    w_map::iterator itr = m_widgets.find(id);
    if(itr == m_widgets.end())
        return;
    delete itr->second;
    m_widgets.erase(itr);
}

void AnalyzerDataArea::fixWidgetPos(QPoint &pos, QWidget *w)
{
    int x = pos.x() + w->width();
    int y = pos.y() + w->height();

    if(x > width())
        pos.setX(width() - w->width());

    if(y > height())
        pos.setY(height() - w->height());
}
