/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SHORTCUTINPUTBOX_H
#define SHORTCUTINPUTBOX_H

#include <QShortcut>

#include "resettablelineedit.h"

class ShortcutInputBox : public ResettableLineEdit
{
    Q_OBJECT
    
public:
    explicit ShortcutInputBox(QWidget *parent = 0);
    ShortcutInputBox(const QKeySequence& seq, QWidget *parent = 0);

    QKeySequence getKeySequence() { return m_sequence; }
    void setKeySequence(const QKeySequence& seq);

protected slots:
    void reset();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    QKeySequence m_sequence;
};

#endif // SHORTCUTINPUTBOX_H
