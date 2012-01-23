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

#ifndef GRAPHDIALOGS_H
#define GRAPHDIALOGS_H

#include <QDialog>

namespace Ui {
    class GraphCurveAddDialog;
    class GraphCurveEditWidget;
}

class GraphCurveAddDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GraphCurveAddDialog(QWidget *parent = 0);
    ~GraphCurveAddDialog();

private:
    Ui::GraphCurveAddDialog *ui;
    Ui::GraphCurveEditWidget *edit_widget_ui;
    QWidget *m_edit_widget;
};

#endif // GRAPHDIALOGS_H
