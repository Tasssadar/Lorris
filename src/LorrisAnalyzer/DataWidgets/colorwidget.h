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

#ifndef COLORWIDGET_H
#define COLORWIDGET_H

#include "datawidget.h"

class QSlider;

class ColorWidget : public DataWidget
{
    Q_OBJECT
public:
    ColorWidget(QWidget *parent = 0);
    ~ColorWidget();

    void setUp(AnalyzerDataStorage *);
    void saveWidgetInfo(AnalyzerDataFile *file);
    void loadWidgetInfo(AnalyzerDataFile *file);

protected:
     void processData(analyzer_data *data);

private slots:
     void brightTriggered();
     void colorTriggered();
     void brightChanged(int value);
     void colorChangedR(int value);
     void colorChangedG(int value);
     void colorChangedB(int value);

private:
     void updateColor();

     QWidget *m_widget;
     QHBoxLayout *m_brightness_layout;
     QHBoxLayout *m_color_layout[3];
     qint16 m_brightness;
     qint16 m_color_cor[3];
     quint8 m_color[3];
     QAction *brightAct;
     QAction *colorAct;
};

class ColorWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    ColorWidgetAddBtn(QWidget *parent = 0);

};
#endif // COLORWIDGET_H
