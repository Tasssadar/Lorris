/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef UNDOSTACK_H
#define UNDOSTACK_H

#include <QObject>
#include <vector>
#include <QPoint>

#include "undoactions.h"

class UndoStack : public QObject
{
    Q_OBJECT
Q_SIGNALS:
    void undoAvailable(bool available);
    void redoAvailable(bool available);

public:
    explicit UndoStack(WidgetArea *area);
    ~UndoStack();
    
public slots:
    void addAction(UndoAction *act);
    void clear();

    void undo();
    void redo();
    void checkValid();
    void areaMoved(const QPoint& diff);

private:
    void clearUndo();
    void clearRedo();

    inline WidgetArea *widgetArea()
    {
        return (WidgetArea*)parent();
    }

    std::vector<UndoAction*> m_undoStack;
    std::vector<UndoAction*> m_redoStack;
};

#endif // UNDOSTACK_H
