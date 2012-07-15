/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef BARWIDGET_H
#define BARWIDGET_H

#include "datawidget.h"

class QSpinBox;
class QwtThermo;

class BarWidget : public DataWidget
{
    Q_OBJECT
public:
    BarWidget(QWidget *parent);

    void setUp(Storage *);
    void processData(analyzer_data *data);
    void saveWidgetInfo(DataFileParser *file);
    void loadWidgetInfo(DataFileParser *file);

public slots:
    void setValue(const QVariant &var);
    void setRange(double min, double max);
    void rotationSelected(int i);
    void setDataType(int i);

private slots:
    void rangeSelected();
    void showScale(bool show);
    void showVal(bool show);

private:
    void rotate(int i);
    int getScalePos();

    QwtThermo *m_bar;
    QLabel *m_label;
    qint64 m_min;
    qint64 m_max;
    quint8 m_numberType;

    QAction *m_bitsAct[NUM_COUNT];
    QAction *m_rotAct[2];
    QAction *m_rangeAct;
    QAction *m_showScaleAct;
    QAction *m_showValAct;
    quint8 m_rotation;
};

class BarWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    BarWidgetAddBtn(QWidget *parent);
};

#endif // BARWIDGET_H
