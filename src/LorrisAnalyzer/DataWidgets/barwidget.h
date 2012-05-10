/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

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
