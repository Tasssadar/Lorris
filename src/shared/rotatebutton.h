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

protected:
    void paintEvent(QPaintEvent *);

private:
    QStyleOptionButton getStyleOption() const;

    Rotations m_rotation;
};

#endif // ROTATEBUTTON_H
