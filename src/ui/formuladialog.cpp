/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QScriptEngine>
#include "formuladialog.h"

FormulaDialog::FormulaDialog(QString formula, QWidget *parent) :
    QDialog(parent), ui(new Ui::FormulaDialog)
{
    ui->setupUi(this);

    formula.replace("%1", "%n");

    ui->formula->setText(formula);
    ui->formula->setDefaultValue(formula);
}

FormulaDialog::~FormulaDialog()
{
    delete ui;
}

QString FormulaDialog::getFormula() const
{
    return ui->formula->text();
}

void FormulaDialog::accept()
{
    QScriptEngine eng;

    QString exp = ui->formula->text();
    exp.replace("%1", "%%1");
    exp.replace("%n", "%1");

    eng.evaluate(exp.arg(10));
    if(eng.hasUncaughtException())
    {
        Utils::showErrorBox(tr("There is an error in the formula, following exception was thrown:\n\n%1")
                            .arg(eng.uncaughtException().toString()));
        return;
    }
    QDialog::accept();
}
