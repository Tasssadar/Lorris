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

#include <QVBoxLayout>
#include "homedialog.h"
#include "HomeTab.h"

HomeDialog::HomeDialog(QWidget *parent) :
    QDialog(parent)
{
    QVBoxLayout *l = new QVBoxLayout(this);

    HomeTab *widget = new HomeTab(this);
    l->addWidget(widget);

    connect(widget, SIGNAL(tabOpened()), SLOT(accept()));

    resize(800, 450);
}
