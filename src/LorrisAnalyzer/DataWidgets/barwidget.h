/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef BARWIDGET_H
#define BARWIDGET_H

#include "datawidget.h"

class QProgressBar;
class QSpinBox;

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

#endif // BARWIDGET_H
