/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QStylePainter>
#include <QMenu>
#include <QStyleOptionButton>

#include "rotatebutton.h"

RotateButton::RotateButton(QWidget *parent) : QPushButton(parent)
{
    m_rotation = ROTATE_0;
}

RotateButton::RotateButton(const QString &text, Rotations rotation, QWidget *parent) :
    QPushButton(text, parent)
{
    m_rotation = rotation;
}

void RotateButton::paintEvent(QPaintEvent *)
{
    QStylePainter p(this);

    switch(m_rotation)
    {
        case ROTATE_90:
            p.rotate(90);
            p.translate(0, -width());
            break;
        case ROTATE_180:
            p.rotate(180);
            p.translate(-width(), -height());
            break;
        case ROTATE_270:
            p.rotate(-90);
            p.translate(-height(), 0);
            break;
        default:
            break;
    }
    p.drawControl(QStyle::CE_PushButton, getStyleOption());
}

QStyleOptionButton RotateButton::getStyleOption() const
{
    QStyleOptionButton opt;
    opt.initFrom(this);
    if (m_rotation == ROTATE_90 || m_rotation == ROTATE_270)
    {
        QSize size = opt.rect.size();
        size.transpose();
        opt.rect.setSize(size);
    }
    opt.features = QStyleOptionButton::None;
    if (isFlat())
        opt.features |= QStyleOptionButton::Flat;
    if (menu())
        opt.features |= QStyleOptionButton::HasMenu;
    if (autoDefault() || isDefault())
        opt.features |= QStyleOptionButton::AutoDefaultButton;
    if (isDefault())
        opt.features |= QStyleOptionButton::DefaultButton;
    if (isDown() || (menu() && menu()->isVisible()))
        opt.state |= QStyle::State_Sunken;
    if (isChecked())
        opt.state |= QStyle::State_On;
    if (!isFlat() && !isDown())
        opt.state |= QStyle::State_Raised;
    opt.text = text();
    opt.icon = icon();
    opt.iconSize = iconSize();
    return opt;
}
