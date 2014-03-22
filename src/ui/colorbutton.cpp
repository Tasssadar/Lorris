/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QColorDialog>

#include "colorbutton.h"

ColorButton::ColorButton(QColor color, QWidget *parent) :
    QPushButton(parent)
{
    setIconSize(QSize(32, 16));
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setColor(color);

    connect(this, SIGNAL(clicked()), SLOT(choose()));
}

ColorButton::ColorButton(QWidget *parent) : QPushButton(parent)
{
    setIconSize(QSize(32, 16));
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setColor(Qt::black);

    connect(this, SIGNAL(clicked()), SLOT(choose()));
}

ColorButton::ColorButton(const ColorButton &btn) : QPushButton(btn.parentWidget())
{
    ColorButton(btn.getColor(), btn.parentWidget());
}

void ColorButton::setColor(const QColor &color)
{
    if(m_color == color)
        return;

    m_color = color;

    QPixmap map(iconSize());
    map.fill(color);
    setIcon(QIcon(map));

    emit colorChanged(color);
}

void ColorButton::choose()
{
    QColor c = QColorDialog::getColor(m_color);
    if(!c.isValid())
        return;
    setColor(c);
}
