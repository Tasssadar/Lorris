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

#ifndef BARWIDGET_H
#define BARWIDGET_H

#include <QDialog>

#include "datawidget.h"

class QProgressBar;
class QSpinBox;

class BarWidget : public DataWidget
{
    Q_OBJECT
public:
    BarWidget(QWidget *parent);

    void setUp(AnalyzerDataStorage *);
    void processData(analyzer_data *data);
    void saveWidgetInfo(AnalyzerDataFile *file);
    void loadWidgetInfo(AnalyzerDataFile *file);

public slots:
    void setValue(const QVariant &var);
    void setRange(int min, int max);
    void rotationSelected(int i);
    void setDataType(int i);

private slots:
    void rangeSelected();

private:
    void rotate(int i);

    QProgressBar *m_bar;
    qint64 m_min;
    qint64 m_max;
    quint8 m_numberType;

    QAction *bitsAction[NUM_COUNT];
    QAction *rotAction[4];
    QAction *rangeAction;
    quint8 m_rotation;
};

class BarWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    BarWidgetAddBtn(QWidget *parent);
};

namespace Ui {
  class RangeSelectDialog;
}

class RangeSelectDialog : public QDialog
{
    Q_OBJECT
public:
    RangeSelectDialog(int val_min, int val_max, int max, int min, QWidget *parent);
    ~RangeSelectDialog();

    int getMax() { return m_maxRes; }
    int getMin() { return m_minRes; }
    bool getRes() { return m_res; }

private slots:
    void maxChanged(int value);
    void minChanged(int value);
    void boxClicked(QAbstractButton* b);

private:
    Ui::RangeSelectDialog *ui;
    int m_minRes;
    int m_maxRes;
    bool m_res;
};

#endif // BARWIDGET_H
