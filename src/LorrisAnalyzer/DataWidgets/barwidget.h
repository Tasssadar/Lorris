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

    void setUp();
    void processData(analyzer_data *data);

private slots:
    void bitsSelected(int i);
    void rangeSelected();

private:
    QProgressBar *m_bar;
    qint64 m_min;
    qint64 m_max;
    quint8 m_numberType;

    QAction *bitsAction[NUM_COUNT];
    QAction *rangeAction;
};

class BarWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    BarWidgetAddBtn(QWidget *parent);

protected:
    QPixmap getRender();
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
    QSpinBox *m_min;
    QSpinBox *m_max;
    int m_minRes;
    int m_maxRes;
    bool m_res;
};

#endif // BARWIDGET_H
