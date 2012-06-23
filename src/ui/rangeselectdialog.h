/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef RANGESELECTDIALOG_H
#define RANGESELECTDIALOG_H

#include <QDialog>

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

#endif // RANGESELECTDIALOG_H
