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

#ifndef NUMBERWIDGET_H
#define NUMBERWIDGET_H

#include "datawidget.h"

class QMenu;

enum NumberFormats
{
    FMT_DECIMAL,
    FMT_EXPONENT,
    FMT_HEX,
    FMT_COUNT
};

class NumberWidget : public DataWidget
{
    Q_OBJECT
public:
    NumberWidget(QWidget *parent = 0);
    ~NumberWidget();

    void setUp();
    void saveWidgetInfo(AnalyzerDataFile *file);
    void loadWidgetInfo(AnalyzerDataFile *file);

protected:
     void processData(analyzer_data *data);
     void resizeEvent ( QResizeEvent * event );

private slots:
     void fmtSelected(int i);
     void bitsSelected(int i);
     void levelSelected();

private:
     QLabel *num;
     quint8 numberType;
     quint8 format;
     bool level;

     QAction *bitsAction[NUM_COUNT];
     QAction *fmtAction[FMT_COUNT];
     QAction *levelAction;
};

class NumberWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    NumberWidgetAddBtn(QWidget *parent = 0);
};

#endif // NUMBERWIDGET_H
