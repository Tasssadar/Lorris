/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef RANGESELECTDIALOG_H
#define RANGESELECTDIALOG_H

#include <QDialog>
#include <QAbstractButton>

namespace Ui {
  class RangeSelectDialog;
}

class QDoubleValidator;

class RangeSelectDialog : public QDialog
{
    Q_OBJECT
public:
    RangeSelectDialog(double val_min, double val_max, bool isInt, QWidget *parent);
    ~RangeSelectDialog();

    double getMax() { return m_maxRes; }
    double getMin() { return m_minRes; }

private slots:
    void maxChanged(const QString& text);
    void minChanged(const QString& text);

private:
    Ui::RangeSelectDialog *ui;
    double m_minRes;
    double m_maxRes;

    QDoubleValidator *m_valMin;
    QDoubleValidator *m_valMax;
};

#endif // RANGESELECTDIALOG_H
