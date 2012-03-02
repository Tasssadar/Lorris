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

#ifndef INPUTWIDGETCLASSES_H
#define INPUTWIDGETCLASSES_H

#include <QPushButton>
#include <QLineEdit>

class Button : public QPushButton
{
    Q_OBJECT
public:
    Button( QWidget * parent = 0 ) : QPushButton(parent) { }
    ~Button() { }

    static QWidget *newInstance(QWidget *parent) { return new Button(parent); }

public slots:
    void setText(const QString &text) { QPushButton::setText(text); }
    QString text() const { return QPushButton::text(); }
};

class LineEdit : public QLineEdit
{
    Q_OBJECT
public:
    LineEdit(QWidget *parent = 0) : QLineEdit(parent) { }
    ~LineEdit() { }

    static QWidget *newInstance(QWidget *parent) { return new LineEdit(parent); }

public slots:
    QString text() const { return QLineEdit::text(); }
};

#endif // INPUTWIDGETCLASSES_H
