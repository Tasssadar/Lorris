/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

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
