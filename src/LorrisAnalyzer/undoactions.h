/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef UNDOACTIONS_H
#define UNDOACTIONS_H

#include <QByteArray>
#include <QPoint>
#include <QSize>

class WidgetArea;
class DataWidget;

enum UndoActType
{
    UNDO_WIDGET_DELETE = 0,
    UNDO_WIDGET_RESTORE,
    UNDO_WIDGET_MOVE,

    UNDO_MAX
};

class UndoAction
{
public:
    virtual ~UndoAction();

    int getType() const { return m_type; }

    virtual UndoAction* restore(WidgetArea *area) = 0;
    virtual bool valid(WidgetArea *) { return true; }
    virtual void move(const QPoint& /*diff*/) { }

protected:
    UndoAction(int type, quint32 id);

    quint32 m_id;

private:
    int m_type;
};

class RestoreAction : public UndoAction
{
public:
    RestoreAction(DataWidget *w);

    UndoAction* restore(WidgetArea *area);
    void move(const QPoint& diff);

private:
    QPoint m_pos;
    QByteArray m_data;
};

class DeleteAction : public UndoAction
{
public:
    DeleteAction(DataWidget *w);

    UndoAction *restore(WidgetArea *area);
    bool valid(WidgetArea *area);
};

class MoveAction : public UndoAction
{
public:
    MoveAction(DataWidget *w);

    UndoAction *restore(WidgetArea *area);
    bool valid(WidgetArea *area);
    void move(const QPoint& diff);

private:
    QPoint m_pos;
    QSize m_size;
};

#endif // UNDOACTIONS_H
