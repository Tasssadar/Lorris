/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include "undostack.h"
#include "widgetarea.h"
#include "../misc/utils.h"

#define MAX_ACTIONS 200

UndoStack::UndoStack(WidgetArea *area) :
    QObject(area)
{
}

UndoStack::~UndoStack()
{
    clear();
}

void UndoStack::clear()
{
    clearUndo();
    clearRedo();
}

void UndoStack::clearUndo()
{
    delete_vect(m_undoStack);
    emit undoAvailable(false);
}

void UndoStack::clearRedo()
{
    delete_vect(m_redoStack);
    emit redoAvailable(false);
}

void UndoStack::addAction(UndoAction *act)
{
    m_undoStack.push_back(act);

    while(m_undoStack.size() > MAX_ACTIONS)
    {
        delete m_undoStack.front();
        m_undoStack.erase(m_undoStack.begin());
    }

    emit undoAvailable(true);

    clearRedo();
}

void UndoStack::undo()
{
    if(m_undoStack.empty())
        return;

    UndoAction* act = m_undoStack.back();
    m_undoStack.pop_back();

    UndoAction *opposite = act->restore(widgetArea());
    if(opposite)
        m_redoStack.push_back(opposite);

    delete act;

    emit undoAvailable(!m_undoStack.empty());
    emit redoAvailable(!m_redoStack.empty());
}

void UndoStack::redo()
{
    if(m_redoStack.empty())
        return;

    UndoAction *act = m_redoStack.back();
    m_redoStack.pop_back();

    UndoAction *opposite = act->restore(widgetArea());
    if(opposite)
        m_undoStack.push_back(opposite);

    delete act;

    emit undoAvailable(!m_undoStack.empty());
    emit redoAvailable(!m_redoStack.empty());
}

void UndoStack::checkValid()
{
    for(std::vector<UndoAction*>::iterator itr = m_undoStack.begin(); itr != m_undoStack.end();)
    {
        if(!(*itr)->valid(widgetArea()))
        {
            delete *itr;
            itr = m_undoStack.erase(itr);
        }
        else
            ++itr;
    }

    for(std::vector<UndoAction*>::iterator itr = m_redoStack.begin(); itr != m_redoStack.end();)
    {
        if(!(*itr)->valid(widgetArea()))
        {
            delete *itr;
            itr = m_undoStack.erase(itr);
        }
        else
            ++itr;
    }

    emit undoAvailable(!m_undoStack.empty());
    emit redoAvailable(!m_redoStack.empty());
}

void UndoStack::areaMoved(const QPoint &diff)
{
    bool run = true;
    for(quint32 i = 0; run; ++i)
    {
        run = false;
        if(i < m_undoStack.size())
        {
            m_undoStack[i]->move(diff);
            run = true;
        }

        if(i < m_redoStack.size())
        {
            m_redoStack[i]->move(diff);
            run = true;
        }
    }
}
