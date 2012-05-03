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

#ifndef BUTTONWIDGET_H
#define BUTTONWIDGET_H

#include "datawidget.h"

class ButtonWidget : public DataWidget
{
    Q_OBJECT
public:
    ButtonWidget(QWidget *parent);
    void setUp(Storage *storage);
    void saveWidgetInfo(DataFileParser *file);
    void loadWidgetInfo(DataFileParser *file);

public slots:
    void setButtonName(const QString& name);
    void setButtonName();

private slots:
    void buttonClicked();

private:
    QPushButton *m_button;
};

class ButtonWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    ButtonWidgetAddBtn(QWidget *parent = 0);

};
#endif // TERMINALWIDGET_H
