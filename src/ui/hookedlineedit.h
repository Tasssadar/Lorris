/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef HOOKEDLINEEDIT_H
#define HOOKEDLINEEDIT_H

#include <QLineEdit>

class HookedLineEdit : public QLineEdit
{
    Q_OBJECT
Q_SIGNALS:
    void keyPressed(int keyCode);
    void keyReleased(int keyCode);

public:
    explicit HookedLineEdit(QWidget *parent = 0);

protected:
    void keyPressEvent(QKeyEvent *ev);
    void keyReleaseEvent(QKeyEvent *ev);
};

#endif // HOOKEDLINEEDIT_H
