/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef FORMULADIALOG_H
#define FORMULADIALOG_H

#include <QDialog>
#include "ui_formuladialog.h"

class FormulaDialog : public QDialog, private Ui::FormulaDialog
{
    Q_OBJECT
public:
    FormulaDialog(QString formula, QWidget *parent = NULL);
    ~FormulaDialog();

    QString getFormula() const;

public slots:
    void accept();

private:
    Ui::FormulaDialog *ui;
};

#endif // FORMULADIALOG_H
