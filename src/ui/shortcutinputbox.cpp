/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QToolButton>
#include <QKeyEvent>
#include <QStyle>

#include "shortcutinputbox.h"

ShortcutInputBox::ShortcutInputBox(QWidget *parent) : ResettableLineEdit(parent)
{
    setPlaceholderText(tr("Press keys..."));
    setToolTip(tr("Press keys..."));
}

ShortcutInputBox::ShortcutInputBox(const QKeySequence &seq, QWidget *parent) : ResettableLineEdit(parent)
{
    setPlaceholderText(tr("Press keys..."));
    setToolTip(tr("Press keys..."));
    setKeySequence(seq);
}

void ShortcutInputBox::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        return QLineEdit::keyPressEvent(event);

    // FIXME: we don't want this, do we?
    //if(event->text().isEmpty())
    //    return;

    switch(event->key())
    {
        case 0:
        case Qt::Key_unknown:
        case Qt::Key_Control: // modifier keycodes are not
        case Qt::Key_Shift:   // handled by QKeySequence::toString()
        case Qt::Key_Alt:
        case Qt::Key_AltGr:
        case Qt::Key_Meta:
            m_sequence = QKeySequence(event->modifiers());
            break;
        default:
            m_sequence = QKeySequence(event->key() | event->modifiers());
            break;
    }

    setText(m_sequence.toString(QKeySequence::NativeText));
}

void ShortcutInputBox::reset()
{
    ResettableLineEdit::reset();

    m_sequence = QKeySequence();
}

void ShortcutInputBox::setKeySequence(const QKeySequence &seq)
{
    m_sequence = seq;
    setText(m_sequence.toString(QKeySequence::NativeText));
}
