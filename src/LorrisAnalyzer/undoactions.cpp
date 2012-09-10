/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "undoactions.h"
#include "widgetarea.h"
#include "DataWidgets/datawidget.h"
#include "../misc/datafileparser.h"

UndoAction::UndoAction(int type, quint32 id)
{
    m_type = type;
    m_id = id;
}

UndoAction::~UndoAction()
{
}

RestoreAction::RestoreAction(DataWidget *w) :
    UndoAction(UNDO_WIDGET_RESTORE, w->getId())
{
    DataFileParser p(&m_data, QIODevice::WriteOnly);
    w->saveWidgetInfo(&p);
    m_pos = w->pos();
}

UndoAction *RestoreAction::restore(WidgetArea *area)
{
    DataFileParser p(&m_data, QIODevice::ReadOnly);

    if(DataWidget *w = area->LoadOneWidget(&p))
    {
        w->move(m_pos);
        return new DeleteAction(w);
    }
    return NULL;
}

void RestoreAction::move(const QPoint &diff)
{
    m_pos += diff;
}

DeleteAction::DeleteAction(DataWidget *w) :
    UndoAction(UNDO_WIDGET_DELETE, w->getId())
{
}

UndoAction* DeleteAction::restore(WidgetArea *area)
{
    area->removeWidget(m_id);
    return NULL;
}

bool DeleteAction::valid(WidgetArea *area)
{
    return area->getWidget(m_id) != NULL;
}

MoveAction::MoveAction(DataWidget *w) :
    UndoAction(UNDO_WIDGET_MOVE, w->getId())
{
    m_pos = w->pos();
    m_size = w->size();
    m_scaledUp = w->isScaledUp();
}

UndoAction *MoveAction::restore(WidgetArea *area)
{
    DataWidget *w = area->getWidget(m_id);
    if(!w)
        return NULL;

    UndoAction *opposite = new MoveAction(w);

    w->setScaledUp(m_scaledUp);
    w->move(m_pos);
    w->resize(m_size);

    return opposite;
}

bool MoveAction::valid(WidgetArea *area)
{
    return area->getWidget(m_id) != NULL;
}

void MoveAction::move(const QPoint &diff)
{
    m_pos += diff;
}
