/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef ROTATEBUTTON_H
#define ROTATEBUTTON_H

#include <QPushButton>

enum Rotations
{
    ROTATE_0,
    ROTATE_90,
    ROTATE_180,
    ROTATE_270
};

class RotateButton : public QPushButton
{
    Q_OBJECT

    Q_PROPERTY(Rotations rotation READ getRotation WRITE setRotation)
public:
    explicit RotateButton(QWidget *parent = 0);
    RotateButton(const QString& text, Rotations rotation, QWidget *parent = 0);
    
    Rotations getRotation() { return m_rotation; }
    void setRotation(Rotations rot) { m_rotation = rot; }

    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *);

private:
    QStyleOptionButton getStyleOption() const;

    Rotations m_rotation;
};

#endif // ROTATEBUTTON_H
