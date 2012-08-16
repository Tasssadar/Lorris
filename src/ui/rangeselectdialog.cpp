/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QDoubleValidator>
#include <float.h>

#include "rangeselectdialog.h"
#include "ui_rangeselectdialog.h"

RangeSelectDialog::RangeSelectDialog(double val_min, double val_max, bool isInt, QWidget *parent) :
    QDialog(parent), ui(new Ui::RangeSelectDialog)
{
    ui->setupUi(this);

    m_valMin = m_valMax = NULL;

    if(isInt)
    {
        m_valMin = new QDoubleValidator(-DBL_MAX, DBL_MAX, 0, this);
        m_valMax = new QDoubleValidator(-DBL_MAX, DBL_MAX, 0, this);
    }
    else
    {
        m_valMin = new QDoubleValidator(-DBL_MAX, DBL_MAX, DBL_DIG, this);
        m_valMax = new QDoubleValidator(-DBL_MAX, DBL_MAX, DBL_DIG, this);
    }

    ui->minBox->setValidator(m_valMin);
    ui->maxBox->setValidator(m_valMax);
    ui->minBox->setText(QString::number(val_min));
    ui->maxBox->setText(QString::number(val_max));

    connect(ui->maxBox, SIGNAL(textEdited(QString)), SLOT(maxChanged(QString)));
    connect(ui->minBox, SIGNAL(textEdited(QString)), SLOT(minChanged(QString)));

    m_minRes = val_min;
    m_maxRes = val_max;

    setFixedSize(size());
}

RangeSelectDialog::~RangeSelectDialog()
{
    delete ui;
}

void RangeSelectDialog::maxChanged(const QString& text)
{
    bool ok;
    double val = text.toDouble(&ok);
    if(!ok)
        return;

    if(val < m_minRes)
        ui->maxBox->setStyleSheet("background-color: red");
    else
    {
        m_maxRes = val;
        ui->maxBox->setStyleSheet("");
    }
}

void RangeSelectDialog::minChanged(const QString &text)
{
    bool ok;
    double val = text.toDouble(&ok);
    if(!ok)
        return;

    if(val > m_maxRes)
        ui->minBox->setStyleSheet("background-color: red");
    else
    {
        m_minRes = val;
        ui->minBox->setStyleSheet("");
    }
}
